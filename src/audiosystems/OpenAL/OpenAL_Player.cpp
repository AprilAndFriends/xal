/// @file
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENAL
#include <stdio.h>
#include <string.h>

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
#include "xal.h"

namespace xal
{
	OpenAL_Player::OpenAL_Player(Sound* sound) : Player(sound), sourceId(0)
	{
        this->pendingPitchUpdate = false;
		this->createOpenALBuffers();
#ifdef _MAC
		this->macSampleOffset = -1;
#endif
	}
	
	OpenAL_Player::~OpenAL_Player()
	{
		// AudioManager calls _stop before destruction
		this->destroyOpenALBuffers();
	}
	
	void OpenAL_Player::createOpenALBuffers()
	{
		memset(this->bufferIds, 0, STREAM_BUFFER_COUNT * sizeof(unsigned int));
		alGenBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
	}

	void OpenAL_Player::destroyOpenALBuffers()
	{
		alDeleteBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
	}
	
	void OpenAL_Player::_update(float timeDelta)
	{
		Player::_update(timeDelta);
		if (!this->_systemIsPlaying() && this->sourceId != 0)
		{
			this->_stop();
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
	
	unsigned int OpenAL_Player::_systemGetBufferPosition()
	{
		int bytes = 0;
		if (this->sourceId != 0)
		{
			alGetSourcei(this->sourceId, AL_BYTE_OFFSET, &bytes);
		}
		if (!this->sound->isStreamed())
		{
			return bytes;
		}
		return ((bytes + this->bufferIndex * STREAM_BUFFER_SIZE) % STREAM_BUFFER);
	}
	
	float OpenAL_Player::_systemGetOffset()
	{
		float offset = 0.0f;
		if (this->sourceId != 0)
		{
			alGetSourcef(this->sourceId, AL_SAMPLE_OFFSET, &offset);
		}
		return offset;
	}
	
	void OpenAL_Player::_systemSetOffset(float value)
	{
#ifndef __APPLE__
		if (this->sourceId != 0)
#else
		if (this->sourceId != 0 && !this->sound->isStreamed()) // Hack for iOS and Mac because apple has a bug in OpenAL and setting offset when buffers are queued messes up stuff and causes crashes.
#endif
		{
#ifdef _MAC
			// This is a necessarry mac-specific work-arround, setting sample offset on sources that aren't playing on MacOS doesn't work.
			// It's a know bug and apple won't fix it. However, setting it on playing sources works. So if a source isn't playing, queue the request
			// until the next call to alSourcePlay.
			int state;
			alGetSourcei(this->sourceId, AL_SOURCE_STATE, &state);
			if (state == AL_PLAYING)
			{
				alSourcef(this->sourceId, AL_SAMPLE_OFFSET, value);
			}
			else
			{
				this->macSampleOffset = value;
			}
#else
			alSourcef(this->sourceId, AL_SAMPLE_OFFSET, value);
#endif
		}
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
			int count = STREAM_BUFFER_COUNT - this->_getQueuedBuffersCount();
			if (count > 0)
			{
				count = this->_fillBuffers(this->bufferIndex, count);
				if (count > 0)
				{
					this->_queueBuffers(this->bufferIndex, count);
					this->bufferIndex = (this->bufferIndex + count) % STREAM_BUFFER_COUNT;
				}
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
	
	void OpenAL_Player::_systemUpdatePitch()
	{
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_PITCH, this->pitch);
		}
        else
        {
            this->pendingPitchUpdate = true;
        }
	}
	
	void OpenAL_Player::_systemPlay()
	{
		if (this->sourceId != 0)
		{
			alSourcePlay(this->sourceId);
#ifdef _MAC
			if (this->macSampleOffset >= 0)
			{
				alSourcef(this->sourceId, AL_SAMPLE_OFFSET, this->macSampleOffset);
				this->macSampleOffset = -1;
			}
#endif
            if (this->pendingPitchUpdate)
            {
                this->pendingPitchUpdate = false;
                alSourcef(this->sourceId, AL_PITCH, this->pitch);
            }
		}
	}
	
	int OpenAL_Player::_systemStop()
	{
		int result = 0;
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
#ifndef _IOS // hack for iOS only, has problems when audio is suspended
				this->_unqueueBuffers();
#endif
				alSourcei(this->sourceId, AL_BUFFER, AL_NONE); // necessary to avoid a memory leak in OpenAL
				if (this->paused)
				{
					this->bufferIndex = (this->bufferIndex + processed) % STREAM_BUFFER_COUNT;
					result = processed * STREAM_BUFFER_SIZE;
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
		return result;
	}
	
	int OpenAL_Player::_systemUpdateStream()
	{
		int queued = this->_getQueuedBuffersCount();
		if (queued == 0)
		{
			this->_stop();
			return 0;
		}
		int processed = this->_getProcessedBuffersCount();
		if (processed == 0)
		{
			return 0;
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
				float speed = this->fadeSpeed;
				float time = this->fadeTime;
				this->_pause();
				this->_play();
				this->fadeSpeed = speed;
				this->fadeTime = time;
			}
		}
		if (this->_getQueuedBuffersCount() == 0)
		{
			this->_stop();
		}
		return (processed * STREAM_BUFFER_SIZE);
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
#ifdef __APPLE__ // needed for ios because in IOS 5 alSourceUnqueueBuffers doesn't lock the thread and returns before all requested buffers were unqueued, Also seldomly causes crashes on MacOS
		int n = this->_getQueuedBuffersCount();
		int safeWait = 50;
#endif
		if (index + count <= STREAM_BUFFER_COUNT)
		{
			alSourceUnqueueBuffers(this->sourceId, count, &this->bufferIds[index]);
#ifdef __APPLE__
			while (n - this->_getQueuedBuffersCount() != count && safeWait > 0)
			{
				hthread::sleep(1);
				--safeWait;
			}
			if (safeWait == 0) // I am so going to burn for this.
			{
				// let's hope the underrun detection will realize what's going on and restart the sound
				alSourceStop(this->sourceId);
				alSourcei(this->sourceId, AL_BUFFER, AL_NONE);
			}
#endif
		}
		else
		{
			alSourceUnqueueBuffers(this->sourceId, STREAM_BUFFER_COUNT - index, &this->bufferIds[index]);
#ifdef __APPLE__
			while (n - this->_getQueuedBuffersCount() != STREAM_BUFFER_COUNT - index && safeWait > 0)
			{
				hthread::sleep(1);
				--safeWait;
			}
			if (safeWait == 0) // iOS hacking at its prime.
			{
				// let's hope the underrun detection will realize what's going on and restart the sound
				alSourceStop(this->sourceId);
				alSourcei(this->sourceId, AL_BUFFER, AL_NONE);
				return;
			}
			n -= STREAM_BUFFER_COUNT - index;
#endif
			alSourceUnqueueBuffers(this->sourceId, count + index - STREAM_BUFFER_COUNT, this->bufferIds);
#ifdef __APPLE__
			safeWait = 50;
			while (n - this->_getQueuedBuffersCount() != count + index - STREAM_BUFFER_COUNT && safeWait > 0)
			{
				hthread::sleep(1);
				--safeWait;
			}
			if (safeWait == 0) // Apple makes me resort to horrible hacks like this. :(
			{
				// let's hope the underrun detection will realize what's going on and restart the sound
				alSourceStop(this->sourceId);
				alSourcei(this->sourceId, AL_BUFFER, AL_NONE);
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
