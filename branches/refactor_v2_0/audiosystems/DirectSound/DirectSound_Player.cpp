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

#include "Buffer.h"
#include "DirectSound_Player.h"
#include "DirectSound_AudioManager.h"
#include "Sound.h"
#include "xal.h"

namespace xal
{
	DirectSound_Player::DirectSound_Player(Sound* sound, Buffer* buffer) :
		Player(sound, buffer), playing(false)
	{
		this->dsBuffer = NULL;
		if (!xal::mgr->isEnabled())
		{
			return;
		}
		hstr realFilename = this->sound->getRealFilename();
		wchar_t* filename = realFilename.w_str();

		//hstr2wstr(
		//char* filename = (char*)this->sound->getRealFilename().c_str();
		HMMIO wavefile = mmioOpen(filename, 0, MMIO_READ | MMIO_ALLOCBUF);

		if (wavefile == NULL)
		{
			return;
		}
		MMCKINFO parent;
		memset (&parent, 0, sizeof(MMCKINFO));
		parent.fccType = mmioFOURCC ('W', 'A', 'V', 'E');
		mmioDescend (wavefile, &parent, 0, MMIO_FINDRIFF);
		MMCKINFO child;
		memset (&child, 0, sizeof(MMCKINFO));
		child.fccType = mmioFOURCC ('f', 'm', 't', ' ');
		mmioDescend (wavefile, &child, &parent, 0);
		WAVEFORMATEX wavefmt;
		mmioRead (wavefile, (char*)&wavefmt, sizeof(wavefmt));

		if (wavefmt.wFormatTag != WAVE_FORMAT_PCM)
		{
			return;
		}
		mmioAscend (wavefile, &child, 0);
		child.ckid = mmioFOURCC ('d', 'a', 't', 'a');
		mmioDescend (wavefile, &child, &parent, MMIO_FINDCHUNK);

		// creating a dsBuffer
		DSBUFFERDESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(DSBUFFERDESC));
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		bufferDesc.dwFlags = (DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS);
		bufferDesc.dwBufferBytes = child.cksize;
		bufferDesc.lpwfxFormat = &wavefmt;

		HRESULT result = ((DirectSound_AudioManager*)xal::mgr)->dsDevice->CreateSoundBuffer(&bufferDesc, &this->dsBuffer, NULL);
		if (result != DS_OK)
		{
			this->dsBuffer = NULL;
			return;
		}

		// filling buffer data
		void* write1 = 0;
		void* write2 = 0;
		unsigned long length1;
		unsigned long length2;
		this->dsBuffer->Lock(0, child.cksize, &write1, &length1, &write2, &length2, 0);
		if (write1 > 0)
			mmioRead(wavefile, (char*)write1, length1);
		if (write2 > 0)
			mmioRead(wavefile, (char*)write2, length2);
		this->dsBuffer->Unlock(write1, length1, write2, length2);
		mmioClose(wavefile, 0);
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

	void DirectSound_Player::setGain(float value)
	{
		Player::setGain(value);
		if (this->dsBuffer != NULL)
		{
			this->dsBuffer->SetVolume(DSBVOLUME_MIN + (LONG)((DSBVOLUME_MAX - DSBVOLUME_MIN) * this->_calcGain()));
		}
	}

	void DirectSound_Player::_sysSetOffset(float value)
	{
		this->dsBuffer->SetCurrentPosition((DWORD)value);
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
		return (this->dsBuffer != NULL);
	}

	void DirectSound_Player::_sysPrepareBuffer(int channels, int rate, unsigned char* stream, int size)
	{
	}

	void DirectSound_Player::_sysUpdateFadeGain()
	{
		this->dsBuffer->SetVolume(DSBVOLUME_MIN + (LONG)((DSBVOLUME_MAX - DSBVOLUME_MIN) * this->_calcFadeGain()));
	}

	void DirectSound_Player::_sysPlay()
	{
		if (this->dsBuffer != NULL)
		{
			HRESULT result = this->dsBuffer->Play(0, 0, (this->looping ? DSBPLAY_LOOPING : 0));
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