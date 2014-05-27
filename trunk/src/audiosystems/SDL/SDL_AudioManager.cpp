/// @file
/// @version 3.14
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _SDL
#include <SDL/SDL.h>

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "SDL_AudioManager.h"
#include "SDL_Player.h"
#include "Source.h"
#include "Buffer.h"
#include "xal.h"

namespace xal
{
	SDL_AudioManager::SDL_AudioManager(void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(backendId, threaded, updateTime, deviceName)
	{
		this->name = XAL_AS_SDL;
		hlog::write(xal::logTag, "Initializing SDL Audio.");
		this->buffer = new unsigned char[1];
		this->bufferSize = 1;
		int result = SDL_InitSubSystem(SDL_INIT_AUDIO);
		if (result != 0)
		{
			hlog::errorf(xal::logTag, "Unable to initialize SDL: %s", SDL_GetError());
			return;
		}
		this->format.freq = this->samplingRate;
		this->format.format = (this->bitsPerSample == 16 ? AUDIO_S16 : AUDIO_S8);
		this->format.channels = this->channels;
		this->format.samples = 2048;
		this->format.callback = &SDL_AudioManager::_mixAudio;
		this->format.userdata = NULL;
		// open audio device
		result = SDL_OpenAudio(&this->format, NULL);
		if (result < 0)
		{
			hlog::errorf(xal::logTag, "Unable to initialize SDL: %s", SDL_GetError());
			return;
		}
		SDL_PauseAudio(0);
		this->enabled = true;
	}

	SDL_AudioManager::~SDL_AudioManager()
	{
		hlog::write(xal::logTag, "Destroying SDL Audio.");
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		delete [] this->buffer;
	}
	
	Player* SDL_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new SDL_Player(sound);
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
	
	int SDL_AudioManager::_convertStream(Buffer* buffer, unsigned char** stream, int *streamSize, int dataSize)
	{	
		if (*stream == NULL || *streamSize <= 0 || dataSize <= 0)
		{
			return dataSize;
		}
		SDL_AudioSpec format = this->getFormat();
		int srcFormat = (buffer->getBitsPerSample() == 16 ? AUDIO_S16 : AUDIO_S8);
		int srcChannels = buffer->getChannels();
		int srcSamplingRate = buffer->getSamplingRate();
		if (srcFormat == format.format && srcChannels == format.channels && srcSamplingRate == format.freq)
		{
			return dataSize;
		}
		SDL_AudioCVT cvt;
		cvt.buf = NULL;
		int result = SDL_BuildAudioCVT(&cvt, srcFormat, srcChannels, srcSamplingRate, format.format, format.channels, format.freq);
		if (result <= 0)
		{
			hlog::error(xal::logTag, "Could not build converter: " + buffer->getFilename());
			return dataSize;
		}
		cvt.buf = (Uint8*)new unsigned char[dataSize * cvt.len_mult];
		cvt.len = dataSize;
		memcpy(cvt.buf, *stream, dataSize * sizeof(unsigned char));
		result = SDL_ConvertAudio(&cvt);
		if (result != 0)
		{
			delete [] cvt.buf;
			cvt.buf = NULL;
			hlog::error(xal::logTag, "Could not convert audio: " + buffer->getFilename());
			return dataSize;
		}
		if (cvt.len_cvt > 0)
		{
			if (cvt.len_cvt > *streamSize)
			{
				delete [] *stream;
				*streamSize = cvt.len_cvt;
				*stream = new unsigned char[*streamSize];
			}
			memcpy(*stream, cvt.buf, cvt.len_cvt * sizeof(unsigned char));
		}
		delete [] cvt.buf;
		cvt.buf = NULL;
		return cvt.len_cvt;
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