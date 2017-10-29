/// @file
/// @version 3.6
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
		hlog::write(logTag, "Initializing SDL Audio.");
		int result = SDL_InitSubSystem(SDL_INIT_AUDIO);
		if (result != 0)
		{
			hlog::errorf(logTag, "Unable to initialize SDL: %s", SDL_GetError());
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
			hlog::errorf(logTag, "Unable to initialize SDL: %s", SDL_GetError());
			return;
		}
		SDL_PauseAudio(0);
		this->enabled = true;
	}

	SDL_AudioManager::~SDL_AudioManager()
	{
		hlog::write(logTag, "Destroying SDL Audio.");
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
	
	Player* SDL_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new SDL_Player(sound);
	}

	void SDL_AudioManager::mixAudio(void* unused, unsigned char* stream, int length)
	{
		hmutex::ScopeLock lock(&this->mutex);
		if (this->buffer.size() < length)
		{
			this->buffer.clear(length); // to make sure there is enough space available
		}
		this->buffer.fill(0, length);
		bool first = true;
		harray<SDL_Player*> players = this->players.cast<SDL_Player*>();
		foreach (SDL_Player*, it, players)
		{
			if ((*it)->mixAudio(this->buffer, length, first)) // returns true if playing and first audio data has been mixed into the stream
			{
				first = false;
			}
		}
		// because stream mixing is done manually, there is no need to call SDL_MixAudio and memcpy is enough,
		// the following line is here only for demonstration how it would look like with SDL_MixAudio
		//SDL_MixAudio(stream, this->buffer, this->bufferSize, SDL_MIX_MAXVOLUME);
		memcpy(stream, (unsigned char*)this->buffer, length);
	}

	void SDL_AudioManager::_mixAudio(void* unused, unsigned char* stream, int length)
	{
		((SDL_AudioManager*)xal::manager)->mixAudio(unused, stream, length);
	}
	
	void SDL_AudioManager::_convertStream(Source* source, hstream& stream)
	{	
		if (stream.size() == 0)
		{
			return;
		}
		SDL_AudioSpec format = this->getFormat();
		int srcFormat = (source->getBitsPerSample() == 16 ? AUDIO_S16 : AUDIO_S8);
		int srcChannels = source->getChannels();
		int srcSamplingRate = source->getSamplingRate();
		if (srcFormat == format.format && srcChannels == format.channels && srcSamplingRate == format.freq)
		{
			return;
		}
		SDL_AudioCVT cvt;
		cvt.buf = NULL;
		int result = SDL_BuildAudioCVT(&cvt, srcFormat, srcChannels, srcSamplingRate, format.format, format.channels, format.freq);
		if (result <= 0)
		{
			hlog::error(logTag, "Could not build converter: " + source->getFilename());
			return;
		}
		cvt.buf = (Uint8*)new unsigned char[(int)stream.size() * cvt.len_mult];
		cvt.len = (int)stream.size();
		result = stream.readRaw(cvt.buf, cvt.len);
		if (result > 0)
		{
			stream.seek(-result);
		}
		result = SDL_ConvertAudio(&cvt);
		if (result != 0)
		{
			delete[] cvt.buf;
			cvt.buf = NULL;
			hlog::error(logTag, "Could not convert audio: " + source->getFilename());
			return;
		}
		if (cvt.len_cvt > 0)
		{
			if (cvt.len_cvt > stream.size())
			{
				stream.setCapacity((int)stream.position() + cvt.len_cvt);
			}
			int written = stream.writeRaw(cvt.buf, cvt.len_cvt);
			if (written > 0)
			{
				stream.seek(-written);
			}
		}
		delete[] cvt.buf;
		cvt.buf = NULL;
		return;
	}
	
}
#endif