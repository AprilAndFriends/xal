/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/hstring.h>
#include "Category.h"
#include "Source.h"
#include "SoundBuffer.h"
#include "StreamSound.h"
#include "AudioManager.h"

#ifndef __APPLE__
	#include <AL/al.h>
#else
	#include <OpenAL/al.h>
#endif

namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	Source::Source(SoundBuffer* sound, unsigned int sourceId) : Sound(),
		gain(1.0f), looping(false), paused(false), fadeTime(0.0f),
		fadeSpeed(0.0f), bound(true), sampleOffset(0.0f)
	{
		this->sound = sound;
		this->sourceId = sourceId;
	}

	Source::~Source()
	{
		this->stop();
	}

/******* METHODS *******************************************************/
	
	void Source::update(float k)
	{
		if (this->sourceId == 0)
		{
			return;
		}
		this->sound->setSourceId(this->sourceId);
		this->sound->update(k);
		if (this->isPlaying())
		{
			if (this->isFading())
			{
				this->fadeTime += this->fadeSpeed * k;
				if (this->fadeTime >= 1.0f && this->fadeSpeed > 0.0f)
				{
					alSourcef(this->sourceId, AL_GAIN, this->gain *
						this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain());
					this->fadeTime = 1.0f;
					this->fadeSpeed = 0.0f;
				}
				else if (this->fadeTime <= 0.0f && this->fadeSpeed < 0.0f)
				{
					this->paused ? this->pause() : this->stop();
					this->fadeTime = 0.0f;
					this->fadeSpeed = 0.0f;
				}
				else
				{
					alSourcef(this->sourceId, AL_GAIN, this->fadeTime * this->gain *
						this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain());
				}
			}
		}
		if (!this->isPlaying() && !this->isPaused())
		{
			this->unbind();
		}
	}

	Sound* Source::play(float fadeTime, bool looping)
	{
		if (this->sourceId == 0)
		{
			this->sourceId = xal::mgr->allocateSourceId();
			if (this->sourceId == 0)
			{
				return NULL;
			}
#ifdef _DEBUG
			xal::mgr->logMessage(hsprintf("Allocated source ID %d", this->sourceId));
#endif
		}
#ifdef _DEBUG
		xal::mgr->logMessage("Play sound " + this->getSound()->getVirtualFileName());
#endif
		if (!this->paused)
		{
			this->looping = looping;
		}
		bool alreadyFading = this->isFading();
		if (!alreadyFading)
		{
			if (this->sound->getCategory()->isStreamed())
			{
				alSourcei(this->sourceId, AL_BUFFER, 0);
				this->sound->setSourceId(this->sourceId);
				((StreamSound*)this->sound)->queueBuffers();
				alSourcei(this->sourceId, AL_LOOPING, false);
			}
			else
			{
				alSourcei(this->sourceId, AL_BUFFER, this->getBuffer());
				alSourcei(this->sourceId, AL_LOOPING, this->looping);
			}
			if (this->isPaused())
			{
				alSourcef(this->sourceId, AL_SEC_OFFSET, this->sampleOffset);
			}
		}
		if (fadeTime > 0)
		{
			this->fadeSpeed = 1.0f / fadeTime;
#ifdef _DEBUG
			xal::mgr->logMessage("Fading in sound " + this->getSound()->getVirtualFileName());
#endif
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
		return this;
	}

	void Source::stop(float fadeTime)
	{
		this->_stop(fadeTime);
	}

	void Source::pause(float fadeTime)
	{
		this->_stop(fadeTime, true);
	}

	void Source::_stop(float fadeTime, bool pause)
	{
		if (this->sourceId == 0)
		{
			return;
		}
		if (fadeTime > 0)
		{
#ifdef _DEBUG
			xal::mgr->logMessage("Fading out sound " + this->getSound()->getVirtualFileName());
#endif
			this->fadeSpeed = -1.0f / fadeTime;
		}
		else
		{
#ifdef _DEBUG
			if (pause)
			{
				xal::mgr->logMessage("Pause sound " + this->getSound()->getVirtualFileName());
			}
			else
			{
				xal::mgr->logMessage("Stop sound " + this->getSound()->getVirtualFileName());
			}
#endif
			this->fadeTime = 0.0f;
			this->fadeSpeed = 0.0f;
			alGetSourcef(this->sourceId, AL_SEC_OFFSET, &this->sampleOffset);
			alSourceStop(this->sourceId);
			if (this->sound->getCategory()->isStreamed())
			{
				this->sound->setSourceId(this->sourceId);
				if (pause)
				{
					((StreamSound*)this->sound)->unqueueBuffers();
				}
				else
				{
					((StreamSound*)this->sound)->rewindStream();
				}
			}
			this->unbind(pause);
		}
		this->paused = pause;
	}

	void Source::unbind(bool pause)
	{
		if (!this->isLocked())
		{
			this->sourceId = 0;
			if (!pause)
			{
				this->bound = false;
			}
		}
	}
	
/******* PROPERTIES ****************************************************/

	void Source::setGain(float gain)
	{
		this->gain = gain;
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->gain *
				this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain());
		}
	}

	unsigned int Source::getBuffer()
	{
		return this->sound->getBuffer();
	}
	
	bool Source::isPlaying()
	{
		if (this->sourceId == 0)
		{
			return false;
		}
		if (this->sound->getCategory()->isStreamed())
		{
			return (!this->isPaused());
		}
		int state;
		alGetSourcei(this->sourceId, AL_SOURCE_STATE, &state);
		return (state == AL_PLAYING);
	}

	bool Source::isPaused()
	{
		return (this->paused && !this->isFading());
	}
	
	bool Source::isFading()
	{
		return (this->fadeSpeed != 0);
	}

	bool Source::isFadingIn()
	{
		return (this->fadeSpeed < 0);
	}

	bool Source::isFadingOut()
	{
		return (this->fadeSpeed < 0);
	}

}
