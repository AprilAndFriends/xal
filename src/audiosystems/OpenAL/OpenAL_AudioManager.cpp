/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for OpenAL.

#ifdef _OPENAL
#include <string.h>

#ifndef __APPLE__
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <TargetConditionals.h>
#endif

#include <hltypes/hdir.h>
#include <hltypes/hexception.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "Buffer.h"
#include "Category.h"
#include "OpenAL_AudioManager.h"
#include "OpenAL_Player.h"
#include "Sound.h"
#include "xal.h"

#ifdef __ANDROID__
extern "C" int __openal__JNI_OnLoad(void* vm);
#elif defined(_IOS)
#include <sys/sysctl.h>

void OpenAL_iOS_init();
void OpenAL_iOS_destroy();
bool OpenAL_iOS_isAudioSessionActive();
bool restoreiOSAudioSession();
bool _restoreiOSAudioSession();
bool hasiOSAudioSessionRestoreFailed();
#endif

#define _CASE_STRING(x) case x: return #x;

static hstr alGetErrorString(ALenum error)
{
	switch (error)
	{
	_CASE_STRING(AL_NO_ERROR);
	_CASE_STRING(AL_INVALID_NAME);
	_CASE_STRING(AL_INVALID_ENUM);
	_CASE_STRING(AL_INVALID_VALUE);
	_CASE_STRING(AL_INVALID_OPERATION);
	_CASE_STRING(AL_OUT_OF_MEMORY);
	default:
		return "AL_UNKNOWN";
	};
}

static hstr alcGetErrorString(ALCenum error)
{
	switch (error)
	{
	_CASE_STRING(ALC_NO_ERROR);
	_CASE_STRING(ALC_INVALID_DEVICE);
	_CASE_STRING(ALC_INVALID_CONTEXT);
	_CASE_STRING(ALC_INVALID_ENUM);
	_CASE_STRING(ALC_INVALID_VALUE);
	_CASE_STRING(ALC_OUT_OF_MEMORY);
	default:
		return "ALC_UNKNOWN";
	};
}

namespace xal
{
	OpenAL_AudioManager::OpenAL_AudioManager(void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(backendId, threaded, updateTime, deviceName),
		device(NULL),
		context(NULL)
	{
		this->name = AudioSystemType::OpenAL.getName();
		hlog::write(logTag, "Initializing " + this->name + ".");
#ifdef __ANDROID__
		__openal__JNI_OnLoad(backendId);
#endif
		this->numActiveSources = 0;
		this->initOpenAL();
	}

	OpenAL_AudioManager::~OpenAL_AudioManager()
	{
		hlog::write(logTag, "Destroying " + this->name + ".");
		this->destroyOpenAL();
	}
	
	void OpenAL_AudioManager::initOpenAL()
	{
		ALCdevice* currentDevice = alcOpenDevice(this->deviceName.cStr());
		ALenum error = alcGetError(currentDevice);
		if (error != ALC_NO_ERROR)
		{
			hlog::error(logTag, "Could not create device!, " + alcGetErrorString(error));
			return;
		}
		this->deviceName = alcGetString(currentDevice, ALC_DEVICE_SPECIFIER);
		hlog::write(logTag, "Audio device: " + this->deviceName);
#ifdef _IOS
		// iOS generates only 4 stereo sources by default, so lets override that
		ALCint params[5] = {ALC_STEREO_SOURCES, 32, ALC_MONO_SOURCES, 32, 0};
		ALCcontext* currentContext = alcCreateContext(currentDevice, params);
#else
		ALCcontext* currentContext = alcCreateContext(currentDevice, NULL);
#endif
		error = alcGetError(currentDevice);
		if (error != ALC_NO_ERROR)
		{
			hlog::error(logTag, "Could not create context!, " + alcGetErrorString(error));
			return;
		}
		alcMakeContextCurrent(currentContext);
		error = alcGetError(currentDevice);
		if (error != ALC_NO_ERROR)
		{
			hlog::error(logTag, "Could not set context as current!, " + alcGetErrorString(error));
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
	
	void OpenAL_AudioManager::destroyOpenAL()
	{
#ifdef _IOS // you can't touch this, there may be dragons
		OpenAL_iOS_destroy();
#endif
		if (this->device != NULL)
		{
			alcMakeContextCurrent(NULL);
			alcDestroyContext(this->context);
			alcCloseDevice(this->device);
		}
	}
	
	void OpenAL_AudioManager::resetOpenAL()
	{
		hlog::write(logTag, "Restarting OpenAL.");
		foreach (Player*, it, this->players)
		{
			((OpenAL_Player*)*it)->destroyOpenALBuffers();
		}
		this->destroyOpenAL();
		this->initOpenAL();
		foreach (Player*, it, this->players)
		{
			((OpenAL_Player*)*it)->createOpenALBuffers();
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
		if (id == 0)
		{
			ALenum error = alGetError();
			if (error != AL_NO_ERROR)
			{
				hlog::warnf(logTag, "Unable to allocate audio source! error = %s, numActiveSources = %d", alGetErrorString(error).cStr(), this->numActiveSources);
				return 0;
			}
		}
		++this->numActiveSources;
		//hlog::debug(logTag, hsprintf("Allocated source: %d, currently active sources: %d", id, this->numActiveSources));
		return id;
	}

	void OpenAL_AudioManager::_releaseSourceId(unsigned int sourceId)
	{
		if (sourceId != 0)
		{
			--this->numActiveSources;
			alDeleteSources(1, &sourceId);
		}
		//hlog::debug(logTag, hsprintf("Released source: %d, currently active sources: %d", sourceId, this->numActiveSources));
	}
	
	bool OpenAL_AudioManager::resumeOpenALContext() // iOS specific hack
	{
		hmutex::ScopeLock lock(&this->mutex); // otherwise don't lock because at this point we're already locked
		return this->_resumeOpenALContext();
	}

#ifdef _IOS
	void OpenAL_AudioManager::_suspendAudio()
	{
		AudioManager::_suspendAudio();
		this->pendingResume = false;
	}

	void OpenAL_AudioManager::_resumeAudio()
	{
		AudioManager::_resumeAudio();
		this->pendingResume = false;
	}

	void OpenAL_AudioManager::_update(float timeDelta)
	{
		if (hasiOSAudioSessionRestoreFailed())
		{
			if (!_restoreiOSAudioSession())
			{
				hthread::sleep(50.0f);
				return;
			}
		}
		if (this->pendingResume)
		{
			this->_resumeAudio();
		}
		AudioManager::_update(timeDelta);
	}

	void OpenAL_AudioManager::suspendOpenALContext() // iOS specific hack
	{
		hmutex::ScopeLock lock(&this->mutex);
		hlog::write(logTag, "Suspending OpenAL Context.");
		// all sounds must released their source IDs immediately to avoid issues with audio system state corruption
		float suspendResumeFadeTime = this->suspendResumeFadeTime;
		this->suspendResumeFadeTime = 0.0f;
		AudioManager::_suspendAudio();
		this->suspendResumeFadeTime = suspendResumeFadeTime;
		alcMakeContextCurrent(NULL);
		alcSuspendContext(this->context);
	}
	
	bool OpenAL_AudioManager::_resumeOpenALContext() // iOS specific hack
	{
		ALCenum error = ALC_NO_ERROR;
		hlog::write(logTag, "Resuming OpenAL Context.");
		alcMakeContextCurrent(this->context);
		this->pendingResume = true;
		if ((error = alcGetError(this->device)) == ALC_NO_ERROR)
		{
			alcProcessContext(this->context);
			if ((error = alcGetError(this->device)) == ALC_NO_ERROR)
			{
				this->_resumeAudio();
			}
		}
		if (error != ALC_NO_ERROR)
		{
			hlog::write(logTag, "Failed resuming OpenAL Context, will try again later: " + alcGetErrorString(error));
			return false;
		}
		return true;
	}
#else
	void OpenAL_AudioManager::suspendOpenALContext() // iOS specific hack
	{
		hlog::debug(logTag, "Not iOS, suspendOpenALContext() ignored.");
	}
	
	bool OpenAL_AudioManager::_resumeOpenALContext() // iOS specific hack
	{
		hlog::debug(logTag, "Not iOS, resumeOpenALContext() ignored.");
		return true;
	}
#endif

}
#endif
