/// @file
/// @author  Boris Mikic
/// @version 2.7
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_DIRECTSOUND
#include <dsound.h>

#include <hltypes/hstring.h>

#include "DirectSound_AudioManager.h"
#include "DirectSound_Player.h"
/*
#ifdef HAVE_WAV
#include "DirectSound_WAV_Source.h"
#endif
*/
#include "xal.h"

namespace xal
{
	DirectSound_AudioManager::DirectSound_AudioManager(chstr systemName, void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName)
	{
		xal::log("initializing DirectSound");
		HRESULT result = DirectSoundCreate(NULL, &this->dsDevice, NULL);
		if (FAILED(result))
		{
			this->dsDevice = NULL;
			xal::log("could not create device");
			return;
		}
		result = this->dsDevice->SetCooperativeLevel((HWND)backendId, DSSCL_NORMAL);
		if (FAILED(result))
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
		xal::log("destroying DirectSound");
		if (this->dsDevice != NULL)
		{
			this->dsDevice->Release();
			this->dsDevice = NULL;
		}
	}
	
	Player* DirectSound_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new DirectSound_Player(sound);
	}

	Source* DirectSound_AudioManager::_createSource(chstr filename, Category* category, Format format)
	{
		/*
		Source* source;
		switch (format)
		{
#ifdef HAVE_WAV
		case WAV:
			source = new DirectSound_WAV_Source(filename, category);
			break;
#endif
		default:
			source = AudioManager::_createSource(filename, category, format);
			break;
		}
		return source;
		*/
		return AudioManager::_createSource(filename, category, format);
	}

}
#endif