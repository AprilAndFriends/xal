/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_DIRECTSOUND
#include <dsound.h>
#include <windows.h>

#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "Buffer.h"
#include "DirectSound_Player.h"
#include "DirectSound_AudioManager.h"
#if HAVE_WAV
#include "DirectSound_WAV_Source.h"
#endif
#include "Sound.h"
#include "xal.h"

namespace xal
{
	DirectSound_Player::DirectSound_Player(Sound* sound, Buffer* buffer) :
		Player(sound, buffer), playing(false), dsBuffer(NULL)
	{
	}

	DirectSound_Player::~DirectSound_Player()
	{
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Release();
			this->dsBuffer = NULL;
		}
	}

	void DirectSound_Player::update(float k)
	{
		Player::update(k);
		if (this->playing)
		{
			unsigned long status;
			this->dsBuffer->GetStatus(&status);
			if ((status & DSBSTATUS_PLAYING) == 0)
			{
				this->playing = false;
			}
		}
	}

	void DirectSound_Player::_sysSetOffset(float value)
	{
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->SetCurrentPosition((DWORD)value);
		}
	}

	float DirectSound_Player::_sysGetOffset()
	{
		if (this->dsBuffer == NULL)
		{
			return 0.0f;
		}
		unsigned long position;
		this->dsBuffer->GetCurrentPosition(&position, NULL);
		return (float)position;
	}

	bool DirectSound_Player::_sysPreparePlay()
	{
		WAVEFORMATEX wavefmt;
#if HAVE_WAV
		DirectSound_WAV_Source* wavSource = dynamic_cast<DirectSound_WAV_Source*>(this->buffer->getSource());
		if (wavSource != NULL)
		{
			wavefmt = wavSource->getWavefmt();
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
		HRESULT result = ((DirectSound_AudioManager*)xal::mgr)->dsDevice->CreateSoundBuffer(&bufferDesc, &this->dsBuffer, NULL);
		if (FAILED(result))
		{
			this->dsBuffer = NULL;
			return false;
		}
		return true;
	}

	void DirectSound_Player::_sysPrepareBuffer()
	{
		this->buffer->prepare();
		// filling buffer data
		void* write1 = NULL;
		void* write2 = NULL;
		unsigned long length1;
		unsigned long length2;
		HRESULT result = this->dsBuffer->Lock(0, this->buffer->getSize(), &write1, &length1, &write2, &length2, 0);
		if (FAILED(result))
		{
			xal::log("cannot lock buffer for " + this->sound->getRealFilename());
			return;
		}
		unsigned char* stream = this->buffer->getStream();
		if (write1 != NULL)
		{
			memcpy(write1, stream, length1);
		}
		if (write2 != NULL)
		{
			memcpy(write2, &stream[length1], length2);
		}
		this->dsBuffer->Unlock(write1, length1, write2, length2);
	}

	void DirectSound_Player::_sysUpdateGain()
	{
		if (this->dsBuffer != NULL)
		{
			printf(". %f\n", this->_calcGain());
			this->dsBuffer->SetVolume(DSBVOLUME_MIN + (LONG)((DSBVOLUME_MAX - DSBVOLUME_MIN) * this->_calcGain()));
		}
	}

	void DirectSound_Player::_sysUpdateFadeGain()
	{
		if (this->dsBuffer != NULL)
		{
			float gain = this->_calcFadeGain();
			LONG value = DSBVOLUME_MIN;
			if (gain > 0.0f)
			{
				value = (LONG)(log10(gain) / 4 * (DSBVOLUME_MAX - DSBVOLUME_MIN));
			}
			this->dsBuffer->SetVolume(value);
		}
	}

	void DirectSound_Player::_sysPlay()
	{
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Play(0, 0, (this->looping ? DSBPLAY_LOOPING : 0));
			this->playing = true;
		}
	}

	void DirectSound_Player::_sysStop()
	{
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->Stop();
			this->playing = false;
		}
	}

}
#endif