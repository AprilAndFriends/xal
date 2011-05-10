/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_SDL
#include <SDL/SDL.h>
#include <SDL_mixer/SDL_mixer.h>

#include <hltypes/hstring.h>

#include "SDL_AudioManager.h"
#include "SDL_Player.h"
#include "SDL_Source.h"
#include "Source.h"
#include "xal.h"

namespace xal
{
	SDL_AudioManager::SDL_AudioManager(chstr systemName, unsigned long backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, threaded, threaded, updateTime, deviceName)
	{
		xal::log("initializing SDL Audio");
		int result = SDL_InitSubSystem(SDL_INIT_AUDIO);
		if (result != 0)
		{
			xal::log(hsprintf("Unable to initialize SDL: %s\n", SDL_GetError()));
			return;
		}
		result = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 16384); // 44.1 kHz, 16 bit stereo, 2 channels (stereo), stream chunks of 16kB
		if (result != 0)
		{
			xal::log(hsprintf("Unable to initialize audio: %s\n", Mix_GetError()));
			return;
		}
		this->enabled = true;
	}

	SDL_AudioManager::~SDL_AudioManager()
	{
		xal::log("destroying SDL Audio");
		Mix_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
	
	Player* SDL_AudioManager::_createAudioPlayer(Sound* sound, Buffer* buffer)
	{
		return new SDL_Player(sound, buffer);
	}

	Source* SDL_AudioManager::_createSource(chstr filename, Format format)
	{
		Source* source;
		switch (format)
		{
		default:
			source = new SDL_Source(filename);
			break;
		}
		return source;
	}
}
#endif