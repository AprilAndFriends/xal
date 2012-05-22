/// @file
/// @author  Boris Mikic
/// @version 2.7
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager that does not play any audio.

#ifndef XAL_NOAUDIO_AUDIO_MANAGER_H
#define XAL_NOAUDIO_AUDIO_MANAGER_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;
	class Player;

	class xalExport NoAudio_AudioManager : public AudioManager
	{
	public:
		NoAudio_AudioManager(chstr systemName, void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~NoAudio_AudioManager();

	protected:
		Player* _createSystemPlayer(Sound* sound);

	};

}

#endif
