/// @file
/// @version 3.14
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _XAUDIO2
#include <xaudio2.h>

#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "XAudio2_AudioManager.h"
#include "XAudio2_Player.h"
#include "xal.h"

using namespace Microsoft::WRL;

namespace xal
{
	XAudio2_AudioManager::XAudio2_AudioManager(void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(backendId, threaded, updateTime, deviceName), xa2Device(NULL), xa2MasteringVoice(NULL)
	{
		this->name = XAL_AS_XAUDIO2;
		hlog::write(xal::logTag, "Initializing XAudio2.");
		HRESULT result = XAudio2Create(&this->xa2Device, 0);
		if (FAILED(result))
		{
			this->xa2Device = NULL;
			hlog::error(xal::logTag, "Could not create device!");
			return;
		}
		result = this->xa2Device->CreateMasteringVoice(&this->xa2MasteringVoice, 2, 44100);
		if (FAILED(result)) // if can't use 44.1 kHz stereo, use default
		{
			hlog::write(xal::logTag, "Could not create device with 2 channels and 44100 Hz, attempting defaults...");
			result = this->xa2Device->CreateMasteringVoice(&this->xa2MasteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE);
		}
		if (FAILED(result))
		{
			this->xa2Device->Release();
			this->xa2Device = NULL;
			hlog::error(xal::logTag, "Could not create mastering voice!");
			return;
		}
		result = this->xa2Device->StartEngine();
		if (FAILED(result))
		{
			this->xa2MasteringVoice->DestroyVoice();
			this->xa2MasteringVoice = NULL;
			this->xa2Device->Release();
			this->xa2Device = NULL;
			hlog::error(xal::logTag, "Could not start engine!");
			return;
		}
		this->enabled = true;
	}

	XAudio2_AudioManager::~XAudio2_AudioManager()
	{
		hlog::write(xal::logTag, "Destroying XAudio2.");
		this->xa2Device->StopEngine();
		if (this->xa2MasteringVoice != NULL)
		{
			this->xa2MasteringVoice->DestroyVoice();
			this->xa2MasteringVoice = NULL;
		}
		_HL_TRY_RELEASE(this->xa2Device);
	}
	
	void XAudio2_AudioManager::suspendAudio()
	{
		AudioManager::suspendAudio();
		this->xa2Device->StopEngine();
	}

	void XAudio2_AudioManager::resumeAudio()
	{
		HRESULT result = this->xa2Device->StartEngine();
		if (FAILED(result))
		{
			hlog::error(xal::logTag, "Could not restart engine!");
		}
		AudioManager::resumeAudio();
	}

	Player* XAudio2_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new XAudio2_Player(sound);
	}

}
#endif
