/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.0
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

#ifndef __APPLE__
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <TargetConditionals.h>
#endif

#include <hltypes/exception.h>
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "Buffer.h"
#include "Category.h"
#include "OpenAL_AudioManager.h"
#include "OpenAL_Player.h"
#include "xal.h"

#ifdef _IOS
void OpenAL_iOS_init();
void OpenAL_iOS_destroy();
static bool gAudioSuspended = false; // iOS specific hack as well
#endif

namespace xal
{
	OpenAL_AudioManager::OpenAL_AudioManager(chstr systemName, void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName), device(NULL), context(NULL)
	{
		hlog::write(xal::logTag, "Initializing OpenAL.");
		ALCdevice* currentDevice = alcOpenDevice(deviceName.c_str());
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			hlog::error(xal::logTag, "Could not create device!");
			return;
		}
		this->deviceName = alcGetString(currentDevice, ALC_DEVICE_SPECIFIER);
		hlog::write(xal::logTag, "Audio device: " + this->deviceName);
		ALCcontext* currentContext = alcCreateContext(currentDevice, NULL);
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			hlog::error(xal::logTag, "Could not create context!");
			return;
		}
		alcMakeContextCurrent(currentContext);
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			hlog::error(xal::logTag, "Could not set context as current!");
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
		hlog::write(xal::logTag, "Destroying OpenAL.");
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
	
	Player* OpenAL_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new OpenAL_Player(sound);
	}
	
	unsigned int OpenAL_AudioManager::_allocateSourceId()
	{
		unsigned int id = 0;
		alGenSources(1, &id);
		if (alGetError() != AL_NO_ERROR)
		{
			hlog::warn(xal::logTag, "Unable to allocate audio source!");
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
#ifdef _IOS
		this->_lock();
		hlog::debug(xal::logTag, "Suspending OpenAL Context.");
		gAudioSuspended = this->isSuspended();
		if (!gAudioSuspended)
		{
			this->_suspendAudio();
		}
		alcSuspendContext(this->context);
		alcMakeContextCurrent(NULL);
		this->_unlock();
#else
		hlog::debug(xal::logTag, "Not iOS, suspendOpenALContext does nothing.");
#endif
	}

	void OpenAL_AudioManager::resumeOpenALContext() // TODO - iOS specific hack, should be removed later
	{
#ifdef _IOS
		this->_lock();
		hlog::debug(xal::logTag, "Resuming OpenAL Context.");
		alcMakeContextCurrent(this->context);
		alcProcessContext(this->context);
		if (!gAudioSuspended)
		{
			this->_resumeAudio();
		}
		this->_unlock();
#else
		hlog::debug(xal::logTag, "Not iOS, resumeOpenALContext does nothing.");
#endif
	}
}
#endif
