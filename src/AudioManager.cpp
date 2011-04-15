/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
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
#include "OpenAL_AudioManager.h"
#include "Category.h"
#include "SimpleSound.h"
#include "SoundBuffer.h"
#include "Source.h"
#include "StreamSound.h"
#include "xal.h"

#if TARGET_OS_IPHONE
#include "SourceApple.h"
#endif

namespace xal
{
	AudioManager* mgr;

	AudioManager::AudioManager(chstr deviceName, bool threaded, float updateTime) : gain(1.0f),
		updating(false), thread(NULL)
	{
		this->deviceName = deviceName;
		this->updateTime = updateTime;
	}

	AudioManager::~AudioManager()
	{
		if (this->thread != NULL)
		{
			while (this->updating);
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
		Sound* source;
		while (this->sources.size() > 0)
		{
			source = this->sources.pop_front();
			source->unlock();
			source->stop();
			delete source;
		}
	}
	
/******* PROPERTIES ****************************************************/

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
			Source* source = dynamic_cast<Source*> (*it); // FIXME what about SourceApple?
			if (source != NULL)
			{
				id = source->getSourceId();
				if (id != 0)
				{
					allocated += id;
				}
			}
		}
		harray<unsigned int> unallocated(this->sourceIds, XAL_MAX_SOURCES);
		unallocated -= allocated;
		if (unallocated.size() > 0)
		{
			return unallocated.front();
		}
		xal::log("unable to allocate audio source!");
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
			xal::log("created a dynamic sound: " + filename);
		}
		else if (!sound->load())
		{
			xal::log("failed to load sound " + filename);
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
		this->lockUpdate();
		harray<Sound*> sources(this->sources);
		foreach (Sound*, it, sources)
		{
			(*it)->unlock();
			(*it)->stop(fadeTime);
		}
		this->unlockUpdate();
	}
	
	void AudioManager::pauseAll(float fadeTime)
	{
		this->lockUpdate();
		harray<Sound*> sources(this->sources);
		foreach (Sound*, it, sources)
		{
			(*it)->pause(fadeTime);
		}
		this->unlockUpdate();
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
	
	void AudioManager::lockUpdate()
	{
		while (this->updating);
		this->updating = true;
	}
	
	void AudioManager::unlockUpdate()
	{
		this->updating = false;
	}
	
}
