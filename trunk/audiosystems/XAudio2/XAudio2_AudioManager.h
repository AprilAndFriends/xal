/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for XAudio2.

#ifdef HAVE_XAUDIO2
#ifndef XAL_XAUDIO2_AUDIO_MANAGER_H
#define XAL_XAUDIO2_AUDIO_MANAGER_H

#include <xaudio2.h>

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

using namespace Microsoft::WRL;

namespace xal
{
	class Buffer;
	class Sound;
	class Player;

	class xalExport XAudio2_AudioManager : public AudioManager
	{
	public:
        IXAudio2* xa2Device;
        IXAudio2MasteringVoice* xa2MasteringVoice;

		XAudio2_AudioManager(chstr systemName, void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~XAudio2_AudioManager();

		void suspendAudio();
		void resumeAudio();

	protected:
		Player* _createSystemPlayer(Sound* sound);

	};

}

#endif
#endif
