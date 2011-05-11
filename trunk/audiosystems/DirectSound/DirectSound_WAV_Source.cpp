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

	bool DirectSound_WAV_Source::open()
	{
		bool result = Source::open();
		if (result)
		{
		}
		return result;
	}

	bool DirectSound_WAV_Source::close()
	{
		bool result = Source::open();
		if (result)
		{
		}
		return result;
	}

	bool DirectSound_WAV_Source::rewind()
	{
		bool result = Source::open();
		if (result)
		{
		}
		return result;
	}

	bool DirectSound_WAV_Source::load(unsigned char* output)
	{
		if (!Source::load(output))
		{
			return false;
		}
		wchar_t* filenameW = this->filename.w_str();
		HMMIO file = mmioOpen(filenameW, 0, (MMIO_READ | MMIO_ALLOCBUF));
		delete [] filenameW;
		if (file == NULL)
		{
			return false;
		}
		// getting all the info
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
		this->samplingRate = wavefmt.nSamplesPerSec;
		this->bitsPerSample = wavefmt.wBitsPerSample;
		// reading audio data
		mmioAscend(file, &child, 0);
		child.ckid = mmioFOURCC('d', 'a', 't', 'a');
		mmioDescend(file, &child, &parent, MMIO_FINDCHUNK);
		this->size = child.cksize;
		mmioRead(file, (char*)output, this->size);
		mmioClose(file, 0);
		return true;
	}

}
#endif
