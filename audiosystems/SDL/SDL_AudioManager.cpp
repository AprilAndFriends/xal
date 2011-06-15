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

#include <hltypes/hstring.h>

#include "SDL_AudioManager.h"
#include "SDL_Player.h"
#include "Source.h"
#include "xal.h"

#define SDL_MAX_PLAYERS 32

namespace xal
{
	SDL_AudioManager::SDL_AudioManager(chstr systemName, unsigned long backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName)
	{
		xal::log("initializing SDL Audio");
		int result = SDL_InitSubSystem(SDL_INIT_AUDIO);
		if (result != 0)
		{
			xal::log(hsprintf("Unable to initialize SDL: %s\n", SDL_GetError()));
			return;
		}
		SDL_AudioSpec fmt;
		fmt.freq = 44100;
		fmt.format = AUDIO_S16;
		fmt.channels = 2;
		fmt.samples = (Uint16)(STREAM_BUFFER_SIZE * STREAM_BUFFER_COUNT);
		fmt.callback = &SDL_AudioManager::_mixAudio;
		fmt.userdata = NULL;
		// open audio device
		result = SDL_OpenAudio(&fmt, NULL);
		if (result < 0)
		{
			xal::log(hsprintf("Unable to initialize SDL: %s\n", SDL_GetError()));
			return;
		}
		this->enabled = true;
		if (threaded)
		{
			this->_setupThread();
		}
	}

	SDL_AudioManager::~SDL_AudioManager()
	{
		xal::log("destroying SDL Audio");
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
	
	Player* SDL_AudioManager::_createAudioPlayer(Sound* sound, Buffer* buffer)
	{
		return new SDL_Player(sound, buffer);
	}

	void SDL_AudioManager::_mixAudio(void* unused, unsigned char* stream, int length)
	{
		//SDL_AudioManager::update();
	}

}
#endif