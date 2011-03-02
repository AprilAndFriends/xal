/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "StreamSound.h"
#include "Source.h"

#include <iostream>
#if HAVE_OGG
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#endif

#ifndef __APPLE__
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#include "Endianess.h"

namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	StreamSound::StreamSound(chstr filename, chstr category, chstr prefix) :
		SoundBuffer(filename, category, prefix), bufferIndex(0), 
	{
		for (int i = 0; i < STREAM_BUFFER_COUNT; i++)
		{
			this->buffers[i] = 0;
		}
	}

	StreamSound::~StreamSound()
	{
		this->destroySources();
		for (int i = 0; i < STREAM_BUFFER_COUNT; i++)
		{
			if (this->buffers[i] != 0)
			{
				alDeleteBuffers(1, &this->buffers[i]);
			}
		}
		if (xal::mgr->isEnabled())
		{
#if HAVE_OGG
			if (this->isOgg())
			{
				ov_clear(&this->oggStream);
			}
#endif
		}
	}
	
/******* METHODS *******************************************************/
	
	Sound* StreamSound::play(float fadeTime, bool looping)
	{
		if (this->getBuffer() == 0)
		{
			return NULL;
		}
		Source* source = NULL;
		if (this->sources.size() > 0 && this->sources[0]->isPlaying())
		{
			if (!this->isFading())
			{
				return this->sources[0];
			}
			this->pause(fadeTime);
		}
		return SoundBuffer::play(fadeTime, looping);
	}
	
	unsigned int StreamSound::getBuffer() const
	{
		return this->buffers[this->bufferIndex];
	}
	
	void StreamSound::update(float k)
	{
		int queued;
		alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
		if (queued == 0)
		{
			this->stopSoft();
			return;
		}
		int count;
		alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &count);
		if (count == 0)
		{
			return;
		}
		this->unqueueBuffers((this->bufferIndex + STREAM_BUFFER_COUNT - queued) % STREAM_BUFFER_COUNT, count);
		int bytes = 0;
		int result;
		int i = 0;
		for (; i < count; i++)
		{
			result = this->_fillBuffer(this->buffers[(this->bufferIndex + i) % STREAM_BUFFER_COUNT]);
			if (result == 0)
			{
				break;
			}
			bytes += result;
		}
		if (bytes > 0)
		{
			this->queueBuffers(this->bufferIndex, i);
			if (count < STREAM_BUFFER_COUNT)
			{
				this->bufferIndex = (this->bufferIndex + i) % STREAM_BUFFER_COUNT;
			}
			else // underrun happened, sound was stopped
			{
				this->pause();
				this->play();
			}
		}
		alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
		if (queued == 0)
		{
			this->stopSoft();
		}
	}
	
	int StreamSound::_readStream(char* buffer, int size)
	{
#if HAVE_OGG
		if (this->isOgg())
		{
			int section;
			return ov_read(&this->oggStream, buffer, size, 0, 2, 1, &section);
		}
#endif
		return 0;
	}
	
	void StreamSound::_resetStream()
	{
#if HAVE_OGG
		if (this->isOgg())
		{
			ov_raw_seek(&this->oggStream, 0);
		}
#endif
	}
	
	int StreamSound::_fillBuffer(unsigned int buffer)
	{
		char data[STREAM_BUFFER_SIZE] = {0};
		int size = 0;
		int result;
		while (size < STREAM_BUFFER_SIZE)
		{
			result = this->_readStream(&data[size], STREAM_BUFFER_SIZE - size);
			if (result > 0)
			{
				size += result;
			}
			else if (result == 0)
			{
				if (!this->isLooping())
				{
					break;
				}
				this->_resetStream();
			}
			else
			{
				xal::log("error while filling buffer for " + this->name);
			}
		}
		if (size > 0)
		{
#ifdef __BIG_ENDIAN__
			for (uint16_t* p = (uint16_t*)data; (char*)p < data + size; p++)
			{
				XAL_NORMALIZE_ENDIAN(*p);
			}
#endif	
#if HAVE_OGG
			// FIXME what if we're not using Ogg/Vorbis?
			// then vorbisInfo does not exist.
			alBufferData(buffer, (this->vorbisInfo->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
				data, size, this->vorbisInfo->rate);
#endif
		}
		return size;
	}
	
	void StreamSound::rewindStream()
	{
		this->unqueueBuffers();
		this->_fillStartBuffers();
	}

	void StreamSound::_fillStartBuffers()
	{
		this->_resetStream();
		for (int i = 0; i < STREAM_BUFFER_COUNT; i++)
		{
			this->_fillBuffer(this->buffers[i]);
		}
		this->bufferIndex = 0;
	}

	void StreamSound::queueBuffers(int index, int count)
	{
		if (index + count <= STREAM_BUFFER_COUNT)
		{
			alSourceQueueBuffers(this->sourceId, count, &this->buffers[index]);
		}
		else
		{
			alSourceQueueBuffers(this->sourceId, STREAM_BUFFER_COUNT - index, &this->buffers[index]);
			alSourceQueueBuffers(this->sourceId, count + index - STREAM_BUFFER_COUNT, this->buffers);
		}
	}
 
	void StreamSound::unqueueBuffers(int index, int count)
	{
		if (index + count <= STREAM_BUFFER_COUNT)
		{
			alSourceUnqueueBuffers(this->sourceId, count, &this->buffers[index]);
		}
		else
		{
			alSourceUnqueueBuffers(this->sourceId, STREAM_BUFFER_COUNT - index, &this->buffers[index]);
			alSourceUnqueueBuffers(this->sourceId, count + index - STREAM_BUFFER_COUNT, this->buffers);
		}
	}
 
	void StreamSound::queueBuffers()
	{
		int queued;
		alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
		this->queueBuffers(this->bufferIndex, STREAM_BUFFER_COUNT - queued);
	}
 
	void StreamSound::unqueueBuffers()
	{
		int queued;
		alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
		this->unqueueBuffers((this->bufferIndex + STREAM_BUFFER_COUNT - queued) % STREAM_BUFFER_COUNT, queued);
	}
 
	bool StreamSound::_loadOgg()
	{
#if HAVE_OGG
		xal::log("loading ogg stream sound " + this->fileName);
		if (ov_fopen((char*)this->virtualFileName.c_str(), &this->oggStream) != 0)
		{
			xal::log("ogg: Error opening file!");
			return false;
		}
		alGenBuffers(STREAM_BUFFER_COUNT, this->buffers);
		this->vorbisInfo = ov_info(&this->oggStream, -1);
		unsigned long len = (unsigned long)ov_pcm_total(&this->oggStream, -1) * this->vorbisInfo->channels * 2; // always 16 bit data
		this->duration = (float)len / (this->vorbisInfo->rate * this->vorbisInfo->channels * 2);
		int bytes;
		for (int i = 0; i < STREAM_BUFFER_COUNT; i++)
		{
			bytes = this->_fillBuffer(this->buffers[i]);
			if (bytes == 0)
			{
				alDeleteBuffers(STREAM_BUFFER_COUNT, this->buffers);
				this->buffers[0] = 0;
				xal::log("sound " + this->virtualFileName + " is too small to be streamed.");
				break;
			}
		}
		return true;
#else
#warning HAVE_OGG is not defined to 1. No Ogg support.
		xal::log("no ogg support built in, cannot load stream sound " + this->fileName);
		return false;
#endif
		
	}
	
}