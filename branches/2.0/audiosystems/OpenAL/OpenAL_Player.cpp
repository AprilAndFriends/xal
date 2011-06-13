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
	}

	OpenAL_Player::~OpenAL_Player()
	{
		if (this->bufferIds[0] != 0)
		{
			alDeleteBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
		}
	}

	void OpenAL_Player::_update(float k)
	{
		Player::_update(k);
		if (!this->_sysIsPlaying() && this->sourceId != 0)
		{
			this->_stopSound();
		}
	}

	bool OpenAL_Player::_sysIsPlaying()
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

	float OpenAL_Player::_sysGetOffset()
	{
		float offset;
		alGetSourcef(this->sourceId, AL_SAMPLE_OFFSET, &offset);
		return offset;
	}

	void OpenAL_Player::_sysSetOffset(float value)
	{
		// TODO - should be int
		alSourcef(this->sourceId, AL_SAMPLE_OFFSET, value);
		//alSourcei(this->sourceId, AL_SAMPLE_OFFSET, value);
	}

	bool OpenAL_Player::_sysPreparePlay()
	{
		if (this->sourceId == 0)
		{
			this->sourceId = ((OpenAL_AudioManager*)xal::mgr)->_allocateSourceId();
		}
		return (this->sourceId != 0);
	}

	void OpenAL_Player::_sysPrepareBuffer()
	{
		// making sure all buffer data is loaded before accessing anything
		alGenBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
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

	void OpenAL_Player::_sysUpdateGain()
	{
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcGain());
		}
	}

	void OpenAL_Player::_sysUpdateFadeGain()
	{
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcFadeGain());
		}
	}

	void OpenAL_Player::_sysPlay()
	{
		alSourcePlay(this->sourceId);
	}

	void OpenAL_Player::_sysStop()
	{
		if (this->sourceId != 0)
		{
			int processed = this->_getProcessedBuffersCount();
			alSourceStop(this->sourceId);
			alSourcei(this->sourceId, AL_BUFFER, AL_NONE); // necessary to avoid a memory leak in OpenAL
			if (this->sound->isStreamed())
			{
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
			}
			alDeleteBuffers((!this->sound->isStreamed() ? 1 : STREAM_BUFFER_COUNT), this->bufferIds);
			memset(this->bufferIds, 0, STREAM_BUFFER_COUNT * sizeof(unsigned int));
			((OpenAL_AudioManager*)xal::mgr)->_releaseSourceId(this->sourceId);
			this->sourceId = 0;
		}
	}

	void OpenAL_Player::_sysUpdateStream()
	{
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
		int queued = 0;
		alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
		return queued;
	}

	int OpenAL_Player::_getProcessedBuffersCount()
	{
		int processed = 0;
		alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
		return processed;
	}

	int OpenAL_Player::_fillBuffers(int index, int count)
	{
		int size = this->buffer->load(this->looping, count);
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
		int queued = this->_getQueuedBuffersCount();
		if (queued > 0)
		{
			this->_unqueueBuffers((this->bufferIndex + STREAM_BUFFER_COUNT - queued) % STREAM_BUFFER_COUNT, queued);
		}
	}

}
#endif
