/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_DIRECTSOUND && HAVE_WAV
#include <dsound.h>
#include <windows.h>

#include <hltypes/hfile.h>
#include <hltypes/hstring.h>

#include "DirectSound_AudioManager.h"
#include "DirectSound_WAV_Source.h"
#include "xal.h"

namespace xal
{
	DirectSound_WAV_Source::DirectSound_WAV_Source(chstr filename) : Source(filename)
	{
	}

	DirectSound_WAV_Source::~DirectSound_WAV_Source()
	{
	}

	bool DirectSound_WAV_Source::load(unsigned char** output)
	{
		xal::log("loading wav sound " + this->filename);
		if (!hfile::exists(this->filename))
		{
			xal::log("file not found " + this->filename);
			return false;
		}
		wchar_t* filenameW = this->filename.w_str();
		HMMIO file = mmioOpen(filenameW, 0, MMIO_READ | MMIO_ALLOCBUF);
		delete [] filenameW;
		if (file == NULL)
		{
			return false;
		}
		MMCKINFO parent;
		memset(&parent, 0, sizeof(MMCKINFO));
		parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		mmioDescend(file, &parent, 0, MMIO_FINDRIFF);
		MMCKINFO child;
		memset(&child, 0, sizeof(MMCKINFO));
		child.fccType = mmioFOURCC('f', 'm', 't', ' ');
		mmioDescend(file, &child, &parent, 0);
		mmioRead(file, (char*)&this->wavefmt, sizeof(wavefmt));
		if (wavefmt.wFormatTag != WAVE_FORMAT_PCM)
		{
			return false;
		}
		this->channels = wavefmt.nChannels;
		this->size = wavefmt.cbSize;
		this->samplingRate = wavefmt.nSamplesPerSec;
		this->bitsPerSample = wavefmt.wBitsPerSample;
		mmioAscend(file, &child, 0);
		child.ckid = mmioFOURCC('d', 'a', 't', 'a');
		mmioDescend(file, &child, &parent, MMIO_FINDCHUNK);

		*output = new unsigned char[this->size];
		mmioRead(file, (char*)(*output), this->size);
		mmioClose(file, 0);
		return true;
		/*





		// creating a dsBuffer
		DSBUFFERDESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(DSBUFFERDESC));
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		bufferDesc.dwFlags = (DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS);
		bufferDesc.dwBufferBytes = child.cksize;
		bufferDesc.lpwfxFormat = &wavefmt;

		HRESULT result = ((DirectSound_AudioManager*)xal::mgr)->dsDevice->CreateSoundBuffer(&bufferDesc, &this->dsBuffer, NULL);
		if (FAILED(result))
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
		return true;
		*/
	}

}
#endif
