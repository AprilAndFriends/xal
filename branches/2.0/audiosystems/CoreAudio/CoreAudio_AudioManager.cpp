/// @file
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
/// Represents an implementation of the AudioManager for CoreAudio.
/// This implementation uses Audio Queue Services from the
/// Audio Toolbox framework.

#if HAVE_COREAUDIO
#include <string.h>

#include <hltypes/exception.h>
#include <hltypes/hdir.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include <AudioToolbox/AudioToolbox.h>

#include "Buffer.h"
#include "Category.h"
#include "CoreAudio_AudioManager.h"
#include "CoreAudio_Player.h"
#include "xal.h"

namespace xal
{
	CoreAudio_AudioManager::CoreAudio_AudioManager(chstr systemName, unsigned long backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName)
	{
		xal::log("initializing CoreAudio");
		/*
		memset(this->allocated, 0, COREAUDIO_MAX_SOURCES * sizeof(bool));
		ALCdevice* currentDevice = alcOpenDevice(deviceName.c_str());
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			xal::log("could not create device");
			return;
		}
		this->deviceName = alcGetString(currentDevice, ALC_DEVICE_SPECIFIER);
		 */
		xal::log("audio device: " + this->deviceName);
		/*
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
		alGenSources(COREAUDIO_MAX_SOURCES, this->sourceIds);
		this->device = currentDevice;
		this->context = currentContext;
		this->enabled = true;
		if (threaded)
		{
			this->_setupThread();
		}
		 */
	}

	CoreAudio_AudioManager::~CoreAudio_AudioManager()
	{
		xal::log("destroying CoreAudio");
		/*
		if (this->device != NULL)
		{
			alDeleteSources(COREAUDIO_MAX_SOURCES, this->sourceIds);
			alcMakeContextCurrent(NULL);
			alcDestroyContext(this->context);
			alcCloseDevice(this->device);
		}
		 */
	}
	
	Player* CoreAudio_AudioManager::_createAudioPlayer(Sound* sound, Buffer* buffer)
	{
		return new CoreAudio_Player(sound, buffer);
	}
	
	unsigned int CoreAudio_AudioManager::_allocateSourceId()
	{
		/*
		for (int i = 0; i < COREAUDIO_MAX_SOURCES; i++)
		{
			if (!this->allocated[i])
			{
				this->allocated[i] = true;
				return this->sourceIds[i];
			}
		}
		 */
		xal::log("unable to allocate audio source!");
		return 0;
	}

	void CoreAudio_AudioManager::_releaseSourceId(unsigned int sourceId)
	{
		/*
		for (int i = 0; i < COREAUDIO_MAX_SOURCES; i++)
		{
			if (this->sourceIds[i] == sourceId)
			{
				this->allocated[i] = false;
				break;
			}
		}
		 */
	}

}
#endif
