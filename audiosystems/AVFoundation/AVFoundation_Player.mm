/// @file
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if 1

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
#include <string.h>

#ifndef __APPLE__
#include <AL/al.h>
#else
#include <OpenAL/al.h>
#endif

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "AVFoundation_AudioManager.h"
#include "AVFoundation_Player.h"
#include "Sound.h"

namespace xal
{
	AVFoundation_Player::AVFoundation_Player(Sound* sound, Buffer* buffer) :
		Player(sound, buffer)
	{
	}

	AVFoundation_Player::~AVFoundation_Player()
	{
	}

	bool AVFoundation_Player::_sysIsPlaying()
	{
		/*
		if (this->sound->isStreamed())
		{
			return (this->_getQueuedBuffersCount() > 0 || this->_getProcessedBuffersCount() > 0);
		}
		int state;
		alGetSourcei(this->sourceId, AL_SOURCE_STATE, &state);
		return (state == AL_PLAYING);
		 */
		return false;
	}

	float AVFoundation_Player::_sysGetOffset()
	{
		/*
		float offset;
		alGetSourcef(this->sourceId, AL_SAMPLE_OFFSET, &offset);
		return offset;
		 */
		return 0;
	}

	void AVFoundation_Player::_sysSetOffset(float value)
	{
		/*
		// TODO - should be int
		alSourcef(this->sourceId, AL_SAMPLE_OFFSET, value);
		//alSourcei(this->sourceId, AL_SAMPLE_OFFSET, value);
		 */
		
		// FIXME stub
	}

	bool AVFoundation_Player::_sysPreparePlay()
	{
		/*
		if (this->sourceId == 0)
		{
			this->sourceId = ((OpenAL_AudioManager*)xal::mgr)->_allocateSourceId();
		}
		return (this->sourceId != 0);*/
		return false;
	}

	void AVFoundation_Player::_sysPrepareBuffer()
	{
		/*
		// making sure all buffer data is loaded before accessing anything
		unsigned int format = (this->buffer->getChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16);
		int samplingRate = this->buffer->getSamplingRate();
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
		 */
		
		// FIXME stub
	}

	void AVFoundation_Player::_sysUpdateGain()
	{
		/*
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcGain());
		}
		 */
		
		// FIXME stub
	}

	void AVFoundation_Player::_sysUpdateFadeGain()
	{
		/*
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcFadeGain());
		}
		 */
		
		// FIXME stub
	}

	void AVFoundation_Player::_sysPlay()
	{
		/*
		alSourcePlay(this->sourceId);
		 */
		
		// FIXME stub
	}

	void AVFoundation_Player::_sysStop()
	{
		/*
		if (this->sourceId != 0)
		{
			int processed = this->_getProcessedBuffersCount();
			int queued = this->_getQueuedBuffersCount();
			alSourceStop(this->sourceId);
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
			((OpenAL_AudioManager*)xal::mgr)->_releaseSourceId(this->sourceId);
			this->sourceId = 0;
		}
		 */
		
		// FIXME stub
	}

	void AVFoundation_Player::_sysUpdateStream()
	{
		/*
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
		 */
		
		// FIXME stub
	}


}
#endif

#endif
