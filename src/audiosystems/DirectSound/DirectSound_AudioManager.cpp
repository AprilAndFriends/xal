/// @file
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTSOUND
#include <dsound.h>


#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "DirectSound_AudioManager.h"
#include "DirectSound_Player.h"
/*
#ifdef _FORMAT_WAV
#include "DirectSound_WAV_Source.h"
#endif
*/
#include "xal.h"

namespace xal
{
	DirectSound_AudioManager::DirectSound_AudioManager(void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(backendId, threaded, updateTime, deviceName)
	{
		this->name = XAL_AS_DIRECTSOUND;
		hlog::write(xal::logTag, "Initializing DirectSound.");
		HRESULT result = DirectSoundCreate(NULL, &this->dsDevice, NULL);
		if (FAILED(result))
		{
			this->dsDevice = NULL;
			hlog::error(xal::logTag, "Could not create device!");
			return;
		}
		result = this->dsDevice->SetCooperativeLevel((HWND)backendId, DSSCL_NORMAL);
		if (FAILED(result))
		{
			this->dsDevice->Release();
			this->dsDevice = NULL;
			hlog::error(xal::logTag, "Could not set cooperative level!");
			return;
		}
		this->dsCaps = new _DSCAPS();
		memset(this->dsCaps, 0, sizeof(_DSCAPS));
		this->dsCaps->dwSize = sizeof(_DSCAPS);
		result = this->dsDevice->GetCaps(this->dsCaps);
		if (result != DS_OK)
		{
			this->dsCaps->dwMaxSecondarySampleRate = 100000; // just in case. 100k is guaranteed to be always supported on all DS hardware
			hlog::error(xal::logTag, "Failed fetching DirectSound device caps");
		}
		else
		{
			hlog::writef(xal::logTag, "DirectSound device caps: { maxSampleRate = %u }", this->dsCaps->dwMaxSecondarySampleRate);
		}
		this->enabled = true;
	}

	DirectSound_AudioManager::~DirectSound_AudioManager()
	{
		hlog::write(xal::logTag, "Destroying DirectSound.");
		if (this->dsDevice != NULL)
		{
			this->dsDevice->Release();
			this->dsDevice = NULL;
			delete this->dsCaps;
			this->dsCaps = NULL;
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
#ifdef _FORMAT_WAV
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