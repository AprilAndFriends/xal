/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef __APPLE__
#include <AL/al.h>
#else
#include <OpenAL/al.h>
#include <TargetConditionals.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "Endianess.h"
#include "SoundBuffer.h"
#include "Source.h"
#include "AudioManager.h"

namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	SoundBuffer::SoundBuffer(chstr fileName, chstr category, chstr prefix) : Sound(),
		duration(0.0f), sources(harray<xal::Sound*>()), loaded(false)
	{
		this->fileName = hstr(fileName);
		this->virtualFileName = this->fileName;
		// extracting filename without extension and prepending the prefix
		this->name = prefix + hstr(fileName).replace("\\", "/").rsplit("/").pop_back().rsplit(".", 1).pop_front();
		this->category = xal::mgr->getCategoryByName(category);
	}

	SoundBuffer::~SoundBuffer()
	{
		xal::mgr->logMessage("destroying sound " + this->name);
	}
	
	void SoundBuffer::destroySources()
	{
		Sound* sound;
		Source* source;
		while (this->sources.size() > 0)
		{
			sound = this->sources.front();
			sound->unlock();
			
			source = dynamic_cast<Source*> (sound);
			if(!source)
				continue;
			source->stopSoft();
			source->unbind();
		}
	}
	
/******* METHODS *******************************************************/

	bool SoundBuffer::load()
	{
		bool result = false;
		if (this->isLink())
		{
			this->virtualFileName = this->_findLinkedFile();
		}
		if (!xal::mgr->isEnabled())
		{
			result = this->isOgg();
#if TARGET_OS_IPHONE
			result |= this->isM4a();
#endif
		}
		else if (this->isOgg())
		{
			result = this->_loadOgg();
		}
#if TARGET_OS_IPHONE
		else if (this->isM4a())
		{
			// no need to load m4a aac.
			//result = this->_loadM4a();
			result = true;
		}
#endif
		if (result)
		{
			this->loaded = result;
		}
		return result;
	}
	
	hstr SoundBuffer::_findLinkedFile()
	{
		if (!hfile::exists(this->fileName))
		{
			return this->fileName;
		}
		harray<hstr> newFolders = hfile::hread(this->fileName).split("/");
		harray<hstr> folders = this->fileName.split("/");
		folders.pop_back();
		foreach (hstr, it, newFolders)
		{
			if ((*it) != "..")
			{
				folders += (*it);
			}
			else
			{
				folders.pop_back();
			}
		}
		return folders.join("/");
	}

	void SoundBuffer::bindSource(Sound* source)
	{
#ifdef _DEBUG
		xal::mgr->logMessage(hsprintf("binding source %d to sound %s", source->getSourceId(), this->virtualFileName.c_str()));
#endif
		this->sources += source;
	}
	
	void SoundBuffer::unbindSource(Sound* source)
	{
#ifdef _DEBUG
		xal::mgr->logMessage(hsprintf("unbinding source from sound %s", this->virtualFileName.c_str()));
#endif
		this->sources -= source;
	}
	
	void SoundBuffer::lock()
	{
		if (this->sources.size() > 0)
		{
			this->sources[0]->lock();
		}
	}

	void SoundBuffer::unlock()
	{
		if (this->sources.size() > 0)
		{
			this->sources[0]->unlock();
		}
	}

	bool SoundBuffer::isLocked()
	{
		return (this->sources.size() > 0 && this->sources[0]->isLocked());
	}

	float SoundBuffer::getSampleOffset()
	{
		return (this->getBuffer() != 0 && this->sources.size() > 0 ? this->sources[0]->getSampleOffset() : 0);
	}

	void SoundBuffer::setGain(float gain)
	{
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->setGain(gain);
		}
	}

	float SoundBuffer::getGain()
	{
		return (this->getBuffer() != 0 && this->sources.size() > 0 ? this->sources[0]->getGain() : 1.0f);
	}

	bool SoundBuffer::isPlaying()
	{
		return (this->sources.size() > 0 && this->sources[0]->isPlaying());
	}

	bool SoundBuffer::isPaused()
	{
		return (this->sources.size() > 0 && this->sources[0]->isPaused());
	}

	bool SoundBuffer::isFading()
	{
		return (this->sources.size() > 0 && this->sources[0]->isFading());
	}

	bool SoundBuffer::isFadingIn()
	{
		return (this->sources.size() > 0 && this->sources[0]->isFadingIn());
	}

	bool SoundBuffer::isFadingOut()
	{
		return (this->sources.size() > 0 && this->sources[0]->isFadingOut());
	}

	bool SoundBuffer::isLooping()
	{
		return (this->sources.size() > 0 && this->sources[0]->isLooping());
	}

	bool SoundBuffer::isLink()
	{
		return this->fileName.ends_with(".xln");
	}

	bool SoundBuffer::isOgg()
	{
		return this->virtualFileName.ends_with(".ogg");
	}
	
	bool SoundBuffer::isM4a()
	{
		return this->virtualFileName.ends_with(".m4a");
	}

/******* PLAY CONTROLS *************************************************/
	
	Sound* SoundBuffer::play(float fadeTime, bool looping)
	{
		xal::mgr->lockUpdate();
		if (!this->loaded)
		{
			this->load();
		}
		if (!this->isM4a() && this->getBuffer() == 0)
		{
			xal::mgr->unlockUpdate();
			return NULL;
		}
		Sound* source = NULL;
		if (this->sources.size() == 0 || this->sources[0]->isPlaying() && !this->sources[0]->isFading())
		{
			unsigned int sourceId = xal::mgr->allocateSourceId();
			if (sourceId == 0)
			{
				xal::mgr->unlockUpdate();
				return NULL;
			}
#ifdef _DEBUG
			xal::mgr->logMessage(hsprintf("allocated new source %d", sourceId));
#endif
			if (this->isOgg())
			{
				source = xal::mgr->createSource(this, sourceId);
			}
#if TARGET_OS_IPHONE
			else if(this->isM4a())
			{
				source = xal::mgr->createSourceApple(this, sourceId);
			}
#endif
			this->bindSource(source);
		}
		else
		{
#ifdef _DEBUG
			xal::mgr->logMessage("using allocated source");
#endif
			source = this->sources[0];
		}
		source->play(fadeTime, looping);
		xal::mgr->unlockUpdate();
		return source;
	}

	void SoundBuffer::stop(float fadeTime)
	{
		xal::mgr->lockUpdate();
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->stop(fadeTime);
		}
		xal::mgr->unlockUpdate();
	}

	void SoundBuffer::stopSoft(float fadeTime, bool pause)
	{		
		xal::mgr->lockUpdate();
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->stopSoft(fadeTime, pause);
		}
		xal::mgr->unlockUpdate();
	}

	void SoundBuffer::stopAll(float fadeTime)
	{
		if (this->getBuffer() != 0)
		{
#ifdef _DEBUG
			xal::mgr->logMessage("stop all");
#endif
			foreach (Sound*, it, this->sources)
			{
				(*it)->stop(fadeTime);
			}
		}
	}
	
	void SoundBuffer::pause(float fadeTime)
	{
		xal::mgr->lockUpdate();
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->pause(fadeTime);
		}
		xal::mgr->unlockUpdate();
	}

}
