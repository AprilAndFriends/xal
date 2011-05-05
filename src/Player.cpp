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
	}

	hstr Player::getName()
	{
		return this->sound->getName();
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
		if (this->isPlaying())
		{
			this->_updateBuffer();
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

	void Player::_updateBuffer()
	{
		int queued = this->_sysGetQueuedBuffersCount();
		if (queued == 0)
		{
			this->_stopSound();
			return;
		}
		int processed = this->_sysGetProcessedBuffersCount();
		if (processed == 0)
		{
			return;
		}
		this->_sysUnqueueBuffers((this->bufferIndex + STREAM_BUFFER_COUNT - queued) % STREAM_BUFFER_COUNT, processed);
		int bytes = 0;
		int result;
		int i = 0;
		unsigned char* data;
		for (; i < processed; i++)
		{
			result = this->buffer->getData(STREAM_BUFFER_SIZE, &data);
			this->__sysSetBufferData(i, data, result);
			if (result == 0)
			{
				break;
			}
			bytes += result;
		}
		if (bytes > 0)
		{
			this->_sysQueueBuffers(this->bufferIndex, i);
			if (processed < STREAM_BUFFER_COUNT)
			{
				this->bufferIndex = (this->bufferIndex + i) % STREAM_BUFFER_COUNT;
			}
			else // underrun happened, sound was stopped
			{
				this->pause();
				this->play();
			}
		}
		if (this->_sysGetQueuedBuffersCount() == 0)
		{
			this->_stopSound();
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
			// making sure the buffer is prepared (with a loaded and decoded source)
			if (this->isPaused())
			{
				this->buffer->prepare(this->offset);
				this->_sysPrepareBuffer();
				this->_sysSetOffset(this->offset);
			}
			else
			{
				this->buffer->prepare();
				this->_sysPrepareBuffer();
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

	void Player::_sysStop()
	{
		if (this->sound->isStreamed())
		{
			this->paused ? this->_sysUnqueueBuffers() : this->buffer->rewind();
		}
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
