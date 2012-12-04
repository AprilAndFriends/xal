/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

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
	Buffer::Buffer(Sound* sound) : loaded(false), decoded(false), idleTime(0.0f)
	{
		this->filename = sound->getRealFilename();
		this->fileSize = hresource::hsize(this->filename);
		this->mode = sound->getCategory()->getBufferMode();
		this->stream = NULL;
		this->streamSize = 0;
		this->dataSize = 0;
		this->source = xal::mgr->_createSource(this->filename, sound->getCategory(), this->getFormat());
		this->loadedData = false;
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
		this->_tryLoadData();
		return this->size;
	}

	int Buffer::getChannels()
	{
		this->_tryLoadData();
		return this->source->getChannels();
	}

	int Buffer::getSamplingRate()
	{
		this->_tryLoadData();
		return this->source->getSamplingRate();
	}

	int Buffer::getBitsPerSample()
	{
		this->_tryLoadData();
		return this->source->getBitsPerSample();
	}

	float Buffer::getDuration()
	{
		this->_tryLoadData();
		return this->source->getDuration();
	}

	Format Buffer::getFormat()
	{
#ifdef HAVE_FLAC
		if (this->filename.ends_with(".flac"))
		{
			return FLAC;
		}
#endif
#ifdef HAVE_M4A
		if (this->filename.ends_with(".m4a"))
		{
			return M4A;
		}
#endif
#ifdef HAVE_OGG
		if (this->filename.ends_with(".ogg"))
		{
			return OGG;
		}
#endif
#ifdef HAVE_SPX
		if (this->filename.ends_with(".spx"))
		{
			return SPX;
		}
#endif
#ifdef HAVE_WAV
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
			this->_tryLoadData();
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
			this->_tryLoadData();
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
				this->dataSize = STREAM_BUFFER_COUNT * STREAM_BUFFER_SIZE;
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

	int Buffer::convertToOutputSize(int size)
	{
		return hround((float)size * xal::mgr->getSamplingRate() * xal::mgr->getChannels() * xal::mgr->getBitsPerSample() /
			((float)this->getSamplingRate() * this->getChannels() * this->getBitsPerSample()));
	}

	int Buffer::convertToInputSize(int size)
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

	void Buffer::_update(float k)
	{
		this->idleTime += k;
		if (this->idleTime > xal::mgr->getIdlePlayerUnloadTime())
		{
			this->_tryClearMemory();
		}
	}

	void Buffer::_tryLoadData()
	{
		if (!this->loadedData)
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
			this->loadedData = true;
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
