/// @file
/// @author  Boris Mikic
/// @version 2.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hstring.h>

#include "NoAudio_AudioManager.h"
#include "NoAudio_Player.h"
#include "xal.h"

namespace xal
{
	NoAudio_AudioManager::NoAudio_AudioManager(chstr systemName, void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName)
	{
		xal::log("initializing NoAudio");
		this->enabled = false;
	}

	NoAudio_AudioManager::~NoAudio_AudioManager()
	{
		xal::log("destroying NoAudio");
	}
	
	Player* NoAudio_AudioManager::_createSystemPlayer(Sound* sound, Buffer* buffer)
	{
		return new NoAudio_Player(sound, buffer);
	}

}
