/// @file
/// @author  Boris Mikic
/// @version 2.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_SDL
#include <SDL/SDL.h>

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "SDL_AudioManager.h"
#include "SDL_Player.h"
#include "Source.h"
#include "Buffer.h"
#include "xal.h"

namespace xal
{
	SDL_AudioManager::SDL_AudioManager(chstr systemName, void* backendId, bool threaded, float updateTime, chstr deviceName) :
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
	}

	SDL_AudioManager::~SDL_AudioManager()
	{
		xal::log("destroying SDL Audio");
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		delete [] this->buffer;
	}
	
	Player* SDL_AudioManager::_createSystemPlayer(Sound* sound, Buffer* buffer)
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
		harray<SDL_Player*> players = this->players.cast<SDL_Player*>();
		foreach (SDL_Player*, it, players)
		{
			if ((*it)->mixAudio(this->buffer, this->bufferSize, first)) // returns true if playing and first audio data has been mixed into the stream
			{
				first = false;
			}
		}
		// because stream mixing is done manually, there is no need to call SDL_MixAudio and memcpy is enough,
		// the following line is here only for demonstration how it would look like with SDL_MixAudio
		//SDL_MixAudio(stream, this->buffer, this->bufferSize, SDL_MIX_MAXVOLUME);
		memcpy(stream, this->buffer, this->bufferSize);
		this->_unlock();
	}

	void SDL_AudioManager::_mixAudio(void* unused, unsigned char* stream, int length)
	{
		((SDL_AudioManager*)xal::mgr)->mixAudio(unused, stream, length);
	}
	
	void SDL_AudioManager::_convertStream(Buffer* buffer, unsigned char** stream, int *streamSize)
	{	
		SDL_AudioSpec format = this->getFormat();
		int srcFormat = (buffer->getBitsPerSample() == 16 ? AUDIO_S16 : AUDIO_S8);
		int srcChannels = buffer->getChannels();
		int srcSamplingRate = buffer->getSamplingRate();
		if (srcFormat != format.format || srcChannels != format.channels || srcSamplingRate != format.freq)
		{
			SDL_AudioCVT cvt;
			int result = SDL_BuildAudioCVT(&cvt, srcFormat, srcChannels, srcSamplingRate, format.format, format.channels, format.freq);
			if (result == -1)
			{
				xal::log("ERROR: Could not build converter " + buffer->getFilename());
				return;
			}
			cvt.buf = (Uint8*)(new unsigned char[*streamSize * cvt.len_mult]); // making sure the conversion buffer is large enough
			memcpy(cvt.buf, *stream, *streamSize * sizeof(unsigned char));
			cvt.len = *streamSize;
			result = SDL_ConvertAudio(&cvt);
			if (result == -1)
			{
				xal::log("ERROR: Could not convert audio " + buffer->getFilename());
				return;
			}
			int newSize = hround(cvt.len * cvt.len_ratio);
			if (*streamSize != newSize) // stream has to be resized
			{
				*streamSize = newSize;
				delete *stream;
				*stream = new unsigned char[*streamSize];
			}
			memcpy(*stream, cvt.buf, *streamSize * sizeof(unsigned char));
			delete [] cvt.buf;
		}
	}
	
	// SDL requires software mixing so the mutex locking has to be done even when there is no threaded update
	void SDL_AudioManager::_lock()
	{
		this->mutex.lock();
	}
	
	void SDL_AudioManager::_unlock()
	{
		this->mutex.unlock();
	}

}
#endif