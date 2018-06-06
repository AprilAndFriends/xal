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
/// Represents an implementation of the AudioManager for CoreAudio.

#ifdef _COREAUDIO
#include <string.h>

#include <CoreAudio/al.h>
#include <CoreAudio/alc.h>
#include <TargetConditionals.h>

#include <hltypes/hdir.h>
#include <hltypes/hexception.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "Buffer.h"
#include "Category.h"
#include "CoreAudio_AudioManager.h"
#include "CoreAudio_Player.h"
#include "Sound.h"
#include "xal.h"

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
	CoreAudio_AudioManager::CoreAudio_AudioManager(void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(backendId, threaded, updateTime, deviceName), device(NULL), context(NULL)
	{
		this->name = XAL_AS_COREAUDIO;
		hlog::write(logTag, "Initializing CoreAudio.");
		this->numActiveSources = 0;
		this->initCoreAudio();
	}

	CoreAudio_AudioManager::~CoreAudio_AudioManager()
	{
		hlog::write(logTag, "Destroying CoreAudio.");
		this->destroyCoreAudio();
	}
	
	void CoreAudio_AudioManager::initCoreAudio()
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
		// iOS generates only 4 stereo sources by default, so lets override that
		ALCint params[5] = {ALC_STEREO_SOURCES, 16, ALC_MONO_SOURCES, 16, 0};
		ALCcontext* currentContext = alcCreateContext(currentDevice, params);
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
		this->pendingResume = false;
		CoreAudio_iOS_init();
	}
	
	void CoreAudio_AudioManager::destroyCoreAudio()
	{
		CoreAudio_iOS_destroy();
		if (this->device != NULL)
		{
			alcMakeContextCurrent(NULL);
			alcDestroyContext(this->context);
			alcCloseDevice(this->device);
		}
	}
	
	void CoreAudio_AudioManager::resetCoreAudio()
	{
		hlog::write(logTag, "Restarting CoreAudio.");
		foreach (Player*, it, this->players)
		{
			((CoreAudio_Player*)*it)->destroyCoreAudioBuffers();
		}
		this->destroyCoreAudio();
		this->initCoreAudio();
		foreach (Player*, it, this->players)
		{
			((CoreAudio_Player*)*it)->createCoreAudioBuffers();
		}
	}
	
	Player* CoreAudio_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new CoreAudio_Player(sound);
	}
	
	unsigned int CoreAudio_AudioManager::_allocateSourceId()
	{
		unsigned int id = 0;
		alGenSources(1, &id);
		ALenum error = alGetError();
		if (error != AL_NO_ERROR)
		{
			hlog::warnf(logTag, "Unable to allocate audio source! error = %s, numActiveSources = %d", alGetErrorString(error).cStr(), this->numActiveSources);
			return 0;
		}
		++this->numActiveSources;
		//hlog::debug(logTag, hsprintf("Allocated source: %d, currently active sources: %d", id, this->numActiveSources));
		return id;
	}

	void CoreAudio_AudioManager::_releaseSourceId(unsigned int sourceId)
	{
		if (sourceId != 0)
		{
			--this->numActiveSources;
			alDeleteSources(1, &sourceId);
		}
		//hlog::debug(logTag, hsprintf("Released source: %d, currently active sources: %d", sourceId, this->numActiveSources));
	}
	
	bool CoreAudio_AudioManager::resumeCoreAudioContext() // iOS specific hack
	{
		hmutex::ScopeLock lock(&this->mutex); // otherwise don't lock because at this point we're already locked
		return this->_resumeCoreAudioContext();
	}

	void CoreAudio_AudioManager::_suspendAudio()
	{
		AudioManager::_suspendAudio();
		this->pendingResume = false;
	}

	void CoreAudio_AudioManager::_resumeAudio()
	{
		AudioManager::_resumeAudio();
		this->pendingResume = false;
	}

	void CoreAudio_AudioManager::_update(float timeDelta)
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

	void CoreAudio_AudioManager::suspendCoreAudioContext() // iOS specific hack
	{
		hmutex::ScopeLock lock(&this->mutex);
		hlog::write(logTag, "Suspending CoreAudio Context.");
		// all sounds must released their source IDs immediately to avoid issues with audio system state corruption
		float suspendResumeFadeTime = this->suspendResumeFadeTime;
		this->suspendResumeFadeTime = 0.0f;
		AudioManager::_suspendAudio();
		this->suspendResumeFadeTime = suspendResumeFadeTime;
		alcMakeContextCurrent(NULL);
		alcSuspendContext(this->context);
	}
	
	bool CoreAudio_AudioManager::_resumeCoreAudioContext() // iOS specific hack
	{
		ALCenum error = ALC_NO_ERROR;
		hlog::write(logTag, "Resuming CoreAudio Context.");
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
			hlog::write(logTag, "Failed resuming CoreAudio Context, will try again later: " + alcGetErrorString(error));
			return false;
		}
		return true;
	}
#else
	void CoreAudio_AudioManager::suspendCoreAudioContext() // iOS specific hack
	{
		hlog::debug(logTag, "Not iOS, suspendCoreAudioContext() ignored.");
	}
	
	bool CoreAudio_AudioManager::_resumeCoreAudioContext() // iOS specific hack
	{
		hlog::debug(logTag, "Not iOS, resumeCoreAudioContext() ignored.");
		return true;
	}
#endif

}
#endif
