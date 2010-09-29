/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <fstream>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "Endianess.h"
#include "SoundBuffer.h"
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

	SoundBuffer::SoundBuffer(chstr fileName, chstr category, chstr prefix) : Sound(),
		duration(0.0f), sources(harray<xal::Source*>())
	{
		this->fileName = hstr(fileName);
		this->virtualFileName = this->fileName;
		this->name = prefix + hstr(fileName).replace("\\", "/").rsplit("/").pop_back().rsplit(".", 1).pop_front();
		this->category = xal::mgr->getCategoryByName(category);
	}

	SoundBuffer::~SoundBuffer()
	{
		foreach (Source*, it, this->sources)
		{
			(*it)->unlock();
			(*it)->unbind();
			xal::mgr->destroySource(*it);
		}
		xal::mgr->logMessage("Destroying sound " + this->name);
	}
	
/******* METHODS *******************************************************/

	bool SoundBuffer::load()
	{
		if (this->isLink())
		{
			this->virtualFileName = this->_findLinkedFile();
		}
		if (!xal::mgr->isEnabled())
		{
			return (this->isOgg());
		}
		if (this->isOgg())
		{
			return this->_loadOgg();
		}
		return false;
	}
	
	hstr SoundBuffer::_findLinkedFile()
	{
		char buffer[1024];
		std::ifstream file(this->fileName.c_str());
		if (!file.is_open())
		{
			return this->fileName;
		}
		file.getline(buffer, 1024);
		file.close();
		harray<hstr> newFolders = hstr(buffer).split("/");
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
		xal::mgr->logMessage(folders.join("/"));
		return folders.join("/");
	}

	void SoundBuffer::bindSource(Source* source)
	{
#ifdef _DEBUG
		xal::mgr->logMessage(hsprintf("Binding source %d to sound %s", source->getSourceId(), this->virtualFileName.c_str()));
#endif
		this->sources += source;
	}
	
	void SoundBuffer::unbindSource(Source* source)
	{
#ifdef _DEBUG
		xal::mgr->logMessage(hsprintf("Unbinding source from sound %s", this->virtualFileName.c_str()));
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
		if (this->sources.size() == 0)
		{
			return false;
		}
		return this->sources[0]->isLocked();
	}

	float SoundBuffer::getSampleOffset()
	{
		if (this->getBuffer() == 0 || this->sources.size() == 0)
		{
			return 0;
		}
		return this->sources[0]->getSampleOffset();
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
		if (this->getBuffer() == 0 || this->sources.size() == 0)
		{
			return 1;
		}
		return this->sources[0]->getGain();
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

/******* PLAY CONTROLS *************************************************/
	
	Sound* SoundBuffer::play(float fadeTime, bool looping)
	{
		if (this->getBuffer() == 0)
		{
			return NULL;
		}
		Source* source = NULL;
		if (this->sources.size() == 0 || this->sources[0]->isPlaying())
		{
			unsigned int sourceId = xal::mgr->allocateSourceId();
			if (sourceId == 0)
			{
				return NULL;
			}
#ifdef _DEBUG
			xal::mgr->logMessage(hsprintf("Allocated new source %d", sourceId));
#endif
			source = xal::mgr->createSource(this, sourceId);
			this->bindSource(source);
		}
		else
		{
#ifdef _DEBUG
			xal::mgr->logMessage("Using allocated source");
#endif
			source = this->sources[0];
		}
		source->play(fadeTime, looping);
		return source;
	}

	void SoundBuffer::stop(float fadeTime)
	{
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->stop(fadeTime);
		}
	}

	void SoundBuffer::stopAll(float fadeTime)
	{
		if (this->getBuffer() != 0)
		{
#ifdef _DEBUG
			xal::mgr->logMessage("Stop all");
#endif
			foreach (Source*, it, this->sources)
			{
				(*it)->stop(fadeTime);
			}
		}
	}
	
	void SoundBuffer::pause(float fadeTime)
	{
		if (this->getBuffer() != 0 && this->sources.size() > 0)
		{
			this->sources[0]->pause(fadeTime);
		}
	}

}
