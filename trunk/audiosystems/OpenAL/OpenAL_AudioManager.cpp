/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for OpenAL.

#ifdef HAVE_OPENAL
#include <string.h>

#include <hltypes/exception.h>
#include <hltypes/hdir.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

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

#ifdef _IOS
void OpenAL_iOS_init();
void OpenAL_iOS_destroy();
#endif

namespace xal
{
	OpenAL_AudioManager::OpenAL_AudioManager(chstr systemName, void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName), device(NULL), context(NULL)
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
		this->device = currentDevice;
		this->context = currentContext;
		this->enabled = true;
#ifdef _IOS
		OpenAL_iOS_init();
#endif
	}

	OpenAL_AudioManager::~OpenAL_AudioManager()
	{
		xal::log("destroying OpenAL");
#ifdef _IOS
		OpenAL_iOS_destroy();
#endif
		if (this->device != NULL)
		{
			alcMakeContextCurrent(NULL);
			alcDestroyContext(this->context);
			alcCloseDevice(this->device);
		}
	}
	
	Player* OpenAL_AudioManager::_createSystemPlayer(Sound* sound, Buffer* buffer)
	{
		return new OpenAL_Player(sound, buffer);
	}
	
	unsigned int OpenAL_AudioManager::_allocateSourceId()
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

	void OpenAL_AudioManager::_releaseSourceId(unsigned int sourceId)
	{
		alDeleteSources(1, &sourceId);
	}
	
	void OpenAL_AudioManager::suspendOpenALContext() // TODO - iOS specific hack, should be removed later
	{
		this->_lock();
#ifdef _DEBUG
		xal::log("suspending OpenAL Context");
#endif
		this->_suspendAudio();
		alcSuspendContext(this->context);
		alcMakeContextCurrent(NULL);
		this->_unlock();
	}

	void OpenAL_AudioManager::resumeOpenALContext() // TODO - iOS specific hack, should be removed later
	{
		this->_lock();
#ifdef _DEBUG
		xal::log("resuming OpenAL Context");
#endif
		alcMakeContextCurrent(this->context);
		alcProcessContext(this->context);
		this->_resumeAudio();
		this->_unlock();
	}

}
#endif
