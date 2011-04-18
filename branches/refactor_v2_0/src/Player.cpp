/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/util.h>

#include "Buffer.h"
#include "Player.h"
#include "Sound.h"

namespace xal
{
	Player::Player(Sound* sound, Buffer* buffer) : gain(1.0f), paused(false),
		looping(false), fadeSpeed(0.0f), fadeTime(0.0f), offset(0.0f)
	{
		this->sound = sound;
		this->buffer = buffer;
	}

	Player::~Player()
	{
	}

	void Player::setGain(float gain)
	{
		this->gain = hclamp(gain, 0.0f, 1.0f);
	}

    float Player::getDuration()
    {
        return this->buffer->getDuration();
    }
	
	bool Player::isPaused()
	{
		return (this->paused && !this->isFading());
	}
	
	bool Player::isFading()
	{
		return (this->fadeSpeed != 0.0f);
	}

	bool Player::isFadingIn()
	{
		return (this->fadeSpeed > 0.0f);
	}

	bool Player::isFadingOut()
	{
		return (this->fadeSpeed < 0.0f);
	}
	
	Category* Player::getCategory()
	{
		return (this->sound->getCategory());
	}

	void Player::update(float k)
	{
		//this->sound->update(k);
		if (this->isFading())
		{
			this->fadeTime += this->fadeSpeed * k;
			if (this->fadeTime >= 1.0f && this->fadeSpeed > 0.0f)
			{
				this->setGain(this->gain);
				this->fadeTime = 1.0f;
				this->fadeSpeed = 0.0f;
			}
			else if (this->fadeTime <= 0.0f && this->fadeSpeed < 0.0f)
			{
				this->fadeTime = 0.0f;
				this->fadeSpeed = 0.0f;
				if (!this->paused)
				{
					this->stop();
					return;
				}
				this->pause();
			}
			else
			{
				this->_sysUpdateFadeGain();
			}
		}
		if (!this->isPlaying() && !this->isPaused())
		{
			// TODO - stop sound, unbind, etc.
			//this->unbind();
		}
	}

	void Player::play(float fadeTime, bool looping)
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
		bool alreadyFading = this->isFading();
		if (!alreadyFading)
		{
			this->buffer->load();
			this->_sysSetBuffer(this->buffer->getChannels(), this->buffer->getRate(), this->buffer->getStream(), this->buffer->getSize());
			if (this->isPaused())
			{
				this->_sysSetOffset(this->offset);
			}
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
		this->_sysUpdateFadeGain();
		if (!alreadyFading)
		{
			this->_sysPlay();
		}
		this->paused = false;
	}

	void Player::stop(float fadeTime)
	{
		this->paused = false;
		this->_stopSound(fadeTime);
		this->offset = 0.0f;
	}

	void Player::pause(float fadeTime)
	{
		this->paused = true;
		this->_stopSound(fadeTime);
	}

	void Player::_stopSound(float fadeTime)
	{
		if (fadeTime > 0.0f)
		{
			this->fadeSpeed = -1.0f / fadeTime;
			return;
		}
		this->fadeTime = 0.0f;
		this->fadeSpeed = 0.0f;
		this->offset = this->_sysGetOffset();
		this->_sysStop();
	}

}
