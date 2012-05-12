/// @file
/// @author  Boris Mikic
/// @version 2.33
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hltypesUtil.h>

#include "Buffer.h"
#include "Category.h"
#include "Player.h"
#include "Sound.h"

namespace xal
{
	Player::Player(Sound* sound, Buffer* buffer) : gain(1.0f), pitch(1.0f), paused(false),
		looping(false), fadeSpeed(0.0f), fadeTime(0.0f), offset(0.0f), bufferIndex(0)
	{
		this->sound = sound;
		this->buffer = buffer;
	}

	Player::~Player()
	{
	}

	float Player::getPlaybackPosition()
	{
		float size = (this->buffer->getSamplingRate() * this->buffer->getChannels() * this->buffer->getBitsPerSample() / 8.0f);
		//return this->buffer->getBitsPerSample();
		return this->_systemGetOffset() / size;
	}
	
	float Player::getGain()
	{
		xal::mgr->_lock();
		float gain = this->_getGain();
		xal::mgr->_unlock();
		return gain;
	}

	float Player::_getGain()
	{
		return this->gain;
	}

	void Player::setGain(float value)
	{
		xal::mgr->_lock();
		this->_setGain(value);
		xal::mgr->_unlock();
	}

	void Player::_setGain(float value)
	{
		this->gain = hclamp(value, 0.0f, 1.0f);
		this->_systemUpdateGain();
	}

	float Player::getPitch()
	{
		xal::mgr->_lock();
		float pitch = this->_getPitch();
		xal::mgr->_unlock();
		return pitch;
	}

	float Player::_getPitch()
	{
		return this->pitch;
	}

	void Player::setPitch(float value)
	{
		xal::mgr->_lock();
		this->_setPitch(value);
		xal::mgr->_unlock();
	}

	void Player::_setPitch(float value)
	{
		this->pitch = hclamp(value, 0.01f, 100.0f);
		this->_systemUpdatePitch();
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
	
    int Player::getSize()
    {
		return this->buffer->getSize();
    }
	
	bool Player::isPlaying()
	{
		return (!this->isFadingOut() && this->_systemIsPlaying());
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
		return this->sound->getCategory();
	}

	void Player::_update(float k)
	{
		if (this->sound->isStreamed() && this->_systemIsPlaying())
		{
			this->_systemUpdateStream();
		}
		if (this->isFading())
		{
			this->fadeTime += this->fadeSpeed * k;
			if (this->fadeTime >= 1.0f && this->fadeSpeed > 0.0f)
			{
				this->_setGain(this->gain);
				this->fadeTime = 1.0f;
				this->fadeSpeed = 0.0f;
			}
			else if (this->fadeTime <= 0.0f && this->fadeSpeed < 0.0f)
			{
				this->fadeTime = 0.0f;
				this->fadeSpeed = 0.0f;
				if (!this->paused)
				{
					this->_stop();
					return;
				}
				this->_pause();
			}
			else
			{
				this->_systemUpdateFadeGain();
			}
		}
	}

	void Player::play(float fadeTime, bool looping)
	{
		xal::mgr->_lock();
		this->_play(fadeTime, looping);
		xal::mgr->_unlock();
	}

	void Player::stop(float fadeTime)
	{
		xal::mgr->_lock();
		this->_stop(fadeTime);
		xal::mgr->_unlock();
	}

	void Player::pause(float fadeTime)
	{
		xal::mgr->_lock();
		this->_pause(fadeTime);
		xal::mgr->_unlock();
	}
	
	void Player::_play(float fadeTime, bool looping)
	{
		if (!xal::mgr->isEnabled())
		{
			return;
		}
		if (xal::mgr->isSuspended())
		{
			if (!xal::mgr->suspendedPlayers.contains(this))
			{
				xal::mgr->suspendedPlayers += this;
				if (!this->paused)
				{
					this->looping = looping;
					this->paused = true;
				}
			}
			return;
		}
		if (!this->_systemPreparePlay())
		{
			return;
		}
		if (!this->paused)
		{
			this->looping = looping;
		}
		bool alreadyFading = this->isFading();
		if (!alreadyFading && !this->_systemIsPlaying())
		{
			this->buffer->prepare();
			this->_systemPrepareBuffer();
			if (this->paused)
			{
				this->_systemSetOffset(this->offset);
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
		this->_systemUpdateFadeGain();
		if (!alreadyFading)
		{
			this->_systemPlay();
		}
		this->paused = false;
	}

	void Player::_stop(float fadeTime)
	{
		if (xal::mgr->isSuspended() && xal::mgr->suspendedPlayers.contains(this))
		{
			xal::mgr->suspendedPlayers -= this;
		}
		this->paused = false;
		this->_stopSound(fadeTime);
		this->offset = 0.0f;
	}

	void Player::_pause(float fadeTime)
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
		this->offset = this->_systemGetOffset();
		this->_systemStop();
		this->buffer->release();
		this->fadeTime = 0.0f;
		this->fadeSpeed = 0.0f;
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
