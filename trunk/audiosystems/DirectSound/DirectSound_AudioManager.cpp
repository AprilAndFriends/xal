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
/*
#if HAVE_WAV
#include "DirectSound_WAV_Source.h"
#endif
*/
#include "xal.h"

namespace xal
{
	DirectSound_AudioManager::DirectSound_AudioManager(chstr systemName, unsigned long backendId, bool threaded, float updateTime, chstr deviceName) :
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
	
	Player* DirectSound_AudioManager::_createPlayer(Sound* sound, Buffer* buffer)
	{
		return new DirectSound_Player(sound, buffer);
	}

	Source* DirectSound_AudioManager::_createSource(chstr filename, Format format)
	{
		/*
		Source* source;
		switch (format)
		{
#if HAVE_WAV
		case WAV:
			source = new DirectSound_WAV_Source(filename);
			break;
#endif
		default:
			source = AudioManager::_createSource(filename, format);
			break;
		}
		return source;
		*/
		return AudioManager::_createSource(filename, format);
	}

}
#endif