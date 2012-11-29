/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_XAUDIO2
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "XAudio2_AudioManager.h"
#include "XAudio2_Player.h"
#include "xal.h"

namespace xal
{
	XAudio2_AudioManager::XAudio2_AudioManager(chstr systemName, void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName)
	{
		hlog::write(xal::logTag, "Initializing XAudio2.");
		this->enabled = false;
	}

	XAudio2_AudioManager::~XAudio2_AudioManager()
	{
		hlog::write(xal::logTag, "Destroying XAudio2.");
	}
	
	Player* XAudio2_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new XAudio2_Player(sound);
	}

}
#endif
