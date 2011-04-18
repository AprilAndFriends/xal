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
#include "Sound2.h"

namespace xal
{
	OpenAL_Player::OpenAL_Player(Sound2* sound, Buffer* buffer, unsigned int sourceId) :
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
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->gain *
				this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain());
		}
	}

	void OpenAL_Player::play(float fadeTime, bool looping)
	{
		/*
		if (this->sourceId == 0)
		{
			this->sourceId = ((OpenAL_AudioManager*)xal::mgr)->_allocateSourceId();
			if (this->sourceId == 0)
			{
				return NULL;
			}
		}
		*/
		if (!this->paused)
		{
			this->looping = looping;
		}
		bool alreadyFading = false;//this->isFading();
		if (!alreadyFading)
		{
			this->buffer->load();
			alBufferData(this->bufferId, (this->buffer->getChannels() == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
				this->buffer->getStream(), this->buffer->getSize(), this->buffer->getRate());
			//if (this->)
			alSourcei(this->sourceId, AL_BUFFER, this->bufferId);
			alSourcei(this->sourceId, AL_LOOPING, this->looping);
			/*
			if (this->isPaused())
			{
				//alSourcef(this->sourceId, AL_SAMPLE_OFFSET, this->sampleOffset);
			}
			*/
		}
		if (fadeTime > 0.0f)
		{
			this->fadeSpeed = 1.0f / fadeTime;
		}
		else
		{
			this->fadeTime = 1.0f;
			this->fadeSpeed = 0.0f;
		}
		alSourcef(this->sourceId, AL_GAIN, this->fadeTime * this->gain *
			this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain());
		if (!alreadyFading)
		{
			alSourcePlay(this->sourceId);
		}
		this->paused = false;
	}

	void OpenAL_Player::stop(float fadeTime)
	{
		this->stopSoft(fadeTime);
		if (this->sourceId != 0 && fadeTime <= 0.0f)
		{
			//this->unbind(this->paused);
		}
	}

	void OpenAL_Player::pause(float fadeTime)
	{
		this->stopSoft(fadeTime, true);
		if (this->sourceId != 0 && fadeTime <= 0.0f)
		{
			//this->unbind(this->paused);
		}
	}

	void OpenAL_Player::stopSoft(float fadeTime, bool pause)
	{
		if (this->sourceId == 0)
		{
			return;
		}
		this->paused = pause;
		if (fadeTime > 0.0f)
		{
			this->fadeSpeed = -1.0f / fadeTime;
			return;
		}
		this->fadeTime = 0.0f;
		this->fadeSpeed = 0.0f;
		//alGetSourcef(this->sourceId, AL_SAMPLE_OFFSET, &this->sampleOffset);
		alSourceStop(this->sourceId);
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
	}

}
#endif