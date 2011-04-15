/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

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
#include "Player.h"
#include "Sound2.h"
#include "xal.h"



#include "SimpleSound.h"
#include "SoundBuffer.h"
#include "Source.h"
#include "StreamSound.h"

#if TARGET_OS_IPHONE
#include "SourceApple.h"
#endif

namespace xal
{
	AudioManager* mgr;

	AudioManager::AudioManager(chstr deviceName, bool threaded, float updateTime) : enabled(false),
		gain(1.0f), updating(false), thread(NULL)
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
		foreach (Player*, it, this->players)
		{
			delete (*it);
		}
		foreach_m (Sound2*, it, this->sounds)
		{
			delete it->second;
		}
		foreach_m (Category*, it, this->categories)
		{
			delete it->second;
		}
		/////////////////////////////////////////////////



		foreach_m (SoundBuffer*, it, this->oldSounds)
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
	
	void AudioManager::_setupThread()
	{
		xal::log("starting thread management");
		this->updateTime = updateTime;
		this->updating = true;
		this->thread = new hthread(&AudioManager::update);
		this->thread->start();
		this->updating = false;
	}

	void AudioManager::setGlobalGain(float value)
	{
		this->gain = value;
		foreach (Sound*, it, this->sources)
		{
			(*it)->setGain((*it)->getGain());
		}
	}

	Category* AudioManager::createCategory(chstr name, bool streamed, bool dynamicLoad)
	{
		if (!this->categories.has_key(name))
		{
			this->categories[name] = new Category(name, streamed, dynamicLoad);
		}
		return this->categories[name];
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


			//////////////////////////////////////////////////



			harray<Sound*> sources(this->sources);
			foreach (Sound*, it, sources)
			{
				(*it)->update(k);
			}
		}
	}

	Sound2* AudioManager::createSound2(chstr filename, chstr categoryName, chstr prefix)
	{
		Category* category = this->getCategoryByName(categoryName);
		Sound2* sound = new Sound2(filename, category, prefix);
		// TODO
		/*
		///##
		if (category->isDynamicLoad())
		{
			xal::log("created a dynamic sound: " + filename);
		}
		else if (!sound->load())
		{
			xal::log("failed to load sound " + filename);
			return NULL;
		}
		*/
		this->sounds[sound->getName()] = sound;
		return sound;
	}

	harray<hstr> AudioManager::createSound2sFromPath(chstr path, chstr prefix)
	{
		harray<hstr> result;
		hstr category;
		harray<hstr> dirs = hdir::directories(path, true);
		foreach (hstr, it, dirs)
		{
			category = (*it).rsplit("/").pop_back();
			result += this->createSound2sFromPath((*it).c_str(), category, prefix);
		}
		return result;
	}

	harray<hstr> AudioManager::createSound2sFromPath(chstr path, chstr category, chstr prefix)
	{
		this->createCategory(category);
		harray<hstr> result;
		harray<hstr> files = hdir::files(path, true);
		SoundBuffer* sound;
		foreach (hstr, it, files)
		{
			sound = (SoundBuffer*)this->createSound2((*it), category, prefix);
			if (sound != NULL)
			{
				result += sound->getName();
			}
		}
		return result;
	}

	void AudioManager::destroySound2(Sound2* sound)
	{
		foreach_m (Sound2*, it, this->sounds)
		{
			if (it->second == sound)
			{
				delete it->second;
				this->sounds.erase(it);
				break;
			}
		}
	}
	
	void AudioManager::destroySound2sWithPrefix(chstr prefix)
	{
		harray<hstr> keys = this->sounds.keys();
		foreach (hstr, it, keys)
		{
			if ((*it).starts_with(prefix))
			{
				delete this->sounds[*it];
				this->sounds.remove_key(*it);
			}
		}
	}

	Player* AudioManager::createPlayer(chstr name)
	{
		Sound2* sound = this->sounds[name];
		Player* player = this->_createPlayer(this->sounds[name], this->sounds[name]->getBuffer());
		this->players += player;
		return player;
	}

	Player* AudioManager::_createPlayer(Sound2* sound, Buffer* buffer)
	{
		return new Player(sound, buffer);
	}
	
	void AudioManager::destroyPlayer(Player* player)
	{
		this->players -= player;
		if (this->managedPlayers.contains(player))
		{
			this->managedPlayers -= player;
		}
		delete player;
	}





//////////////////////////////////////////////////////////////////////////////////////////////////

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
					if (!this->oldSounds.has_key(name))
					{
						throw key_error(name, "Sounds");
					}
					return this->oldSounds[name];
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
					this->oldSounds[sound->getName()] = sound;
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
					foreach_m (SoundBuffer*, it, this->oldSounds)
					{
						if (it->second == sound)
						{
							this->oldSounds.erase(it);
							delete it->second;
							break;
						}
					}
				}
	
				void AudioManager::destroySoundsWithPrefix(chstr prefix)
				{
					harray<hstr> deleteList;
					foreach_m (SoundBuffer*, it, this->oldSounds)
					{
						if (it->first.starts_with(prefix))
						{
							delete it->second;
							deleteList.push_back(it->first);
						}
					}
					foreach (hstr, it, deleteList)
					{
						this->oldSounds.erase(*it);
					}
				}

}
