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
/// Represents an implementation of the AudioManager that does not play any audio.

#ifdef HAVE_XAUDIO2
#ifndef XAL_XAUDIO2_AUDIO_MANAGER_H
#define XAL_XAUDIO2_AUDIO_MANAGER_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;
	class Player;

	class xalExport XAudio2_AudioManager : public AudioManager
	{
	public:
		XAudio2_AudioManager(chstr systemName, void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~XAudio2_AudioManager();

	protected:
		Player* _createSystemPlayer(Sound* sound);

	};

}

#endif
#endif
