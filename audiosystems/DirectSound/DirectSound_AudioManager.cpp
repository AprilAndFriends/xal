/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_DIRECTSOUND
#include <dsound.h>

#include <hltypes/hstring.h>

#include "DirectSound_AudioManager.h"
#include "DirectSound_Player.h"
#include "xal.h"

namespace xal
{
	DirectSound_AudioManager::DirectSound_AudioManager(unsigned long backendId, chstr deviceName, bool threaded, float updateTime) :
		AudioManager(backendId, deviceName, threaded, updateTime)
	{
		xal::log("initializing DirectSound");
		HRESULT result = DirectSoundCreate(NULL, &this->dsDevice, NULL);
		if (result != DS_OK)
		{
			this->dsDevice = NULL;
			xal::log("could not create device");
			return;
		}
		result = this->dsDevice->SetCooperativeLevel((HWND)backendId, DSSCL_NORMAL);
		if (result != DS_OK)
		{
			this->dsDevice->Release();
			this->dsDevice = NULL;
			xal::log("could not set cooperative level");
			return;
		}
		this->enabled = true;
	}

	DirectSound_AudioManager::~DirectSound_AudioManager()
	{
		this->clear();
		xal::log("destroying DirectSound");
		if (this->dsDevice != NULL)
		{
			this->dsDevice->Release();
			this->dsDevice = NULL;
		}
	}
	
	Player* DirectSound_AudioManager::_createAudioPlayer(Sound* sound, Buffer* buffer)
	{
		return new DirectSound_Player(sound, buffer);
	}

}
#endif