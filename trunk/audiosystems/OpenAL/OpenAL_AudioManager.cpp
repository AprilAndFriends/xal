/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.01
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
bool OpenAL_iOS_isAudioSessionActive();
bool restoreiOSAudioSession();
bool hasiOSAudioSessionRestoreFailed();
static bool gAudioSuspended = false; // iOS specific hack as well
#endif

static hstr alGetErrorString(ALenum error)
{
	switch (error)
	{
		case AL_NO_ERROR:
			return "AL_NO_ERROR";
		case AL_INVALID_NAME:
			return "AL_INVALID_NAME";
		case AL_INVALID_ENUM:
			return "AL_INVALID_ENUM";
		case AL_INVALID_VALUE:
			return "AL_INVALID_VALUE";
		case AL_INVALID_OPERATION:
			return "AL_INVALID_OPERATION";
		case AL_OUT_OF_MEMORY:
			return "AL_OUT_OF_MEMORY";
		default:
			return "UNKNOWN";
	};
}

static hstr alcGetErrorStrirg(ALenum error)
{
	switch (error)
	{
		case ALC_NO_ERROR:
			return "AL_NO_ERROR";
		case ALC_INVALID_DEVICE:
			return "ALC_INVALID_DEVICE";
		case ALC_INVALID_CONTEXT:
			return "ALC_INVALID_CONTEXT";
		case ALC_INVALID_ENUM:
			return "ALC_INVALID_ENUM";
		case ALC_INVALID_VALUE:
			return "ALC_INVALID_VALUE";
		case ALC_OUT_OF_MEMORY:
			return "ALC_OUT_OF_MEMORY";
		default:
			return "UNKNOWN";
	};
}

namespace xal
{
	OpenAL_AudioManager::OpenAL_AudioManager(chstr systemName, void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName), device(NULL), context(NULL)
	{
		hlog::write(xal::logTag, "Initializing OpenAL.");
		ALCdevice* currentDevice = alcOpenDevice(deviceName.c_str());
		ALenum error = alcGetError(currentDevice);
		if (error != ALC_NO_ERROR)
		{
			hlog::error(xal::logTag, "Could not create device!, " + alcGetErrorStrirg(error));
			return;
		}
		this->deviceName = alcGetString(currentDevice, ALC_DEVICE_SPECIFIER);
		hlog::write(xal::logTag, "Audio device: " + this->deviceName);
		ALCcontext* currentContext = alcCreateContext(currentDevice, NULL);
		error = alcGetError(currentDevice);
		if (error != ALC_NO_ERROR)
		{
			hlog::error(xal::logTag, "Could not create context!, " + alcGetErrorStrirg(error));
			return;
		}
		alcMakeContextCurrent(currentContext);
		error = alcGetError(currentDevice);
		if (error != ALC_NO_ERROR)
		{
			hlog::error(xal::logTag, "Could not set context as current!, " + alcGetErrorStrirg(error));
			return;
		}
		this->device = currentDevice;
		this->context = currentContext;
		this->enabled = true;
#ifdef _IOS
		this->pendingResume = false;
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
		ALenum error = alGetError();
		if (error != AL_NO_ERROR)
		{
			hlog::warn(xal::logTag, "Unable to allocate audio source! " + alGetErrorString(error));
			return 0;
		}
		return id;
	}

	void OpenAL_AudioManager::_releaseSourceId(unsigned int sourceId)
	{
		alDeleteSources(1, &sourceId);
	}
	
#ifdef _IOS
	void OpenAL_AudioManager::_resumeAudio()
	{
		if (!OpenAL_iOS_isAudioSessionActive())
		{
			this->pendingResume = true;
			return;
		}
		AudioManager::_resumeAudio();
		this->pendingResume = false;
	}
	
	void OpenAL_AudioManager::_suspendAudio()
	{
		this->pendingResume = false;
		AudioManager::_suspendAudio();
	}

	void OpenAL_AudioManager::_update(float k)
	{
#ifdef _IOS
		if (hasiOSAudioSessionRestoreFailed())
		{
			if (!restoreiOSAudioSession())
			{
				hthread::sleep(50);
				return;
			}
		}
#endif
		if (this->pendingResume) _resumeAudio();
		AudioManager::_update(k);
	}
#endif
	
	void OpenAL_AudioManager::suspendOpenALContext() // iOS specific hack
	{
#ifdef _IOS
		this->_lock();
		hlog::write(xal::logTag, "Suspending OpenAL Context.");
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

	void OpenAL_AudioManager::resumeOpenALContext() // iOS specific hack
	{
#ifdef _IOS
		if (!hasiOSAudioSessionRestoreFailed()) this->_lock(); // don't lock because at this point we're already locked
		hlog::write(xal::logTag, "Resuming OpenAL Context.");
		alcMakeContextCurrent(this->context);
		alcProcessContext(this->context);
		if (!gAudioSuspended)
		{
			this->_resumeAudio();
		}
		if (!hasiOSAudioSessionRestoreFailed()) this->_unlock();
#else
		hlog::debug(xal::logTag, "Not iOS, resumeOpenALContext does nothing.");
#endif
	}
}
#endif
