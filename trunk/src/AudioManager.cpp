/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <hltypes/exception.h>
#include <hltypes/hdir.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>
#include <hltypes/util.h>

#ifndef __APPLE__
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <TargetConditionals.h>
#endif

#include "AudioManager.h"
#include "Category.h"
#include "SimpleSound.h"
#include "SoundBuffer.h"
#include "Source.h"
#include "StreamSound.h"

#if TARGET_OS_IPHONE
#include "SourceApple.h"
#endif

namespace xal
{
/******* GLOBAL ********************************************************/
	
	AudioManager* mgr;

	void init(chstr deviceName, bool threaded, float updateTime)
	{
		mgr = new AudioManager();
		mgr->init(deviceName, threaded, updateTime);
	}
	
	void destroy()
	{
		delete mgr;
	}
	
	void xal_writelog(chstr text)
	{
		printf("%s\n", text.c_str());
	}
	
	void (*gLogFunction)(chstr) = xal_writelog;
	ALCdevice* gDevice = NULL;
	ALCcontext* gContext = NULL;

	void setLogFunction(void (*function)(chstr))
	{
		gLogFunction = function;
	}
	
/******* CONSTRUCT / DESTRUCT ******************************************/

	AudioManager::AudioManager() : deviceName(""), updateTime(0.01f),
		sources(harray<Sound*>()), gain(1.0f), updating(false),
		categories(hmap<hstr, Category*>()),
		sounds(hmap<hstr, SoundBuffer*>()), thread(NULL)
	{
	}
	
	void AudioManager::init(chstr deviceName, bool threaded, float updateTime)
	{
		this->logMessage("initializing XAL");
		if (deviceName == "nosound")
		{
			this->deviceName = deviceName;
			this->logMessage("audio is disabled");
			return;
		}
		this->logMessage("initializing OpenAL");
		ALCdevice* currentDevice = alcOpenDevice(deviceName.c_str());
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			return;
		}
		this->deviceName = alcGetString(currentDevice, ALC_DEVICE_SPECIFIER);
		this->logMessage("audio device: " + this->deviceName);
		ALCcontext* currentContext = alcCreateContext(currentDevice, NULL);
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			return;
		}
		alcMakeContextCurrent(currentContext);
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			return;
		}
		alGenSources(XAL_MAX_SOURCES, this->sourceIds);
		gDevice = currentDevice;
		gContext = currentContext;
		this->deviceName = deviceName;
		if (threaded)
		{
			this->logMessage("starting thread management");
			this->updateTime = updateTime;
			this->updating = true;
			this->thread = new hthread(&AudioManager::update);
			this->thread->start();
			this->updating = false;
		}
	}

	AudioManager::~AudioManager()
	{
		if (this->thread != NULL)
		{
			while (this->updating);
			this->updating = false;
			this->thread->stop();
			delete this->thread;
		}
		foreach_m (SoundBuffer*, it, this->sounds)
		{
			delete it->second;
		}
		foreach_m (Category*, it, this->categories)
		{
			delete it->second;
		}
		this->logMessage("destroying OpenAL");
		if (gDevice)
		{
			Sound* source;
			while (this->sources.size() > 0)
			{
				source = this->sources.pop_front();
				source->unlock();
				source->stop();
				delete source;
			}
			alDeleteSources(XAL_MAX_SOURCES, this->sourceIds);
			alcMakeContextCurrent(NULL);
			alcDestroyContext(gContext);
			alcCloseDevice(gDevice);
		}
	}
	
/******* PROPERTIES ****************************************************/

	void AudioManager::logMessage(chstr message, chstr prefix)
	{
		gLogFunction(prefix + message);
	}
	
	bool AudioManager::isEnabled()
	{
		return (gDevice != NULL);
	}

	void AudioManager::update()
	{
		while (true)
		{
			while (xal::mgr->updating);
			xal::mgr->updating = true;
			xal::mgr->update(xal::mgr->updateTime);
			xal::mgr->updating = false;
			hthread::sleep(xal::mgr->updateTime * 1000);
		}
	}
	
	void AudioManager::update(float k)
	{
		if (this->isEnabled())
		{
			// variable copied because (*it)->update can access
			// xal::mgr and erase a source from this->sources.
			// we don't want to break the iterator validity!
			
			harray<Sound*> sources(this->sources);
			foreach (Sound*, it, sources)
			{
				(*it)->update(k);
			}
		}
	}

	unsigned int AudioManager::allocateSourceId()
	{
		harray<unsigned int> allocated;
		unsigned int id = 0;
		foreach (Sound*, it, this->sources)
		{
			Source* source = dynamic_cast<Source*> (*it);
			if(!source)
				continue;
			id = source->getSourceId();
			if (id != 0)
			{
				allocated += id;
			}
		}
		harray<unsigned int> unallocated(this->sourceIds, XAL_MAX_SOURCES);
		unallocated -= allocated;
		if (unallocated.size() > 0)
		{
			return unallocated.front();
		}
		this->logMessage("unable to allocate audio source!");
		return 0;
	}

	Sound* AudioManager::createSource(SoundBuffer* sound, unsigned int sourceId)
	{
		Source* source = new Source(sound, sourceId);
		this->sources += source;
		return source;
	}
	
	Sound* AudioManager::createSourceApple(SoundBuffer* sound, unsigned int sourceId)
	{
#if TARGET_OS_IPHONE
		SourceApple* source = new SourceApple(sound, sourceId);
		this->sources += source;
		return source;
#else
		return NULL;
#endif
	}
	
	void AudioManager::destroySource(Sound* source)
	{
		this->sources -= source;
		delete source;
	}

	Sound* AudioManager::getSound(chstr name)
	{
		if (!this->sounds.has_key(name))
		{
			throw key_error(name, "Sounds");
		}
		return this->sounds[name];
	}
	
	Sound* AudioManager::createSound(chstr filename, chstr categoryName, chstr prefix)
	{
		Category* category = this->getCategoryByName(categoryName);
		SoundBuffer* sound;
		if (category->isStreamed())
		{
			sound = new StreamSound(filename, categoryName, prefix);
		}
		else
		{
			sound = new SimpleSound(filename, categoryName, prefix);
		}
		if (category->isDynamicLoad())
		{
			this->logMessage("creating dynamic sound " + filename);
		}
		else if (!sound->load())
		{
			return NULL;
		}
		this->sounds[sound->getName()] = sound;
		return sound;
	}

	harray<hstr> AudioManager::createSoundsFromPath(chstr path, chstr prefix)
	{
		harray<hstr> result;
		hstr category;
		harray<hstr> dirs = hdir::directories(path, true);
		foreach (hstr, it, dirs)
		{
			category = (*it).rsplit("/").pop_back();
			result += this->createSoundsFromPath((*it).c_str(), category, prefix);
		}
		return result;
	}

	harray<hstr> AudioManager::createSoundsFromPath(chstr path, chstr category, chstr prefix)
	{
		this->createCategory(category);
		harray<hstr> result;
		harray<hstr> files = hdir::files(path, true);
		SoundBuffer* sound;
		foreach (hstr, it, files)
		{
			sound = (SoundBuffer*)this->createSound((*it).c_str(), category, prefix);
			if (sound != NULL)
			{
				result += sound->getName();
			}
		}
		return result;
	}

	void AudioManager::destroySound(SoundBuffer* sound)
	{
		foreach_m (SoundBuffer*, it, this->sounds)
		{
			if (it->second == sound)
			{
				this->sounds.erase(it);
				delete it->second;
				break;
			}
		}
	}
	
	void AudioManager::destroySoundsWithPrefix(chstr prefix)
	{
		harray<hstr> deleteList;
		foreach_m (SoundBuffer*, it, this->sounds)
		{
			if (it->first.starts_with(prefix))
			{
				delete it->second;
				deleteList.push_back(it->first);
			}
		}
		foreach (hstr, it, deleteList)
		{
			this->sounds.erase(*it);
		}
	}

	void AudioManager::createCategory(chstr name, bool streamed, bool dynamicLoad)
	{
		if (!this->categories.has_key(name))
		{
			this->categories[name] = new Category(name, streamed, dynamicLoad);
		}
	}

	void AudioManager::setGlobalGain(float value)
	{
		this->gain = value;
		foreach (Sound*, it, this->sources)
		{
			(*it)->setGain((*it)->getGain());
		}
	}

	Category* AudioManager::getCategoryByName(chstr name)
	{
		if (!this->categories.has_key(name))
		{
			throw ("Audio Manager: Category '" + name + "' does not exist!").c_str();
		}
		return this->categories[name];
	}

	float AudioManager::getCategoryGain(chstr name)
	{
		if (!this->categories.has_key(name))
		{
			throw ("Audio Manager: Category '" + name + "' does not exist!").c_str();
		}
		return this->categories[name]->getGain();
	}

	void AudioManager::setCategoryGain(chstr name, float gain)
	{
		this->getCategoryByName(name)->setGain(gain);
		foreach (Sound*, it, this->sources)
		{
			(*it)->setGain((*it)->getGain());
		}
	}

	void AudioManager::stopAll(float fadeTime)
	{
		while (this->updating);
		this->updating = true;
		harray<Sound*> sources(this->sources);
		foreach (Sound*, it, sources)
		{
			(*it)->unlock();
			(*it)->stop(fadeTime);
		}
		this->updating = false;
	}
	
	void AudioManager::stopCategory(chstr categoryName, float fadeTime)
	{
		Category* category = this->categories[categoryName];
		harray<Sound*> sources(this->sources);
		foreach (Sound*, it, sources)
		{
			if ((*it)->getCategory() == category)
			{
				(*it)->unlock();
				(*it)->stop(fadeTime);
			}
		}
	}
	
}
