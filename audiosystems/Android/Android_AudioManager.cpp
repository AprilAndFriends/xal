/// @file
/// @author  Boris Mikic
/// @version 2.7
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for Android.

#ifdef HAVE_ANDROID
#include <string.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <hltypes/exception.h>
#include <hltypes/hdir.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "Android_AudioManager.h"
#include "Android_Player.h"
#include "Buffer.h"
#include "Category.h"
#include "xal.h"

extern "C" int __openal__JNI_OnLoad(void* vm);

namespace xal
{
	Android_AudioManager::Android_AudioManager(chstr systemName, void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName), device(NULL), context(NULL)
	{
		__openal__JNI_OnLoad(backendId);
		xal::log("initializing Android Audio");
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
		this->device = currentDevice;
		this->context = currentContext;
		this->enabled = true;
	}

	Android_AudioManager::~Android_AudioManager()
	{
		xal::log("destroying Android Audio");
		if (this->device != NULL)
		{
			alcMakeContextCurrent(NULL);
			alcDestroyContext(this->context);
			alcCloseDevice(this->device);
		}
	}
	
	Player* Android_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new Android_Player(sound);
	}
	
	unsigned int Android_AudioManager::_allocateSourceId()
	{
		unsigned int id = 0;
		alGenSources(1, &id);
		if (alGetError() != AL_NO_ERROR)
		{
			xal::log("unable to allocate audio source!");
			return 0;
		}
		return id;
	}

	void Android_AudioManager::_releaseSourceId(unsigned int sourceId)
	{
		alDeleteSources(1, &sourceId);
	}
	
}
#endif
