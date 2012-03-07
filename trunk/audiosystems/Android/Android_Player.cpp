/// @file
/// @author  Boris Mikic
/// @version 2.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_ANDROID
#include <stdio.h>
#include <string.h>
#include "xal.h"

#include <AL/al.h>

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "Android_AudioManager.h"
#include "Android_Player.h"
#include "Sound.h"

namespace xal
{
	Android_Player::Android_Player(Sound* sound, Buffer* buffer) :
		Player(sound, buffer), sourceId(0)
	{
		memset(this->bufferIds, 0, STREAM_BUFFER_COUNT * sizeof(unsigned int));
		alGenBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
	}

	Android_Player::~Android_Player()
	{
		// AudioManager calls _stop before destruction
		alDeleteBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
	}

	void Android_Player::_update(float k)
	{
		Player::_update(k);
		if (!this->_systemIsPlaying() && this->sourceId != 0)
		{
			this->_stopSound();
		}
	}

	bool Android_Player::_systemIsPlaying()
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

	float Android_Player::_systemGetOffset()
	{
		float offset = 0.0f;
		if (this->sourceId != 0)
		{
			alGetSourcef(this->sourceId, AL_SAMPLE_OFFSET, &offset);
		}
		return offset;
	}

	void Android_Player::_systemSetOffset(float value)
	{
		// TODO - should be int
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_SAMPLE_OFFSET, value);
		}
		//alSourcei(this->sourceId, AL_SAMPLE_OFFSET, value);
	}

	bool Android_Player::_systemPreparePlay()
	{
		if (this->sourceId == 0)
		{
			this->sourceId = ((Android_AudioManager*)xal::mgr)->_allocateSourceId();
		}
		return (this->sourceId != 0);
	}

	void Android_Player::_systemPrepareBuffer()
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

	void Android_Player::_systemUpdateGain()
	{
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcGain());
		}
	}

	void Android_Player::_systemUpdateFadeGain()
	{
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcFadeGain());
		}
	}

	void Android_Player::_systemPlay()
	{
		if (this->sourceId != 0)
		{
			alSourcePlay(this->sourceId);
		}
	}

	void Android_Player::_systemStop()
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
			((Android_AudioManager*)xal::mgr)->_releaseSourceId(this->sourceId);
			this->sourceId = 0;
		}
	}

	void Android_Player::_systemUpdateStream()
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

	int Android_Player::_getQueuedBuffersCount()
	{
		int queued = 0;
		if (this->sourceId)
		{
			alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
		}
		return queued;
	}

	int Android_Player::_getProcessedBuffersCount()
	{
		int processed = 0;
		if (this->sourceId)
		{
			alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
		}
		return processed;
	}

	int Android_Player::_fillBuffers(int index, int count)
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

	void Android_Player::_queueBuffers(int index, int count)
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
 
	void Android_Player::_queueBuffers()
	{
		int queued = this->_getQueuedBuffersCount();
		if (queued < STREAM_BUFFER_COUNT)
		{
			this->_queueBuffers(this->bufferIndex, STREAM_BUFFER_COUNT - queued);
		}
	}
 
	void Android_Player::_unqueueBuffers(int index, int count)
	{
		if (index + count <= STREAM_BUFFER_COUNT)
		{
			alSourceUnqueueBuffers(this->sourceId, count, &this->bufferIds[index]);
		}
		else
		{
			alSourceUnqueueBuffers(this->sourceId, STREAM_BUFFER_COUNT - index, &this->bufferIds[index]);
			alSourceUnqueueBuffers(this->sourceId, count + index - STREAM_BUFFER_COUNT, this->bufferIds);
		}
	}

	void Android_Player::_unqueueBuffers()
	{
		int queued = this->_getQueuedBuffersCount();
		if (queued > 0)
		{
			this->_unqueueBuffers((this->bufferIndex + STREAM_BUFFER_COUNT - queued) % STREAM_BUFFER_COUNT, queued);
		}
	}

}
#endif
