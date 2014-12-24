/// @file
/// @version 3.32
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>

#include "Source.h"
#include "Buffer.h"
#include "Category.h"
#include "Player.h"
#include "Sound.h"
#include "xal.h"

namespace xal
{
	Player::Player(Sound* sound) : gain(1.0f), pitch(1.0f), paused(false), looping(false), fadeSpeed(0.0f),
		fadeTime(0.0f), offset(0.0f), bufferIndex(0), processedByteCount(0), idleTime(0.0f), asyncPlayQueued(false)
	{
		this->sound = sound;
		this->buffer = sound->getBuffer();
		if (this->buffer->isStreamed()) // streamed buffers cannot be shared
		{
			this->buffer = xal::mgr->_createBuffer(this->sound);
		}
	}

	Player::~Player()
	{
		hmutex::ScopeLock lock(&this->asyncPlayMutex);
		this->asyncPlayQueued = false;
		if (this->buffer->isStreamed()) // this buffer was created internally
		{
			xal::mgr->_destroyBuffer(this->buffer);
		}
	}

	float Player::getGain()
	{
		hmutex::ScopeLock lock(&xal::mgr->mutex);
		return this->_getGain();
	}

	float Player::_getGain()
	{
		return this->gain;
	}

	void Player::setGain(float value)
	{
		hmutex::ScopeLock lock(&xal::mgr->mutex);
		this->_setGain(value);
	}

	void Player::_setGain(float value)
	{
		this->gain = hclamp(value, 0.0f, 1.0f);
		this->_systemUpdateGain();
	}

	float Player::getPitch()
	{
		hmutex::ScopeLock lock(&xal::mgr->mutex);
		return this->_getPitch();
	}

	float Player::_getPitch()
	{
		return this->pitch;
	}

	void Player::setPitch(float value)
	{
		hmutex::ScopeLock lock(&xal::mgr->mutex);
		this->_setPitch(value);
	}

	void Player::_setPitch(float value)
	{
		this->pitch = hclamp(value, 0.01f, 100.0f);
		this->_systemUpdatePitch();
	}

	float Player::getTimePosition()
	{
		return ((float)this->getSamplePosition() / this->buffer->getSamplingRate());
	}

	unsigned int Player::getSamplePosition()
	{
		hmutex::ScopeLock lock(&xal::mgr->mutex);
		unsigned int position = this->_systemGetBufferPosition();
		if (this->sound->isStreamed() && this->_systemNeedsStreamedBufferPositionCorrection())
		{
			// corrects position by using number of processed bytes (circular)
			position = (position + (STREAM_BUFFER_COUNT - this->bufferIndex) * STREAM_BUFFER_SIZE) % STREAM_BUFFER;
			// adds streamed processed byte count
			position += this->processedByteCount;
		}
		position = hmin(position, (unsigned int)this->sound->getSize());
		return (unsigned int)(position / (this->buffer->getChannels() * this->buffer->getBitsPerSample() * 0.125f));
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

	int Player::getBufferSize()
	{
		if (this->buffer->isStreamed())
		{
			return (int)this->buffer->getStream().size();
		}
		else
		{
			return this->buffer->getSize();
		}
	}

	int Player::getSourceSize()
	{
		return this->buffer->getSource()->getRamSize();
	}

	bool Player::isPlaying()
	{
		hmutex::ScopeLock lock(&xal::mgr->mutex);
		return (!this->isFadingOut() && this->_isPlaying());
	}

	bool Player::_isPlaying()
	{
		if (this->_systemIsPlaying())
		{
			return true;
		}
		hmutex::ScopeLock lock(&this->asyncPlayMutex);
		return this->asyncPlayQueued;
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

	bool Player::_isAsyncPlayQueued()
	{
		if (!this->buffer->isLoaded())
		{
			return false;
		}
		hmutex::ScopeLock lock(&this->asyncPlayMutex);
		return this->asyncPlayQueued;
	}

	void Player::_update(float timeDelta)
	{
		if (this->_isPlaying())
		{
			this->buffer->keepLoaded();
			if (!this->sound->isStreamed())
			{
				this->_systemUpdateNormal();
			}
			else
			{
				this->processedByteCount += this->_systemUpdateStream();
			}
		}
		else if (this->paused)
		{
			this->buffer->keepLoaded();
		}
		if (this->isFading())
		{
			this->fadeTime += this->fadeSpeed * timeDelta;
			if (this->fadeTime >= 1.0f && this->fadeSpeed > 0.0f)
			{
				this->_systemUpdateGain();
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
				this->_systemUpdateGain();
			}
		}
	}

	void Player::play(float fadeTime, bool looping)
	{
		hmutex::ScopeLock lock(&xal::mgr->mutex);
		this->_play(fadeTime, looping);
	}

	void Player::playAsync(float fadeTime, bool looping)
	{
		hmutex::ScopeLock lock(&xal::mgr->mutex);
		this->_playAsync(fadeTime, looping);
	}

	void Player::stop(float fadeTime)
	{
		hmutex::ScopeLock lock(&xal::mgr->mutex);
		this->_stop(fadeTime);
	}

	void Player::pause(float fadeTime)
	{
		hmutex::ScopeLock lock(&xal::mgr->mutex);
		this->_pause(fadeTime);
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
			this->buffer->bind(this, this->paused);
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
		this->_systemUpdateGain();
		this->_systemUpdatePitch();
		if (!alreadyFading)
		{
			this->_systemPlay();
		}
		this->paused = false;
		hmutex::ScopeLock lock(&this->asyncPlayMutex);
		this->asyncPlayQueued = false;
	}

	void Player::_playAsync(float fadeTime, bool looping)
	{
		if (!xal::mgr->isEnabled())
		{
			return;
		}
		if (!this->paused)
		{
			this->looping = looping;
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
		this->buffer->prepareAsync();
		hmutex::ScopeLock lock(&this->asyncPlayMutex);
		this->asyncPlayQueued = true;
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
		this->processedByteCount = 0;
	}

	void Player::_pause(float fadeTime)
	{
		if (!this->_isPlaying() && !this->paused)
		{
			hlog::warn(xal::logTag, "Player cannot be paused, it's not playing: " + this->getName());
			return;
		}
		this->paused = true;
		this->_stopSound(fadeTime);
	}

	float Player::_calcGain()
	{
		float result = this->gain * this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain();
		if (this->isFading())
		{
			result *= this->fadeTime;
		}
		return hclamp(result, 0.0f, 1.0f);
	}

	void Player::_stopSound(float fadeTime)
	{
		hmutex::ScopeLock lock(&this->asyncPlayMutex);
		this->asyncPlayQueued = false;
		lock.release();
		if (fadeTime > 0.0f)
		{
			this->fadeSpeed = -1.0f / fadeTime;
			return;
		}
		this->offset = this->_systemGetOffset();
		this->processedByteCount += this->_systemStop();
		this->buffer->unbind(this, this->paused);
		this->fadeTime = 0.0f;
		this->fadeSpeed = 0.0f;
	}

}
