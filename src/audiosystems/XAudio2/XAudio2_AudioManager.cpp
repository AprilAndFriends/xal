/// @file
/// @version 4.0
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
		AudioManager(backendId, threaded, updateTime, deviceName), xa2Device(NULL), xa2MasteringVoice(NULL), stopped(false)
	{
		this->name = AudioSystemType::XAudio2.getName();
		hlog::write(logTag, "Initializing " + this->name + ".");
		HRESULT result = XAudio2Create(&this->xa2Device, 0);
		if (FAILED(result))
		{
			this->xa2Device = NULL;
			hlog::error(logTag, "Could not create device!");
			return;
		}
		result = this->xa2Device->CreateMasteringVoice(&this->xa2MasteringVoice, 2, 44100);
		if (FAILED(result)) // if can't use 44.1 kHz stereo, use default
		{
			hlog::write(logTag, "Could not create device with 2 channels and 44100 Hz, attempting defaults...");
			result = this->xa2Device->CreateMasteringVoice(&this->xa2MasteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE);
		}
		if (FAILED(result))
		{
			this->xa2Device->Release();
			this->xa2Device = NULL;
			hlog::error(logTag, "Could not create mastering voice!");
			return;
		}
		result = this->xa2Device->StartEngine();
		if (FAILED(result))
		{
			this->xa2MasteringVoice->DestroyVoice();
			this->xa2MasteringVoice = NULL;
			this->xa2Device->Release();
			this->xa2Device = NULL;
			hlog::error(logTag, "Could not start engine!");
			return;
		}
		this->enabled = true;
	}

	XAudio2_AudioManager::~XAudio2_AudioManager()
	{
		hlog::write(logTag, "Destroying " + this->name + ".");
		this->xa2Device->StopEngine();
		if (this->xa2MasteringVoice != NULL)
		{
			this->xa2MasteringVoice->DestroyVoice();
			this->xa2MasteringVoice = NULL;
		}
		_HL_TRY_RELEASE(this->xa2Device);
	}

	void XAudio2_AudioManager::_update(float timeDelta)
	{
		AudioManager::_update(timeDelta);
		if (this->suspended && !this->stopped && !this->_isAnyFading())
		{
			hlog::write(logTag, "Stopping engine.");
			this->xa2Device->StopEngine();
			this->stopped = true;
		}
	}
	
	void XAudio2_AudioManager::_suspendSystem()
	{
		AudioManager::_suspendSystem();
		if (!this->stopped && !this->_isAnyFading())
		{
			hlog::write(logTag, "Stopping engine.");
			this->xa2Device->StopEngine();
			this->stopped = true;
		}
	}

	void XAudio2_AudioManager::_resumeSystem()
	{
		if (this->stopped)
		{
			hlog::write(logTag, "Starting engine.");
			HRESULT result = this->xa2Device->StartEngine();
			if (FAILED(result))
			{
				hlog::error(logTag, "Could not restart engine!");
			}
			this->stopped = false;
		}
		AudioManager::_resumeSystem();
	}

	Player* XAudio2_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new XAudio2_Player(sound);
	}

}
#endif
