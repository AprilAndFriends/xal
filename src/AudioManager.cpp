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
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#ifndef __APPLE__
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#include "AudioManager.h"
#include "Category.h"
#include "Sound.h"
#include "SimpleSound.h"
#include "StreamSound.h"
#include "Source.h"
#include "Util.h"

namespace xal
{
/******* GLOBAL ********************************************************/
	
	AudioManager* mgr;

	void init(chstr deviceName)
	{
		mgr = new AudioManager(deviceName);
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
	ALCdevice* gDevice;
	ALCcontext* gContext;

	void setLogFunction(void (*function)(chstr))
	{
		gLogFunction = function;
	}
	
/******* CONSTRUCT / DESTRUCT ******************************************/

	AudioManager::AudioManager(chstr deviceName)
	{
		this->logMessage("Initializing XAL");
		if (deviceName == "nosound")
		{
			this->deviceName = "nosound";
			this->logMessage("- Audio is disabled");
			return;
		}
		// init OpenAL
		this->logMessage("Initializing OpenAL");
		ALCdevice* currentDevice = alcOpenDevice(deviceName.c_str());
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			return;
		}
		this->deviceName = alcGetString(currentDevice, ALC_DEVICE_SPECIFIER);
		this->logMessage("Audio device: " + this->deviceName);
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
		ALuint sourceIds[XAL_MAX_SOURCES];
		alGenSources(XAL_MAX_SOURCES, sourceIds);
		for (int i = 0; i < XAL_MAX_SOURCES; i++)
		{
			this->sources[i] = new xal::Source(sourceIds[i]);
		}
		gDevice = currentDevice;
		gContext = currentContext;
		this->deviceName = deviceName;
	}

	AudioManager::~AudioManager()
	{
		for (std::map<hstr, Sound*>::iterator it = this->sounds.begin(); it != this->sounds.end(); it++)
		{
			delete it->second;
		}
		for (std::map<hstr, Category*>::iterator it = this->categories.begin(); it != this->categories.end(); it++)
		{
			delete it->second;
		}
		this->logMessage("Destroying OpenAL");
		if (gDevice)
		{
			ALuint id;
			for (int i = 0; i < XAL_MAX_SOURCES; i++)
			{
				this->sources[i]->stop();
				id = this->sources[i]->getId();
				alDeleteSources(1, &id);
				delete this->sources[i];
			}
			alcMakeContextCurrent(NULL);
			alcDestroyContext(gContext);
			alcCloseDevice(gDevice);
		}
	}
	
/******* PROPERTIES ****************************************************/

	void AudioManager::logMessage(chstr message)
	{
		gLogFunction(message);
	}
	
	bool AudioManager::isEnabled()
	{
		return (gDevice != NULL);
	}

	void AudioManager::update(float k)
	{
		if (this->deviceName != "nosound")
		{
			for (int i = 0; i < XAL_MAX_SOURCES; i++)
			{
				this->sources[i]->update(k);
			}
		}
	}

	Source* AudioManager::allocateSource()
	{
		for (int i = 0; i < XAL_MAX_SOURCES; i++)
		{
			if (!this->sources[i]->hasSound())
			{
				return this->sources[i];
			}
		}
		this->logMessage("AudioManager: Unable to allocate audio source!");
		return NULL;
	}

	Sound* AudioManager::getSound(chstr name)
	{
		if (this->sounds.find(name) == this->sounds.end())
		{
			return NULL;
		}
		return this->sounds[name];
	}
	
	Sound* AudioManager::createSound(chstr filename, chstr categoryName, chstr prefix)
	{
		Category* category = this->getCategoryByName(categoryName);
		Sound* sound;
		if (category->isStreamed())
		{
			sound = new StreamSound(filename, categoryName, prefix);
		}
		else
		{
			sound = new SimpleSound(filename, categoryName, prefix);
		}
		if (!sound->load())
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
		harray<hstr> dirs = getPathDirectories(path);
		for (hstr* it = dirs.iterate(); it; it = dirs.next())
		{
			category = (*it).rsplit("/").pop_back();
			result += this->createSoundsFromPath(hsprintf("%s/%s", path.c_str(), (*it).c_str()), category, prefix);
		}
		return result;
	}

	harray<hstr> AudioManager::createSoundsFromPath(chstr path, chstr category, chstr prefix)
	{
		this->createCategory(category);
		harray<hstr> result;
		harray<hstr> files = getPathFilesRecursive(path);
		Sound* sound;
		for (hstr* it = files.iterate(); it; it = files.next())
		{
			sound = this->createSound(hsprintf("%s/%s", path.c_str(), (*it).c_str()), category, prefix);
			if (sound != NULL)
			{
				result += sound->getName();
			}
		}
		return result;
	}

	void AudioManager::destroySound(Sound* sound)
	{
		std::map<hstr, Sound*>::iterator it = this->sounds.begin();
		for (;it != this->sounds.end(); it++)
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
		std::map<hstr, Sound*>::iterator it = this->sounds.begin();
		for (;it != this->sounds.end(); it++)
		{
			if (it->first.starts_with(prefix))
			{
				delete it->second;
				deleteList.push_back(it->first);
			}
		}
		for (hstr* it = deleteList.iterate(); it ; it = deleteList.next())
		{
			this->sounds.erase(*it);
		}
	}

	void AudioManager::createCategory(chstr name, bool streamed)
	{
		if (this->categories.find(name) == this->categories.end())
		{
			this->categories[name] = new Category(name, streamed);
		}
	}

	Category* AudioManager::getCategoryByName(chstr name)
	{
		if (this->categories.find(name) == this->categories.end())
		{
			throw ("AudioManager: Category '" + name + "' does not exist!").c_str();
		}
		return this->categories[name];
	}

	void AudioManager::setCategoryGain(chstr name, float gain)
	{
		this->getCategoryByName(name)->setGain(gain);
		for (int i = 0; i < XAL_MAX_SOURCES; i++)
		{
			if (this->sources[i]->hasSound())
			{
				alSourcef(this->sources[i]->getId(), AL_GAIN, gain * this->sources[i]->getGain());
			}
		}
	}

}
