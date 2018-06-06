/// @file
/// @version 4.0
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
#include "BufferAsync.h"
#include "Category.h"
#include "Sound.h"
#include "Source.h"
#include "xal.h"

namespace xal
{
	Buffer::Buffer(Sound* sound)
	{
		this->filename = sound->getFilename();
		this->fileSize = (int)hresource::hinfo(this->filename).size;
		Category* category = sound->getCategory();
		this->mode = category->getBufferMode();
		this->loaded = false;
		this->asyncLoadQueued = false;
		this->asyncLoadDiscarded = false;
		this->source = xal::manager->_createSource(this->filename, category->getSourceMode(), this->mode, this->getFormat());
		this->loadedMetaData = false;
		this->size = 0;
		this->channels = 2;
		this->samplingRate = 44100;
		this->bitsPerSample = 16;
		this->duration = 0.0f;
		this->idleTime = 0.0f;
		if (xal::manager->isEnabled() && this->getFormat() != Format::Unknown)
		{
			if (this->mode == BufferMode::Full)
			{
				this->prepare();
			}
			else if (this->mode == BufferMode::Async)
			{
				this->prepareAsync();
			}
		}
	}

	Buffer::Buffer(Category* category, unsigned char* data, int size, int channels, int samplingRate, int bitsPerSample)
	{
		this->stream.writeRaw(data, size);
		this->fileSize = size;
		this->mode = BufferMode::Full;
		this->loaded = true;
		this->asyncLoadQueued = false;
		this->asyncLoadDiscarded = false;
		this->source = NULL;
		this->loadedMetaData = true;
		this->size = size;
		this->channels = channels;
		this->samplingRate = samplingRate;
		this->bitsPerSample = bitsPerSample;
		this->duration = (float)size / (samplingRate * channels * bitsPerSample / 8);
		this->idleTime = 0.0f;
	}

	Buffer::~Buffer()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		this->asyncLoadQueued = false;
		this->asyncLoadDiscarded = false;
		this->loaded = false;
		if (this->source != NULL)
		{
			delete this->source;
		}
	}
	
	int Buffer::getSize()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		this->_tryLoadMetaData();
		return this->size;
	}

	int Buffer::getChannels()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		this->_tryLoadMetaData();
		return this->channels;
	}

	int Buffer::getSamplingRate()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		this->_tryLoadMetaData();
		return this->samplingRate;
	}

	int Buffer::getBitsPerSample()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		this->_tryLoadMetaData();
		return this->bitsPerSample;
	}

	float Buffer::getDuration()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		this->_tryLoadMetaData();
		return this->duration;
	}

	Format Buffer::getFormat() const
	{
		if (this->filename == "" && this->source == NULL)
		{
			return Format::Memory;
		}
#ifdef _FORMAT_FLAC
		if (this->filename.endsWith(".flac"))
		{
			return Format::FLAC;
		}
#endif
#ifdef _FORMAT_M4A
		if (this->filename.endsWith(".m4a"))
		{
			return Format::M4A;
		}
#endif
#ifdef _FORMAT_OGG
		if (this->filename.endsWith(".ogg"))
		{
			return Format::OGG;
		}
#endif
#ifdef _FORMAT_SPX
		if (this->filename.endsWith(".spx"))
		{
			return Format::SPX;
		}
#endif
#ifdef _FORMAT_WAV
		if (this->filename.endsWith(".wav"))
		{
			return Format::WAV;
		}
#endif
		return Format::Unknown;
	}

	bool Buffer::isLoaded()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		return this->loaded;
	}

	bool Buffer::isAsyncLoadQueued()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		return this->asyncLoadQueued;
	}

	bool Buffer::isStreamed() const
	{
		return (this->mode == BufferMode::Streamed);
	}

	bool Buffer::isMemoryManaged() const
	{
		return (this->mode == BufferMode::Managed);
	}

	void Buffer::prepare()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		this->asyncLoadDiscarded = false; // a possible previous unload call must be canceled
		if (!xal::manager->isEnabled() || this->loaded || this->source == NULL)
		{
			this->asyncLoadQueued = false;
			this->loaded = true;
			return;
		}
		if (this->asyncLoadQueued)
		{
			if (!this->loaded)
			{
				lock.release();
				this->_waitForAsyncLoad();
			}
			return;
		}
		if (!this->isStreamed())
		{
			this->loaded = true;
			this->source->open();
			this->stream.clear(this->source->getSize());
			this->source->load(this->stream);
			this->source->close();
			xal::manager->_convertStream(this->source, this->stream);
			return;
		}
		lock.release();
		// streamed sounds cannot be loaded asynchronously and hence require no mutex locking
		if (!this->source->isOpen())
		{
			this->source->open();
			this->_tryLoadMetaData();
		}
	}

	bool Buffer::prepareAsync()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (!xal::manager->isEnabled() || this->loaded)
		{
			this->loaded = true;
			return false;
		}
		if (this->isStreamed())
		{
			hlog::warn(logTag, "Streamed sound cannot be loaded asynchronously: " + this->getFilename());
			return false;
		}
		this->asyncLoadDiscarded = false;
		if (!this->asyncLoadQueued) // this check is down here to allow the upper error messages to be displayed
		{
			this->asyncLoadQueued = BufferAsync::queueLoad(this);
		}
		return this->asyncLoadQueued;
	}

	int Buffer::load(bool looping, int size)
	{
		this->keepLoaded();
		if (!xal::manager->isEnabled())
		{
			return 0;
		}
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (this->isStreamed() && this->source != NULL && this->source->isOpen())
		{
			this->stream.clear(STREAM_BUFFER);
			int read = this->source->loadChunk(this->stream, size);
			size -= read;
			if (size > 0)
			{
				this->stream.seek(read);
				if (!looping)
				{
					// if stream isn't empty, fill the rest with silence so systems that depend on same-sized chunks don't get messed up
					if (this->stream.size() > 0)
					{
						this->stream.fill(0, size);
						this->stream.truncate(read);
					}
				}
				else
				{
					while (size > 0)
					{
						this->source->rewind();
						read = this->source->loadChunk(this->stream, size);
						if (read == 0) // to prevent an infinite loop
						{
							break;
						}
						size -= read;
						this->stream.seek(read);
					}
				}
				this->stream.rewind();
			}
			xal::manager->_convertStream(this->source, this->stream);
		}
		return (int)this->stream.size();
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
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (this->boundPlayers.size() == 0 && this->mode == BufferMode::OnDemand || this->mode == BufferMode::Streamed)
		{
			this->stream.clear(1);
			this->asyncLoadQueued = false;
			this->asyncLoadDiscarded = true;
			this->loaded = false;
		}
		if (this->boundPlayers.size() == 0 && this->mode == BufferMode::Streamed)
		{
			if (this->source != NULL)
			{
				this->source->close();
			}
			this->asyncLoadQueued = false;
			this->asyncLoadDiscarded = true;
			this->loaded = false;
		}
	}

	void Buffer::keepLoaded()
	{
		this->idleTime = 0.0f;
	}

	void Buffer::rewind()
	{
		if (this->source != NULL)
		{
			this->source->rewind();
		}
		else
		{
			this->stream.rewind();
		}
	}

	int Buffer::calcOutputSize(int size)
	{
		return hround((float)size * xal::manager->getSamplingRate() * xal::manager->getChannels() * xal::manager->getBitsPerSample() /
			((float)this->getSamplingRate() * this->getChannels() * this->getBitsPerSample()));
	}

	int Buffer::calcInputSize(int size)
	{
		return hround((float)size * this->getSamplingRate() * this->getChannels() * this->getBitsPerSample() /
			((float)xal::manager->getSamplingRate() * xal::manager->getChannels() * xal::manager->getBitsPerSample()));
	}

	void Buffer::readPcmData(hstream& output)
	{
		// no mutex locking, because a separate source is used
		if (this->getFormat() != Format::Unknown)
		{
			Source* source = xal::manager->_createSource(this->filename, SourceMode::Disk, BufferMode::Full, this->getFormat());
			source->open();
			if (source->getSize() > 0)
			{
				source->load(output);
				xal::manager->_convertStream(source, output);
			}
			source->close();
			delete source;
		}
	}

	void Buffer::_update(float timeDelta)
	{
		this->idleTime += timeDelta;
		if (this->idleTime >= xal::manager->getIdlePlayerUnloadTime())
		{
			this->_tryClearMemory();
		}
	}

	void Buffer::_tryLoadMetaData()
	{
		if (!this->loadedMetaData && this->source != NULL)
		{
			bool open = this->source->isOpen();
			if (!open)
			{
				this->source->open();
			}
			this->size = this->source->getSize();
			this->channels = this->source->getChannels();
			this->samplingRate = this->source->getSamplingRate();
			this->bitsPerSample = this->source->getBitsPerSample();
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
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (this->isMemoryManaged() && this->boundPlayers.size() == 0 && this->source != NULL && (this->loaded || this->mode == BufferMode::Streamed))
		{
			hlog::debug(logTag, "Clearing memory for: " + this->filename);
			this->stream.clear(1L);
			this->source->close();
			this->asyncLoadQueued = false;
			this->asyncLoadDiscarded = true;
			this->loaded = false;
			return true;
		}
		return false;
	}

	bool Buffer::_prepareAsyncStream()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (!this->asyncLoadQueued || this->asyncLoadDiscarded || this->source == NULL)
		{
			this->asyncLoadQueued = false;
			this->asyncLoadDiscarded = false;
			return false;
		}
		this->source->open();
		if (!this->source->isOpen())
		{
			this->asyncLoadQueued = false;
			this->asyncLoadDiscarded = false;
			return false;
		}
		return true;
	}

	void Buffer::_decodeFromAsyncStream()
	{
		hmutex::ScopeLock lock(&this->asyncLoadMutex);
		if (!this->asyncLoadQueued || this->asyncLoadDiscarded || this->loaded || this->source == NULL)
		{
			this->source->close();
			this->asyncLoadQueued = false;
			this->asyncLoadDiscarded = false;
			return;
		}
		this->_tryLoadMetaData();
		this->stream.clear(this->source->getSize());
		this->source->load(this->stream);
		xal::manager->_convertStream(this->source, this->stream);
		this->source->close();
		this->asyncLoadQueued = false;
		this->asyncLoadDiscarded = false;
		this->loaded = true;
	}

	void Buffer::_waitForAsyncLoad(float timeout)
	{
		BufferAsync::prioritizeLoad(this);
		float time = timeout;
		hmutex::ScopeLock lock;
		while (time > 0.0f || timeout <= 0.0f)
		{
			lock.acquire(&this->asyncLoadMutex);
			if (this->loaded || this->asyncLoadDiscarded)
			{
				if (this->asyncLoadDiscarded)
				{
					this->loaded = false;
				}
				this->asyncLoadQueued = false;
				this->asyncLoadDiscarded = false;
				break;
			}
			lock.release();
			hthread::sleep(0.1f);
			time -= 0.0001f;
			BufferAsync::update();
		}
	}

}
