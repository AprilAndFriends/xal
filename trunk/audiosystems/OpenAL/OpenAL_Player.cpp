/// @file
/// @author  Boris Mikic
/// @version 2.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_OPENAL
#include <stdio.h>
#include <string.h>
#include "xal.h"

#ifndef __APPLE__
#include <AL/al.h>
#else
#include <TargetConditionals.h>
#include <OpenAL/al.h>
#endif

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "OpenAL_AudioManager.h"
#include "OpenAL_Player.h"
#include "Sound.h"

namespace xal
{
	OpenAL_Player::OpenAL_Player(Sound* sound, Buffer* buffer) :
		Player(sound, buffer), sourceId(0)
	{
		memset(this->bufferIds, 0, STREAM_BUFFER_COUNT * sizeof(unsigned int));
		alGenBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
	}

	OpenAL_Player::~OpenAL_Player()
	{
		// AudioManager calls _stop before destruction
		alDeleteBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
	}

	void OpenAL_Player::_update(float k)
	{
		Player::_update(k);
		if (!this->_systemIsPlaying() && this->sourceId != 0)
		{
			this->_stopSound();
		}
	}

	bool OpenAL_Player::_systemIsPlaying()
	{
		if (this->sourceId == 0)
		{
			return false;
		}
		if (this->sound->isStreamed())
		{
			return (this->_getQueuedBuffersCount() > 0 || this->_getProcessedBuffersCount() > 0);
		}
		int state;
		alGetSourcei(this->sourceId, AL_SOURCE_STATE, &state);
		return (state == AL_PLAYING);
	}

	float OpenAL_Player::_systemGetOffset()
	{
		float offset = 0.0f;
#if !TARGET_OS_MAC
		if (this->sourceId != 0)
		{
			alGetSourcef(this->sourceId, AL_SAMPLE_OFFSET, &offset);
		}
#else
		// did not find anything that works on Mac OS X and iOS!
#endif
		return offset;
	}

	void OpenAL_Player::_systemSetOffset(float value)
	{
#if !TARGET_OS_MAC
		// TODO - should be int
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_SAMPLE_OFFSET, value);
		}
		//alSourcei(this->sourceId, AL_SAMPLE_OFFSET, value);
#else
		// did not find anything that works on Mac OS X and iOS!
#endif
	}

	bool OpenAL_Player::_systemPreparePlay()
	{
		if (this->sourceId == 0)
		{
			this->sourceId = ((OpenAL_AudioManager*)xal::mgr)->_allocateSourceId();
		}
		return (this->sourceId != 0);
	}

	void OpenAL_Player::_systemPrepareBuffer()
	{
		// making sure all buffer data is loaded before accessing anything
		if (!this->sound->isStreamed())
		{
			this->_fillBuffers(0, 1);
			alSourcei(this->sourceId, AL_BUFFER, this->bufferIds[0]);
			alSourcei(this->sourceId, AL_LOOPING, this->looping);
		}
		else
		{
			alSourcei(this->sourceId, AL_BUFFER, AL_NONE);
			alSourcei(this->sourceId, AL_LOOPING, false);
			int count = STREAM_BUFFER_COUNT;
			if (!this->paused)
			{
				count = this->_fillBuffers(this->bufferIndex, STREAM_BUFFER_COUNT);
			}
			if (count > 0)
			{
				this->_queueBuffers(this->bufferIndex, count);
				this->bufferIndex = (this->bufferIndex + count) % STREAM_BUFFER_COUNT;
			}
		}
	}

	void OpenAL_Player::_systemUpdateGain()
	{
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcGain());
		}
	}

	void OpenAL_Player::_systemUpdateFadeGain()
	{
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcFadeGain());
		}
	}

	void OpenAL_Player::_systemPlay()
	{
		if (this->sourceId != 0)
		{
			alSourcePlay(this->sourceId);
		}
	}

	void OpenAL_Player::_systemStop()
	{
		if (this->sourceId != 0)
		{
			if (!this->sound->isStreamed())
			{
				alSourceStop(this->sourceId);
				alSourcei(this->sourceId, AL_BUFFER, AL_NONE); // necessary to avoid a memory leak in OpenAL
			}
			else
			{
				int processed = this->_getProcessedBuffersCount();
				alSourceStop(this->sourceId);
				this->_unqueueBuffers();
				alSourcei(this->sourceId, AL_BUFFER, AL_NONE); // necessary to avoid a memory leak in OpenAL
				if (this->paused)
				{
					this->bufferIndex = (this->bufferIndex + processed) % STREAM_BUFFER_COUNT;
				}
				else
				{
					this->bufferIndex = 0;
					this->buffer->rewind();
				}
			}
			((OpenAL_AudioManager*)xal::mgr)->_releaseSourceId(this->sourceId);
			this->sourceId = 0;
		}
	}

	void OpenAL_Player::_systemUpdateStream()
	{
		int queued = this->_getQueuedBuffersCount();
		if (queued == 0)
		{
			this->_stopSound();
			return;
		}
		int processed = this->_getProcessedBuffersCount();
		if (processed == 0)
		{
			return;
		}
		this->_unqueueBuffers((this->bufferIndex + STREAM_BUFFER_COUNT - queued) % STREAM_BUFFER_COUNT, processed);
		int count = this->_fillBuffers(this->bufferIndex, processed);
		if (count > 0)
		{
			this->_queueBuffers(this->bufferIndex, count);
			this->bufferIndex = (this->bufferIndex + count) % STREAM_BUFFER_COUNT;
			bool playing = (processed < STREAM_BUFFER_COUNT);
			if (playing)
			{
				int state;
				alGetSourcei(this->sourceId, AL_SOURCE_STATE, &state);
				if (state != AL_PLAYING)
				{
					playing = false;
				}
			}
			if (!playing) // underrun happened, sound was stopped by OpenAL so let's reboot it properly
			{
				this->_pause();
				this->_play();
			}
		}
		if (this->_getQueuedBuffersCount() == 0)
		{
			this->_stopSound();
		}
	}

	int OpenAL_Player::_getQueuedBuffersCount()
	{
		int queued = 0;
		if (this->sourceId)
		{
			alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
		}
		return queued;
	}

	int OpenAL_Player::_getProcessedBuffersCount()
	{
		int processed = 0;
		if (this->sourceId)
		{
			alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
		}
		return processed;
	}

	int OpenAL_Player::_fillBuffers(int index, int count)
	{
		int size = this->buffer->load(this->looping, count * STREAM_BUFFER_SIZE);
		if (!this->sound->isStreamed())
		{
			alBufferData(this->bufferIds[index], (this->buffer->getChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16),
				this->buffer->getStream(), size, this->buffer->getSamplingRate());
			return 1;
		}
		int filled = (size + STREAM_BUFFER_SIZE - 1) / STREAM_BUFFER_SIZE;
		unsigned char* stream = this->buffer->getStream();
		unsigned int format = (this->buffer->getChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16);
		int samplingRate = this->buffer->getSamplingRate();
		for_iter (i, 0, filled)
		{
			alBufferData(this->bufferIds[(index + i) % STREAM_BUFFER_COUNT], format,
				&stream[i * STREAM_BUFFER_SIZE], hmin(size, STREAM_BUFFER_SIZE), samplingRate);
			size -= STREAM_BUFFER_SIZE;
		}
		return filled;
	}

	void OpenAL_Player::_queueBuffers(int index, int count)
	{
		if (index + count <= STREAM_BUFFER_COUNT)
		{
			alSourceQueueBuffers(this->sourceId, count, &this->bufferIds[index]);
		}
		else
		{
			alSourceQueueBuffers(this->sourceId, STREAM_BUFFER_COUNT - index, &this->bufferIds[index]);
			alSourceQueueBuffers(this->sourceId, count + index - STREAM_BUFFER_COUNT, this->bufferIds);
		}
	}
 
	void OpenAL_Player::_queueBuffers()
	{
		int queued = this->_getQueuedBuffersCount();
		if (queued < STREAM_BUFFER_COUNT)
		{
			this->_queueBuffers(this->bufferIndex, STREAM_BUFFER_COUNT - queued);
		}
	}
 
	void OpenAL_Player::_unqueueBuffers(int index, int count)
	{
		if (index + count <= STREAM_BUFFER_COUNT)
		{
#ifdef _IOS // needed for ios because in IOS 5 alSourceUnqueueBuffers doesn't lock the thread and returns before all requested buffers were unqueued
			int n = this->_getQueuedBuffersCount();
#endif
			alSourceUnqueueBuffers(this->sourceId, count, &this->bufferIds[index]);
#ifdef _IOS
			while (n - this->_getQueuedBuffersCount() != count)
			{
				hthread::sleep(1);
			}
#endif
		}
		else
		{
#ifdef _IOS
			int n = this->_getQueuedBuffersCount();
#endif
			alSourceUnqueueBuffers(this->sourceId, STREAM_BUFFER_COUNT - index, &this->bufferIds[index]);
#ifdef _IOS
			while (n - this->_getQueuedBuffersCount() != STREAM_BUFFER_COUNT - index)
			{
				hthread::sleep(1);
			}
			n -= STREAM_BUFFER_COUNT - index;
#endif
			alSourceUnqueueBuffers(this->sourceId, count + index - STREAM_BUFFER_COUNT, this->bufferIds);
#ifdef _IOS
			while (n - this->_getQueuedBuffersCount() != count + index - STREAM_BUFFER_COUNT)
			{
				hthread::sleep(1);
			}
#endif
		}
	}

	void OpenAL_Player::_unqueueBuffers()
	{
		int queued = this->_getQueuedBuffersCount();
		if (queued > 0)
		{
			this->_unqueueBuffers((this->bufferIndex + STREAM_BUFFER_COUNT - queued) % STREAM_BUFFER_COUNT, queued);
		}
	}

}
#endif
