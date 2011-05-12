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
#include "Category.h"
#include "Player.h"
#include "Sound.h"

namespace xal
{
	Player::Player(Sound* sound, Buffer* buffer) : gain(1.0f), paused(false), looping(false),
		fadeSpeed(0.0f), fadeTime(0.0f), offset(0.0f), bufferIndex(0)
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
		this->_sysUpdateGain();
	}

	hstr Player::getName()
	{
		return this->sound->getName();
	}

	hstr Player::getFilename()
	{
		return this->sound->getFilename();
	}

	hstr Player::getRealFilename()
	{
		return this->sound->getRealFilename();
	}

    float Player::getDuration()
    {
        return this->buffer->getDuration();
    }
	
	bool Player::isPlaying()
	{
		return (!this->isFadingOut() && this->_sysIsPlaying());
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
		if (this->sound->isStreamed() && this->_sysIsPlaying())
		{
			this->_sysUpdateStream();
		}
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
	}

	void Player::play(float fadeTime, bool looping)
	{
		if (!xal::mgr->isEnabled())
		{
			return;
		}
		if (!this->_sysPreparePlay())
		{
			return;
		}
		if (!this->paused)
		{
			this->looping = looping;
		}
		bool alreadyFading = this->isFading();
		if (!alreadyFading)
		{
			this->_sysPrepareBuffer();
			if (this->paused)
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
		this->buffer->release();
		this->_sysStop();
	}

	float Player::_calcGain()
	{
		return (this->gain * this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain());
	}

	float Player::_calcFadeGain()
	{
		return (this->gain * this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain() * this->fadeTime);
	}

}
