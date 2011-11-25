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
/// Represents an implementation of the AudioManager for SDL.

#if HAVE_SDL
#ifndef XAL_SDL_AUDIO_MANAGER_H
#define XAL_SDL_AUDIO_MANAGER_H

#include <SDL/SDL.h>

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

#define SDL_MAX_PLAYING 32

namespace xal
{
	class Buffer;
	class SDL_Player;
	class Sound;
	class Player;

	class xalExport SDL_AudioManager : public AudioManager
	{
	public:
		friend class SDL_Player;

		SDL_AudioManager(chstr systemName, unsigned long backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~SDL_AudioManager();

		SDL_AudioSpec getFormat() { return this->format; }

		void mixAudio(void* unused, unsigned char* stream, int length);

	protected:
		SDL_AudioSpec format;
		unsigned char* buffer;
		int bufferSize;

		Player* _createPlayer(Sound* sound, Buffer* buffer);

		static void _mixAudio(void* unused, unsigned char* stream, int length);

		void _convertStream(Buffer* buffer, unsigned char** stream, int *streamSize);

	};

}

#endif
#endif