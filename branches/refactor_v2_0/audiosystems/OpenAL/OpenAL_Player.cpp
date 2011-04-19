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
#include "OpenAL_Player.h"
#include "Sound.h"

namespace xal
{
	OpenAL_Player::OpenAL_Player(Sound* sound, Buffer* buffer, unsigned int sourceId) :
		Player(sound, buffer)
	{
		this->sourceId = sourceId;
		alGenBuffers(1, &this->bufferId);
	}

	OpenAL_Player::~OpenAL_Player()
	{
	}

	void OpenAL_Player::setGain(float gain)
	{
		Player::setGain(gain);
		alSourcef(this->sourceId, AL_GAIN, this->gain *
			this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain());
	}

	bool OpenAL_Player::isPlaying()
	{
		/*
		if (this->sound->getCategory()->isStreamed())
		{
			int queued;
			alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
			int count;
			alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &count);
			return (queued > 0 || count > 0);
		}
		*/
		if (this->isFadingOut())
		{
			return false;
		}
		int state;
		alGetSourcei(this->sourceId, AL_SOURCE_STATE, &state);
		return (state == AL_PLAYING);
	}

	void OpenAL_Player::_sysSetOffset(float value)
	{
		alSourcef(this->sourceId, AL_SAMPLE_OFFSET, value);
	}

	float OpenAL_Player::_sysGetOffset()
	{
		float offset;
		alGetSourcef(this->sourceId, AL_SAMPLE_OFFSET, &offset);
		return offset;
	}

	void OpenAL_Player::_sysSetBuffer(unsigned int channels, unsigned int rate, unsigned char* stream, unsigned int size)
	{
		alBufferData(this->bufferId, (channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16), stream, size, rate);
		alSourcei(this->sourceId, AL_BUFFER, this->bufferId);
		alSourcei(this->sourceId, AL_LOOPING, this->looping);
	}

	void OpenAL_Player::_sysUpdateFadeGain()
	{
		alSourcef(this->sourceId, AL_GAIN, this->fadeTime * this->gain *
			this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain());
	}

	void OpenAL_Player::_sysPlay()
	{
		alSourcePlay(this->sourceId);
	}

	void OpenAL_Player::_sysStop()
	{
		alSourceStop(this->sourceId);
		/*
		if (this->sound->getCategory()->isStreamed())
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
		*/
	}

}
#endif