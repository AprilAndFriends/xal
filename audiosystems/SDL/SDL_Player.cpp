/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_SDL
#include <SDL_mixer/SDL_mixer.h>

#include "Buffer.h"
#include "SDL_Player.h"
#include "SDL_Source.h"
#include "Sound.h"

namespace xal
{
	SDL_Player::SDL_Player(Sound* sound, Buffer* buffer) : Player(sound, buffer), channelId(-1)
	{
	}

	SDL_Player::~SDL_Player()
	{
	}

	bool SDL_Player::isPlaying()
	{
		return (this->channelId != -1 && Mix_Playing(this->channelId) != 0);
	}

	void SDL_Player::setGain(float value)
	{
		Player::setGain(value);
	}

	void SDL_Player::_sysSetOffset(float value)
	{
		/*
		this->dsBuffer->SetCurrentPosition((DWORD)value);
		*/
	}

	float SDL_Player::_sysGetOffset()
	{
		/*
		if (this->dsBuffer == NULL)
		{
			return 0.0f;
		}
		unsigned long position;
		this->dsBuffer->GetCurrentPosition(&position, NULL);
		return (float)position;
		*/
		return 0.0f;
	}

	bool SDL_Player::_sysPreparePlay()
	{
		/*
		WAVEFORMATEX wavefmt;
#if HAVE_WAV
		SDL_Source* source = dynamic_cast<SDL_Source*>(this->buffer->getSource());
		if (source != NULL)
		{
			wavefmt = source->getWavefmt();
		}
		else
#endif
		{
			wavefmt.cbSize = 0;
			wavefmt.nChannels = this->buffer->getChannels();
			wavefmt.nSamplesPerSec = this->buffer->getSamplingRate();
			wavefmt.wBitsPerSample = this->buffer->getBitsPerSample();
			wavefmt.wFormatTag = WAVE_FORMAT_PCM;
			wavefmt.nBlockAlign = wavefmt.nChannels * wavefmt.wBitsPerSample / 8; // standard calculation of WAV PCM data
			wavefmt.nAvgBytesPerSec = wavefmt.nSamplesPerSec * wavefmt.nBlockAlign; // standard calculation of WAV PCM data
		}
		DSBUFFERDESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(DSBUFFERDESC));
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		bufferDesc.dwFlags = (DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS);
		bufferDesc.dwBufferBytes = this->buffer->getSize();
		bufferDesc.lpwfxFormat = &wavefmt;
		HRESULT result = ((SDL_AudioManager*)xal::mgr)->dsDevice->CreateSoundBuffer(&bufferDesc, &this->dsBuffer, NULL);
		if (FAILED(result))
		{
			this->dsBuffer = NULL;
			return false;
		}
		*/
		
		return true;
	}

	void SDL_Player::_sysPrepareBuffer()
	{
		/*
		// filling buffer data
		void* write1 = NULL;
		void* write2 = NULL;
		unsigned long length1;
		unsigned long length2;
		HRESULT result = this->dsBuffer->Lock(0, size, &write1, &length1, &write2, &length2, 0);
		if (FAILED(result))
		{
			xal::log("cannot lock buffer for " + this->sound->getRealFilename());
			return;
		}
		if (write1 != NULL)
		{
			memcpy(write1, stream, length1);
		}
		if (write2 != NULL)
		{
			memcpy(write2, &stream[length1], length2);
		}
		this->dsBuffer->Unlock(write1, length1, write2, length2);
		*/
	}

	void SDL_Player::_sysUpdateFadeGain()
	{
		/*
		this->dsBuffer->SetVolume(DSBVOLUME_MIN + (LONG)((DSBVOLUME_MAX - DSBVOLUME_MIN) * this->_calcFadeGain()));
		*/
	}

	void SDL_Player::_sysPlay()
	{
		if (this->channelId < 0)
		{
			SDL_Source* source = dynamic_cast<SDL_Source*>(this->buffer->getSource());
			this->channelId = Mix_PlayChannel(-1, source->getMixChunk(), (this->looping ? -1 : 0));
		}
		/*
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Play(0, 0, (this->looping ? DSBPLAY_LOOPING : 0));
			this->playing = true;
		}
		*/
	}

	void SDL_Player::_sysStop()
	{
		/*
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Stop();
			this->playing = false;
		}
		*/
	}

}
#endif