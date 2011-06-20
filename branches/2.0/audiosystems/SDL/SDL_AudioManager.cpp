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

#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "SDL_AudioManager.h"
#include "SDL_Player.h"
#include "Source.h"
#include "xal.h"

namespace xal
{
	SDL_AudioManager::SDL_AudioManager(chstr systemName, unsigned long backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName)
	{
		xal::log("initializing SDL Audio");
		this->buffer = new unsigned char[1];
		this->bufferSize = 1;
		int result = SDL_InitSubSystem(SDL_INIT_AUDIO);
		if (result != 0)
		{
			xal::log(hsprintf("Unable to initialize SDL: %s\n", SDL_GetError()));
			return;
		}

		this->format.freq = 44100;
		this->format.format = AUDIO_S16;
		this->format.channels = 2;
		this->format.samples = 2048;
		this->format.callback = &SDL_AudioManager::_mixAudio;
		this->format.userdata = NULL;
		// open audio device
		result = SDL_OpenAudio(&this->format, NULL);
		if (result < 0)
		{
			xal::log(hsprintf("Unable to initialize SDL: %s\n", SDL_GetError()));
			return;
		}
		SDL_PauseAudio(0);
		this->enabled = true;
		if (threaded)
		{
			this->_setupThread();
		}
	}

	SDL_AudioManager::~SDL_AudioManager()
	{
		xal::log("destroying SDL Audio");
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		delete [] this->buffer;
	}
	
	Player* SDL_AudioManager::_createAudioPlayer(Sound* sound, Buffer* buffer)
	{
		return new SDL_Player(sound, buffer);
	}

	void SDL_AudioManager::mixAudio(void* unused, unsigned char* stream, int length)
	{
		this->_lock();
		if (this->bufferSize != length)
		{
			delete [] this->buffer;
			this->buffer = new unsigned char[length];
			this->bufferSize = length;
		}
		memset(this->buffer, 0, this->bufferSize * sizeof(unsigned char));
		bool first = true;
		foreach (Player*, it, this->players)
		{
			((SDL_Player*)(*it))->mixAudio(this->buffer, this->bufferSize, first);
			first = false;
		}
		
		// SDL docs say we don't need to use SDL_MixAudio if we
		// use only one audio channel.
		// 
		/*
		SDL_MixAudio(stream, this->buffer, this->bufferSize, SDL_MIX_MAXVOLUME);
		 */
		memcpy(stream, this->buffer, this->bufferSize);
		
		this->_unlock();
	}

	void SDL_AudioManager::_mixAudio(void* unused, unsigned char* stream, int length)
	{
		((SDL_AudioManager*)xal::mgr)->mixAudio(unused, stream, length);
	}

}
#endif