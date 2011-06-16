/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_OPENAL
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

#define OALP_LOG_FUNCTION_NAME() //printf("%s\n", __PRETTY_FUNCTION__);
#define OALP_ERROR_GUARD(call, onfail) \
	{ \
		call; \
		int er = alGetError(); \
		if (er != AL_NO_ERROR) \
		{ \
			printf("OpenAL ERROR %d!\n", er); \
			onfail; \
		} \
	}

namespace xal
{
	OpenAL_Player::OpenAL_Player(Sound* sound, Buffer* buffer) :
		Player(sound, buffer), sourceId(0)
	{
		OALP_LOG_FUNCTION_NAME();
		memset(this->bufferIds, 0, STREAM_BUFFER_COUNT * sizeof(unsigned int));
		alGenBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
	}

	OpenAL_Player::~OpenAL_Player()
	{
		OALP_LOG_FUNCTION_NAME();
		alDeleteBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
	}

	void OpenAL_Player::_update(float k)
	{
		OALP_LOG_FUNCTION_NAME();
		Player::_update(k);
		if (!this->_sysIsPlaying() && this->sourceId != 0)
		{
			this->_stopSound();
		}
	}

	bool OpenAL_Player::_sysIsPlaying()
	{
		OALP_LOG_FUNCTION_NAME();
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

	float OpenAL_Player::_sysGetOffset()
	{
		OALP_LOG_FUNCTION_NAME();
		float offset = 0;
#if !TARGET_OS_MAC
		alGetSourcef(this->sourceId, AL_SAMPLE_OFFSET, &offset);
#else
		// did not find anything that works on Mac OS X and iOS!
#endif
		return offset;
	}

	void OpenAL_Player::_sysSetOffset(float value)
	{
		OALP_LOG_FUNCTION_NAME();
#if !TARGET_OS_MAC
		// TODO - should be int
		alSourcef(this->sourceId, AL_SAMPLE_OFFSET, value);		
		//alSourcei(this->sourceId, AL_SAMPLE_OFFSET, value);
#else
		// did not find anything that works on Mac OS X and iOS!
#endif
	}

	bool OpenAL_Player::_sysPreparePlay()
	{
		OALP_LOG_FUNCTION_NAME();
		if (this->sourceId == 0)
		{
			this->sourceId = ((OpenAL_AudioManager*)xal::mgr)->_allocateSourceId();
		}
		return (this->sourceId != 0);
	}

	void OpenAL_Player::_sysPrepareBuffer()
	{
		OALP_LOG_FUNCTION_NAME();
		// making sure all buffer data is loaded before accessing anything
		if (!this->sound->isStreamed())
		{
			bool failed = false;
			this->_fillBuffers(0, 1);
			if(!failed) OALP_ERROR_GUARD(alSourcei(this->sourceId, AL_BUFFER, this->bufferIds[0]), failed = true);
			if(!failed) OALP_ERROR_GUARD(alSourcei(this->sourceId, AL_LOOPING, this->looping), failed = true);
			
			if(failed)
			{
				alSourcei(this->sourceId, AL_BUFFER, AL_NONE);
				alSourcei(this->sourceId, AL_LOOPING, false);
			}
			
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

	void OpenAL_Player::_sysUpdateGain()
	{
		OALP_LOG_FUNCTION_NAME();
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcGain());
		}
	}

	void OpenAL_Player::_sysUpdateFadeGain()
	{
		OALP_LOG_FUNCTION_NAME();
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcFadeGain());
		}
	}

	void OpenAL_Player::_sysPlay()
	{
		OALP_LOG_FUNCTION_NAME();
		if (this->sourceId != 0)
		{
			int er = alGetError();
			if (er != AL_NO_ERROR) 
			{
				printf("OpenAL Error before play: %d\n", er);
			}
			else
			{
				alSourcePlay(this->sourceId);
			}
		}
	}

	void OpenAL_Player::_sysStop()
	{
		OALP_LOG_FUNCTION_NAME();
		if (this->sourceId != 0)
		{
			int processed = this->_getProcessedBuffersCount();
			alSourceStop(this->sourceId);
			alSourcei(this->sourceId, AL_BUFFER, AL_NONE); // necessary to avoid a memory leak in OpenAL
			if (this->sound->isStreamed())
			{
				alSourceStop(this->sourceId);
				alSourcei(this->sourceId, AL_BUFFER, AL_NONE); // necessary to avoid a memory leak in OpenAL
			}
			else
			{
				int processed = this->_getProcessedBuffersCount();
				alSourceStop(this->sourceId);
				this->_unqueueBuffers();
				if (this->paused)
				{
					this->bufferIndex = (this->bufferIndex + processed) % STREAM_BUFFER_COUNT;
				}
				else
				{
					this->bufferIndex = 0;
					this->buffer->rewind();
				}
				alSourcei(this->sourceId, AL_BUFFER, AL_NONE); // necessary to avoid a memory leak in OpenAL
			}
			((OpenAL_AudioManager*)xal::mgr)->_releaseSourceId(this->sourceId);
			this->sourceId = 0;
		}
	}

	void OpenAL_Player::_sysUpdateStream()
	{
		OALP_LOG_FUNCTION_NAME();
		// TODO - assert this is a streamed sound
		
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
			if (processed < STREAM_BUFFER_COUNT)
			{
				this->bufferIndex = (this->bufferIndex + count) % STREAM_BUFFER_COUNT;
			}
			else // underrun happened, sound was stopped
			{
				this->pause();
				this->play();
			}
		}
		if (this->_getQueuedBuffersCount() == 0)
		{
			this->_stopSound();
		}
	}

	int OpenAL_Player::_getQueuedBuffersCount()
	{
		OALP_LOG_FUNCTION_NAME();
		int queued = 0;
		alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
		return queued;
	}

	int OpenAL_Player::_getProcessedBuffersCount()
	{
		OALP_LOG_FUNCTION_NAME();
		int processed = 0;
		alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
		return processed;
	}

	int OpenAL_Player::_fillBuffers(int index, int count)
	{
		OALP_LOG_FUNCTION_NAME();
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
		for (int i = 0; i < filled; i++)
		{
			alBufferData(this->bufferIds[(index + i) % STREAM_BUFFER_COUNT], format,
				&stream[i * STREAM_BUFFER_SIZE], hmin(size, STREAM_BUFFER_SIZE), samplingRate);
			size -= STREAM_BUFFER_SIZE;
		}
		return filled;
	}

	void OpenAL_Player::_queueBuffers(int index, int count)
	{
		OALP_LOG_FUNCTION_NAME();
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
		OALP_LOG_FUNCTION_NAME();
		int queued = this->_getQueuedBuffersCount();
		if (queued < STREAM_BUFFER_COUNT)
		{
			this->_queueBuffers(this->bufferIndex, STREAM_BUFFER_COUNT - queued);
		}
	}
 
	void OpenAL_Player::_unqueueBuffers(int index, int count)
	{
		OALP_LOG_FUNCTION_NAME();
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

	void OpenAL_Player::_unqueueBuffers()
	{
		OALP_LOG_FUNCTION_NAME();
		int queued = this->_getQueuedBuffersCount();
		if (queued > 0)
		{
			this->_unqueueBuffers((this->bufferIndex + STREAM_BUFFER_COUNT - queued) % STREAM_BUFFER_COUNT, queued);
		}
	}

}
#endif
