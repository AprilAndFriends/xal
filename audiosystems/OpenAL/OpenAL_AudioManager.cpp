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
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for OpenAL.

#if HAVE_OPENAL
#include <hltypes/exception.h>
#include <hltypes/hdir.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#ifndef __APPLE__
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <TargetConditionals.h>
#endif

#include "Buffer.h"
#include "Category.h"
#include "OpenAL_AudioManager.h"
#include "OpenAL_Player.h"
#include "xal.h"

namespace xal
{
	OpenAL_AudioManager::OpenAL_AudioManager(chstr systemName, unsigned long backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, deviceName, threaded, updateTime), device(NULL), context(NULL)
	{
		xal::log("initializing OpenAL");
		ALCdevice* currentDevice = alcOpenDevice(deviceName.c_str());
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			xal::log("could not create device");
			return;
		}
		this->deviceName = alcGetString(currentDevice, ALC_DEVICE_SPECIFIER);
		xal::log("audio device: " + this->deviceName);
		ALCcontext* currentContext = alcCreateContext(currentDevice, NULL);
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			xal::log("could not create context");
			return;
		}
		alcMakeContextCurrent(currentContext);
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			xal::log("could not set context as current");
			return;
		}
		alGenSources(XAL_MAX_SOURCES, this->sourceIds);
		this->device = currentDevice;
		this->context = currentContext;
		this->enabled = true;
		if (threaded)
		{
			this->_setupThread();
		}
	}

	OpenAL_AudioManager::~OpenAL_AudioManager()
	{
		this->clear();
		xal::log("destroying OpenAL");
		if (this->device != NULL)
		{
			alDeleteSources(XAL_MAX_SOURCES, this->sourceIds);
			alcMakeContextCurrent(NULL);
			alcDestroyContext(this->context);
			alcCloseDevice(this->device);
		}
	}
	
	Player* OpenAL_AudioManager::_createAudioPlayer(Sound* sound, Buffer* buffer)
	{
		return new OpenAL_Player(sound, buffer);
	}
	
	unsigned int OpenAL_AudioManager::allocateSourceId()
	{
		harray<unsigned int> allocated;
		OpenAL_Player* player;
		unsigned int id;
		foreach (Player*, it, this->players)
		{
			player = dynamic_cast<OpenAL_Player*>(*it);
			if (player != NULL)
			{
				id = player->getSourceId();
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

}
#endif