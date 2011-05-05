/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_OPENAL
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
		Category* category = sound->getCategory();
		this->bufferIds[1] = 0;
		alGenBuffers((!this->sound->isStreamed() ? 1 : 2), &this->bufferIds[0]);
	}

	OpenAL_Player::~OpenAL_Player()
	{
		alDeleteBuffers((!this->sound->isStreamed() ? 1 : 2), &this->bufferIds[0]);
	}

	void OpenAL_Player::setGain(float gain)
	{
		Player::setGain(gain);
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->_calcGain());
		}
	}

	bool OpenAL_Player::isPlaying()
	{
		if (this->sound->isStreamed())
		{
			int queued;
			alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
			int count;
			alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &count);
			return (queued > 0 || count > 0);
		}
		if (this->isFadingOut())
		{
			return false;
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
			this->sourceId = ((OpenAL_AudioManager*)xal::mgr)->allocateSourceId();
		}
		return (this->sourceId != 0);
	}

	void OpenAL_Player::_sysPrepareBuffer()
	{
		if (!this->sound->isStreamed())
		{
			alSourcei(this->sourceId, AL_BUFFER, this->bufferIds[0]);
			alSourcei(this->sourceId, AL_LOOPING, this->looping);
			unsigned char* stream;
			int bufferSize = this->buffer->getSize();
			int size = this->buffer->getData((int)this->offset, bufferSize, &stream);
			alBufferData(this->bufferIds[0], (this->buffer->getChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16),
				this->buffer->getStream(), this->buffer->getSize(), this->buffer->getSamplingRate());
		}
		else
		{
			// TODO - implement streaming buffer queueing
			/*
			alSourcei(this->sourceId, AL_BUFFER, 0);
			alSourcei(this->sourceId, AL_LOOPING, false);
			unsigned char* stream;
			int bufferSize = this->buffer->getSize();
			int size = this->buffer->getData((int)this->offset, bufferSize, &stream);
			alBufferData(this->bufferIds[0], (this->buffer->getChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16),
				stream, size, this->buffer->getSamplingRate());
			*/
			/*
			alSourcei(this->sourceId, AL_BUFFER, 0);
			alSourcei(this->sourceId, AL_LOOPING, false);
			this->sound->setSourceId(this->sourceId);
			((StreamSound*)this->sound)->queueBuffers();
			*/
		}
	}

	void OpenAL_Player::_sysUpdateBuffer()
	{
		//if one of two buffers empty
		//    this->buffer->update(this->offset);
		//    fill buffer with new data
	}

	void OpenAL_Player::_sysUpdateFadeGain()
	{
		alSourcef(this->sourceId, AL_GAIN, this->_calcFadeGain());
	}

	void OpenAL_Player::_sysPlay()
	{
		alSourcePlay(this->sourceId);
	}

	void OpenAL_Player::_sysStop()
	{
		if (this->sourceId != 0)
		{
			alSourceStop(this->sourceId);
			((OpenAL_AudioManager*)xal::mgr)->releaseSourceId(this->sourceId);
			this->sourceId = 0;
			if (this->sound->isStreamed())
			{
				//this->sound->setSourceId(this->sourceId);
				if (this->paused)
				{
					//((StreamSound*)this->sound)->unqueueBuffers();
				}
				else
				{
					//((StreamSound*)this->sound)->rewindStream();
				}
			}
		}
	}

}
#endif