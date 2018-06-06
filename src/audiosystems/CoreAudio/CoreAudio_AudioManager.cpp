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
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "Buffer.h"
#include "Category.h"
#include "CoreAudio_AudioManager.h"
#include "CoreAudio_Player.h"
#include "Sound.h"
#include "xal.h"

namespace xal
{
	CoreAudio_AudioManager::CoreAudio_AudioManager(void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(backendId, threaded, updateTime, deviceName)
	{
		this->name = XAL_AS_COREAUDIO;
		hlog::write(logTag, "Initializing CoreAudio.");
	}

	CoreAudio_AudioManager::~CoreAudio_AudioManager()
	{
		hlog::write(logTag, "Destroying CoreAudio.");
	}
	
	Player* CoreAudio_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new CoreAudio_Player(sound);
	}
	
	void CoreAudio_AudioManager::_suspendAudio()
	{
		AudioManager::_suspendAudio();
		//this->pendingResume = false;
	}

	void CoreAudio_AudioManager::_resumeAudio()
	{
		AudioManager::_resumeAudio();
		//this->pendingResume = false;
	}

	void CoreAudio_AudioManager::_update(float timeDelta)
	{
		/*
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
		*/
		AudioManager::_update(timeDelta);
	}

}
#endif
