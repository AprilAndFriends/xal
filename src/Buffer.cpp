/// @file
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <string.h> // required on Unix because of memset usage

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "Sound.h"
#include "Source.h"
#include "xal.h"

namespace xal
{
	Buffer::Buffer(Sound* sound) : loaded(false), idleTime(0.0f)
	{
		this->filename = sound->getRealFilename();
		this->fileSize = hresource::hsize(this->filename);
		this->mode = sound->getCategory()->getBufferMode();
		this->stream = NULL;
		this->streamSize = 0;
		this->dataSize = 0;
		this->source = xal::mgr->_createSource(this->filename, sound->getCategory(), this->getFormat());
		this->loadedMetaData = false;
		this->size = 0;
		this->channels = 2;
		this->samplingRate = 44100;
		this->bitPerSample = 16;
		this->duration = 0.0f;
		if (xal::mgr->isEnabled() && this->getFormat() != UNKNOWN)
		{
			switch (this->mode)
			{
			case FULL:
				this->prepare();
				break;
			case LAZY:
				break;
			case MANAGED:
				break;
			case ON_DEMAND:
				break;
			case STREAMED:
				break;
			default:
				break;
			}
		}
	}

	Buffer::~Buffer()
	{
		if (this->stream != NULL)
		{
			delete [] this->stream;
		}
		delete this->source;
	}
	
	int Buffer::getSize()
	{
		this->_tryLoadMetaData();
		return this->size;
	}

	int Buffer::getChannels()
	{
		this->_tryLoadMetaData();
		return this->source->getChannels();
	}

	int Buffer::getSamplingRate()
	{
		this->_tryLoadMetaData();
		return this->source->getSamplingRate();
	}

	int Buffer::getBitsPerSample()
	{
		this->_tryLoadMetaData();
		return this->source->getBitsPerSample();
	}

	float Buffer::getDuration()
	{
		this->_tryLoadMetaData();
		return this->source->getDuration();
	}

	Format Buffer::getFormat()
	{
#ifdef _FORMAT_FLAC
		if (this->filename.ends_with(".flac"))
		{
			return FLAC;
		}
#endif
#ifdef _FORMAT_M4A
		if (this->filename.ends_with(".m4a"))
		{
			return M4A;
		}
#endif
#ifdef _FORMAT_OGG
		if (this->filename.ends_with(".ogg"))
		{
			return OGG;
		}
#endif
#ifdef _FORMAT_SPX
		if (this->filename.ends_with(".spx"))
		{
			return SPX;
		}
#endif
#ifdef _FORMAT_WAV
		if (this->filename.ends_with(".wav"))
		{
			return WAV;
		}
#endif
		return UNKNOWN;
	}

	bool Buffer::isStreamed()
	{
		return (this->mode == STREAMED);
	}

	bool Buffer::isMemoryManaged()
	{
		return (this->mode == MANAGED);
	}

	void Buffer::prepare()
	{
		if (this->loaded)
		{
			return;
		}
		if (!xal::mgr->isEnabled())
		{
			this->loaded = true;
			return;
		}
		if (!this->isStreamed())
		{
			this->loaded = true;
			this->source->open();
			this->_tryLoadMetaData();
			if (this->stream == NULL)
			{
				this->dataSize = this->source->getSize();
				this->streamSize = this->dataSize;
				this->stream = new unsigned char[this->streamSize];
			}
			this->source->load(this->stream);
			this->source->close();
			this->dataSize = xal::mgr->_convertStream(this, &this->stream, &this->streamSize, this->dataSize);
			return;
		}
		if (!this->source->isOpen())
		{
			this->source->open();
			this->_tryLoadMetaData();
		}
	}

	int Buffer::load(bool looping, int size)
	{
		this->keepLoaded();
		if (!xal::mgr->isEnabled())
		{
			return 0;
		}
		if (this->isStreamed() && this->source->isOpen())
		{
			if (this->stream == NULL)
			{
				this->dataSize = STREAM_BUFFER;
				this->streamSize = this->dataSize;
				this->stream = new unsigned char[this->streamSize];
			}
			this->dataSize = this->source->loadChunk(this->stream, size);
			size -= this->dataSize;
			if (size > 0)
			{
				if (!looping)
				{
					// fill rest of buffer with silence so systems that depends on buffered chunks don't get messed up
					memset(&this->stream[this->dataSize], 0, size * sizeof(unsigned char));
				}
				else
				{
					int read = 0;
					while (size > 0)
					{
						this->source->rewind();
						read = this->source->loadChunk(&this->stream[this->dataSize], size);
						size -= read;
						this->dataSize += read;
					}
				}
			}
			this->dataSize = xal::mgr->_convertStream(this, &this->stream, &this->streamSize, this->dataSize);
		}
		return this->dataSize;
	}

	void Buffer::bind(Player* player, bool playerPaused)
	{
		this->boundPlayers |= player;
	}

	void Buffer::unbind(Player* player, bool playerPaused)
	{
		if (!playerPaused)
		{
			this->boundPlayers /= player;
		}
		if (this->boundPlayers.size() == 0 && this->mode == xal::ON_DEMAND || this->mode == xal::STREAMED)
		{
			if (this->stream != NULL)
			{
				delete [] this->stream;
				this->stream = NULL;
				this->streamSize = 0;
			}
			this->loaded = false;
		}
		if (this->boundPlayers.size() == 0 && this->mode == xal::STREAMED)
		{
			this->source->close();
			this->loaded = false;
		}
	}

	void Buffer::keepLoaded()
	{
		this->idleTime = 0.0f;
	}

	void Buffer::rewind()
	{
		this->source->rewind();
	}

	int Buffer::calcOutputSize(int size)
	{
		return hround((float)size * xal::mgr->getSamplingRate() * xal::mgr->getChannels() * xal::mgr->getBitsPerSample() /
			((float)this->getSamplingRate() * this->getChannels() * this->getBitsPerSample()));
	}

	int Buffer::calcInputSize(int size)
	{
		return hround((float)size * this->getSamplingRate() * this->getChannels() * this->getBitsPerSample() /
			((float)xal::mgr->getSamplingRate() * xal::mgr->getChannels() * xal::mgr->getBitsPerSample()));
	}

	int Buffer::readPcmData(unsigned char** output)
	{
		*output = NULL;
		int result = 0;
		if (this->getFormat() != UNKNOWN)
		{
			this->source->open();
			result = this->source->getSize();
			if (result > 0)
			{
				*output = new unsigned char[result];
				this->source->load(*output);
				result = xal::mgr->_convertStream(this, output, &result, result);
			}
			this->source->close();
		}
		return result;
	}

	void Buffer::_update(float timeDelta)
	{
		this->idleTime += timeDelta;
		if (this->idleTime >= xal::mgr->getIdlePlayerUnloadTime())
		{
			this->_tryClearMemory();
		}
	}

	void Buffer::_tryLoadMetaData()
	{
		if (!this->loadedMetaData)
		{
			bool open = this->source->isOpen();
			if (!open)
			{
				this->source->open();
			}
			this->size = this->source->getSize();
			this->channels = this->source->getChannels();
			this->samplingRate = this->source->getSamplingRate();
			this->bitPerSample = this->source->getBitsPerSample();
			this->duration = this->source->getDuration();
			this->loadedMetaData = true;
			if (!open)
			{
				this->source->close();
			}
		}
	}

	bool Buffer::_tryClearMemory()
	{
		if (this->isMemoryManaged() && this->boundPlayers.size() == 0 && (this->loaded || this->mode == STREAMED))
		{
			hlog::debug(xal::logTag, "Clearing memory for: " + this->filename);
			if (this->stream != NULL)
			{
				delete [] this->stream;
				this->stream = NULL;
				this->streamSize = 0;
			}
			this->source->close();
			this->loaded = false;
			return true;
		}
		return false;
	}

}
