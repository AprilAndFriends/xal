/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "Endianess.h"
#include "Sound.h"
#include "Source.h"
#include "AudioManager.h"

#ifndef __APPLE__
#include <AL/al.h>
#else
#include <OpenAL/al.h>
#endif

namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	Sound::Sound(chstr filename, chstr category, chstr prefix) : duration(0.0f),
		sources(harray<xal::Source*>())
	{
		this->filename = filename;
		this->name = prefix + filename.replace("\\", "/").rsplit("/").pop_back().rsplit(".", 1).pop_front();
		this->category = audiomgr->getCategoryByName(category);
	}

	Sound::~Sound()
	{
		audiomgr->logMessage("Audio Manager: Destroying sound: " + this->name);
	}
	
/******* METHODS *******************************************************/
	
	void Sound::bindSource(Source* source)
	{
		source->setSound(this);
		this->sources += source;
	}
	
	void Sound::unbindSource(Source* source)
	{
		source->setSound(NULL);
		this->sources -= source;
	}
	
	float Sound::getSampleOffset()
	{
		if (this->getBuffer() == 0 || this->sources.size() == 0)
		{
			return 0;
		}
		return this->sources[0]->getSampleOffset();
	}

	void Sound::setGain(float gain)
	{
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->setGain(gain);
		}
	}

	float Sound::getGain()
	{
		if (this->getBuffer() == 0 || this->sources.size() == 0)
		{
			return 1;
		}
		return this->sources[0]->getGain();
	}

	bool Sound::isPlaying()
	{
		for (Source** it = this->sources.iterate(); it; it = this->sources.next())
		{
			if ((*it)->isPlaying())
			{
				return true;
			}
		}
		return false;
	}

	bool Sound::isFading()
	{
		return (this->sources.size() > 0 && this->sources[0]->isFading());
	}

	bool Sound::isFadingIn()
	{
		return (this->sources.size() > 0 && this->sources[0]->isFadingIn());
	}

	bool Sound::isFadingOut()
	{
		return (this->sources.size() > 0 && this->sources[0]->isFadingOut());
	}

	bool Sound::isLooping()
	{
		return (this->sources.size() > 0 && this->sources[0]->isLooping());
	}

/******* PLAY CONTROLS *************************************************/

	Source* Sound::play(float fadeTime, bool looping)
	{
		if (this->getBuffer() == 0)
		{
			return NULL;
		}
		Source* source = NULL;
		if (this->sources.size() == 0 || this->sources[0]->isPlaying())
		{
			source = audiomgr->allocateSource();
			if (source == NULL)
			{
				return NULL;
			}
			this->bindSource(source);
		}
		else
		{
			source = this->sources[0];
		}
		source->play(fadeTime, looping);
		return source;
	}

	Source* Sound::replay(float fadeTime, bool looping)
	{
		if (this->getBuffer() == 0)
		{
			return NULL;
		}
		Source* source = NULL;
		if (this->sources.size() == 0)
		{
			source = audiomgr->allocateSource();
			if (source == NULL)
			{
				return NULL;
			}
			this->bindSource(source);
		}
		else
		{
			source = this->sources[0];
			source->stop();
		}
		source->replay(fadeTime, looping);
		return source;
	}

	void Sound::stop(float fadeTime)
	{
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->stop(fadeTime);
		}
	}

	void Sound::stopAll(float fadeTime)
	{
		if (this->getBuffer() != 0)
		{
			for (Source** it = this->sources.iterate(); it; it = this->sources.next())
			{
				(*it)->stop(fadeTime);
			}
		}
	}
	
	void Sound::pause(float fadeTime)
	{
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->pause(fadeTime);
		}
	}

}
