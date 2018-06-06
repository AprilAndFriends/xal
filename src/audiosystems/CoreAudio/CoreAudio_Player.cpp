/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _COREAUDIO
#include <stdio.h>
#include <string.h>

#include <TargetConditionals.h>
#include <CoreAudio/al.h>

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "CoreAudio_AudioManager.h"
#include "CoreAudio_Player.h"
#include "Sound.h"
#include "Utility.h"
#include "xal.h"

namespace xal
{
	CoreAudio_Player::CoreAudio_Player(Sound* sound) : Player(sound), sourceId(0)
	{
		this->pendingPitchUpdate = false;
		this->createCoreAudioBuffers();
	}
	
	CoreAudio_Player::~CoreAudio_Player()
	{
		// AudioManager calls _stop before destruction
		this->destroyCoreAudioBuffers();
	}
	
	void CoreAudio_Player::createCoreAudioBuffers()
	{
		memset(this->bufferIds, 0, STREAM_BUFFER_COUNT * sizeof(unsigned int));
		alGenBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
	}

	void CoreAudio_Player::destroyCoreAudioBuffers()
	{
		alDeleteBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
	}
	
	void CoreAudio_Player::_update(float timeDelta)
	{
		Player::_update(timeDelta);
		if (!this->_isPlaying() && this->sourceId != 0)
		{
			this->_stop();
		}
	}
	
	bool CoreAudio_Player::_systemIsPlaying() const
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
	
	unsigned int CoreAudio_Player::_systemGetBufferPosition() const
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
	
	float CoreAudio_Player::_systemGetOffset() const
	{
		float offset = 0.0f;
		if (this->sourceId != 0)
		{
			alGetSourcef(this->sourceId, AL_SAMPLE_OFFSET, &offset);
		}
		return offset;
	}
	
	void CoreAudio_Player::_systemSetOffset(float value)
	{
		if (this->sourceId != 0 && !this->sound->isStreamed()) // Hack for iOS and Mac because apple has a bug in CoreAudio and setting offset when buffers are queued messes up stuff and causes crashes.
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
	
	bool CoreAudio_Player::_systemPreparePlay()
	{
		if (this->sourceId == 0)
		{
			this->sourceId = ((CoreAudio_AudioManager*)xal::manager)->_allocateSourceId();
		}
		return (this->sourceId != 0);
	}
	
	void CoreAudio_Player::_systemPrepareBuffer()
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
	
	void CoreAudio_Player::_systemUpdateGain()
	{
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcGain());
		}
	}
	
	void CoreAudio_Player::_systemUpdatePitch()
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
	
	void CoreAudio_Player::_systemPlay()
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
	
	int CoreAudio_Player::_systemStop()
	{
		int result = 0;
		if (this->sourceId != 0)
		{
			if (!this->sound->isStreamed())
			{
				alSourceStop(this->sourceId);
				alSourcei(this->sourceId, AL_BUFFER, AL_NONE); // necessary to avoid a memory leak in CoreAudio
			}
			else
			{
				int processed = this->_getProcessedBuffersCount();
				alSourceStop(this->sourceId);
#ifndef _IOS // hack for iOS only, has problems when audio is suspended
				this->_unqueueBuffers();
#endif
				alSourcei(this->sourceId, AL_BUFFER, AL_NONE); // necessary to avoid a memory leak in CoreAudio
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
			((CoreAudio_AudioManager*)xal::manager)->_releaseSourceId(this->sourceId);
			this->sourceId = 0;
		}
		return result;
	}
	
	int CoreAudio_Player::_systemUpdateStream()
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
			if (!playing) // underrun happened, sound was stopped by CoreAudio so let's reboot it properly
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
			processed = 0;
		}
		return (processed * STREAM_BUFFER_SIZE);
	}
	
	int CoreAudio_Player::_getQueuedBuffersCount() const
	{
		int queued = 0;
		if (this->sourceId)
		{
			alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
		}
		return queued;
	}
	
	int CoreAudio_Player::_getProcessedBuffersCount() const
	{
		int processed = 0;
		if (this->sourceId)
		{
			alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
		}
		return processed;
	}
	
	int CoreAudio_Player::_fillBuffers(int index, int count)
	{
		int size = this->buffer->load(this->looping, count * STREAM_BUFFER_SIZE);
		hstream& stream = this->buffer->getStream();
		if (!this->sound->isStreamed())
		{
			alBufferData(this->bufferIds[index], (this->buffer->getChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16),
				(unsigned char*)stream, size, this->buffer->getSamplingRate());
			return 1;
		}
		int filled = (size + STREAM_BUFFER_SIZE - 1) / STREAM_BUFFER_SIZE;
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
	
	void CoreAudio_Player::_queueBuffers(int index, int count)
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
 	
	void CoreAudio_Player::_queueBuffers()
	{
		int queued = this->_getQueuedBuffersCount();
		if (queued < STREAM_BUFFER_COUNT)
		{
			this->_queueBuffers(this->bufferIndex, STREAM_BUFFER_COUNT - queued);
		}
	}
 	
	void CoreAudio_Player::_unqueueBuffers(int index, int count)
	{
		int n = this->_getQueuedBuffersCount();
		int safeWait = 50;
		if (index + count <= STREAM_BUFFER_COUNT)
		{
			alSourceUnqueueBuffers(this->sourceId, count, &this->bufferIds[index]);
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
		}
		else
		{
			alSourceUnqueueBuffers(this->sourceId, STREAM_BUFFER_COUNT - index, &this->bufferIds[index]);
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
			alSourceUnqueueBuffers(this->sourceId, count + index - STREAM_BUFFER_COUNT, this->bufferIds);
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
		}
	}
	
	void CoreAudio_Player::_unqueueBuffers()
	{
		int queued = this->_getQueuedBuffersCount();
		if (queued > 0)
		{
			this->_unqueueBuffers((this->bufferIndex + STREAM_BUFFER_COUNT - queued) % STREAM_BUFFER_COUNT, queued);
		}
	}
	
}
#endif
