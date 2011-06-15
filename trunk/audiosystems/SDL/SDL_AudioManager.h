/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for DirectSound.

#if HAVE_SDL
#ifndef XAL_SDL_AUDIO_MANAGER_H
#define XAL_SDL_AUDIO_MANAGER_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Sound;
	class Player;

	class xalExport SDL_AudioManager : public AudioManager
	{
	public:
		SDL_AudioManager(chstr systemName, unsigned long backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~SDL_AudioManager();

	protected:
		Player* _createAudioPlayer(Sound* sound, Buffer* buffer);

		static void _mixAudio(void* unused, unsigned char* stream, int length);

	};

}

#endif
#endif