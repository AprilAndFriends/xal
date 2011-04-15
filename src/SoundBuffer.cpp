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
#include "xal.h"

#if TARGET_OS_IPHONE
#include "SourceApple.h"
#endif

namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	SoundBuffer::SoundBuffer(chstr filename, chstr category, chstr prefix) : Sound(),
		duration(0.0f), loaded(false)
	{
		this->filename = hstr(filename);
		this->virtualFilename = this->filename;
		// extracting filename without extension and prepending the prefix
		this->name = prefix + hstr(filename).replace("\\", "/").rsplit("/").pop_back().rsplit(".", 1).pop_front();
		this->category = xal::mgr->getCategoryByName(category);
	}

	SoundBuffer::~SoundBuffer()
	{
		xal::log("destroying sound " + this->name);
	}

	hstr SoundBuffer::_findLinkedFile()
	{
		if (!hfile::exists(this->filename))
		{
			return this->filename;
		}
		harray<hstr> newFolders = hfile::hread(this->filename).split("/");
		harray<hstr> folders = this->filename.split("/");
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

	////////////////////////////////////////////////////////////////////////////////

	
	void SoundBuffer::destroySources()
	{
		Sound* sound;
		Source* source;
		while (this->sources.size() > 0)
		{
			sound = this->sources.front();
			sound->unlock();
			source = dynamic_cast<Source*>(sound);
			if (source != NULL)
			{
				source->stopSoft();
				source->unbind();
				continue;
			}
#if TARGET_OS_IPHONE
			SourceApple* sourceApple = dynamic_cast<SourceApple*>(sound);
			if (sourceApple != NULL)
			{
				sourceApple->stopSoft();
				sourceApple->unbind();
				continue;
			}
#endif
		}
	}
	
/******* METHODS *******************************************************/

	bool SoundBuffer::load()
	{
		bool result = false;
		if (this->isLink())
		{
			this->virtualFilename = this->_findLinkedFile();
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
	
	void SoundBuffer::bindSource(Sound* source)
	{
		this->sources += source;
	}
	
	void SoundBuffer::unbindSource(Sound* source)
	{
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
		return this->filename.ends_with(".xln");
	}

	bool SoundBuffer::isOgg()
	{
		return this->virtualFilename.ends_with(".ogg");
	}
	
	bool SoundBuffer::isM4a()
	{
		return this->virtualFilename.ends_with(".m4a");
	}

/******* PLAY CONTROLS *************************************************/
	
	Sound* SoundBuffer::play(float fadeTime, bool looping)
	{
		xal::mgr->lockUpdate();
		if (!this->loaded)
		{
			this->load();
		}
		if (!this->isM4a() && !this->isValidBuffer()) // TODO check if this should still perhaps say "if(!this->isM4a() && this->getBuffer() == 0)"
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
			source = this->sources[0];
		}
		source->play(fadeTime, looping);
		xal::mgr->unlockUpdate();
		return source;
	}

	void SoundBuffer::stop(float fadeTime)
	{
		xal::mgr->lockUpdate();
		if (this->sources.size() > 0)
		{
			if (this->isValidBuffer()) {
				this->sources[0]->stop(fadeTime);
			}
			
		}
		xal::mgr->unlockUpdate();
	}

	void SoundBuffer::stopSoft(float fadeTime, bool pause)
	{		
		xal::mgr->lockUpdate();
		if (this->isValidBuffer() && this->sources.size() > 0)
		{
			this->sources[0]->stopSoft(fadeTime, pause);
		}
		xal::mgr->unlockUpdate();
	}

	void SoundBuffer::stopAll(float fadeTime)
	{
		if (this->isValidBuffer())
		{
			foreach (Sound*, it, this->sources)
			{
				(*it)->stop(fadeTime);
			}
		}
	}
	
	void SoundBuffer::pause(float fadeTime)
	{
		xal::mgr->lockUpdate();
		if (this->isValidBuffer() && this->sources.size() > 0)
		{
			this->sources[0]->pause(fadeTime);
		}
		xal::mgr->unlockUpdate();
	}
	
	bool SoundBuffer::isValidBuffer() const 
	{
		// helper method for checking if buffer is valid
		// replaces previous check "if (this->getBuffer() != 0)"
		
		// TODO check if this could be replaceable with SoundBuffer::isLoaded()!
		
		if(this->getBuffer() != 0)
		{
			return true;
		}
#if TARGET_OS_IPHONE
		if(this->sources.size())
		{
			SourceApple* sourceApple = dynamic_cast<SourceApple*> (this->sources[0]);
			if (sourceApple && sourceApple->getBuffer() != 0)
			{
				return true;
			}
		}
#endif
		return false;
	}
	

}
