/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/hstring.h>
#include "Category.h"
#include "Source.h"
#include "Sound.h"
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

	Source::Source(unsigned int id) : gain(1.0f), looping(false), paused(false), fadeTime(0.0f),
		fadeSpeed(0.0f), sound(NULL)
	{
		this->id = id;
	}

	Source::~Source()
	{
		if (this->sound != NULL)
		{
			this->stop();
		}
	}

/******* METHODS *******************************************************/
#include <stdio.h> //#
	void Source::update(float k)
	{
		if (this->id == 0 || this->sound == NULL)
		{
			return;
		}
		this->sound->update(this->id);
		if (this->isPlaying())
		{
			if (this->isFading())
			{
				this->fadeTime += this->fadeSpeed * k;
				if (this->fadeTime >= 1.0f && this->fadeSpeed > 0.0f)
				{
					alSourcef(this->id, AL_GAIN, this->gain * this->sound->getCategory()->getGain());
					this->fadeTime = 0.0f;
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
					alSourcef(this->id, AL_GAIN, this->fadeTime * this->gain * this->sound->getCategory()->getGain());
				}
			}
		}
		if (!this->isPlaying() && !this->isPaused() && this->sound != NULL)
		{
			this->sound->unbindSource(this);
		}
	}

	void Source::play(float fadeTime, bool looping)
	{
		if (this->id == 0 || this->sound == NULL)
		{
			return;
		}
		if (this->sound->getCategory()->isStreamed())
		{
			if (!this->isPaused())
			{
				((StreamSound*)this->sound)->queueBuffers(this->id);
			}
			alSourcei(this->id, AL_LOOPING, false);
		}
		else
		{
			if (!this->isPaused())
			{
				alSourcei(this->id, AL_BUFFER, this->getBuffer());
			}
			alSourcei(this->id, AL_LOOPING, this->looping);
		}
		this->paused = false;
		if (fadeTime > 0)
		{
			this->fadeSpeed = 1.0f / fadeTime;
		}
		else
		{
			this->fadeTime = 1.0f;
			this->fadeSpeed = 0.0f;
		}
		alSourcef(this->id, AL_GAIN, this->fadeTime * this->gain * this->sound->getCategory()->getGain());
		alSourcePlay(this->id);
	}

	void Source::replay(float fadeTime, bool looping)
	{
		if (this->id == 0 || this->sound == NULL)
		{
			return;
		}
		if (this->isPlaying())
		{
			this->stop();
		}
		this->play(fadeTime, looping);
	}

	void Source::stop(float fadeTime)
	{
		if (this->id == 0 || this->sound == NULL)
		{
			return;
		}
		this->paused = false;
		if (fadeTime > 0)
		{
			this->fadeSpeed = -1.0f / fadeTime;
		}
		else
		{
			this->fadeTime = 0.0f;
			this->fadeSpeed = 0.0f;
			alSourceStop(this->id);
			this->sound->unbindSource(this);
		}
	}

	void Source::pause(float fadeTime)
	{
		if (this->id == 0 || this->sound == NULL)
		{
			return;
		}
		if (fadeTime > 0)
		{
			this->fadeSpeed = -1.0f / fadeTime;
		}
		else
		{
			this->fadeTime = 0.0f;
			this->fadeSpeed = 0.0f;
			alSourcePause(this->id);
		}
		this->paused = true;
	}

/******* PROPERTIES ****************************************************/

	float Source::getSampleOffset()
	{
		if (this->id == 0 || this->sound == NULL)
		{
			return 0.0f;
		}
		float value;
		alGetSourcef(this->id, AL_SEC_OFFSET, &value);
		return value;
	}

	void Source::setGain(float gain)
	{
		this->gain = gain;
		if (this->id != 0)
		{
			alSourcef(this->id, AL_GAIN, this->gain * this->sound->getCategory()->getGain());
		}
	}

	bool Source::hasSound()
	{
		return (this->sound != NULL);
	}
	
	unsigned int Source::getBuffer()
	{
		return this->sound->getBuffer();
	}
	
	bool Source::isPlaying()
	{
		if (this->id == 0 || this->sound == NULL)
		{
			return false;
		}
		if (this->sound->getCategory()->isStreamed())
		{
			return (!this->isPaused());
		}
		int state;
		alGetSourcei(this->id, AL_SOURCE_STATE, &state);
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
