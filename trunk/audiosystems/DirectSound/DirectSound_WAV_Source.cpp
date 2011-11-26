/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/*
#if HAVE_DIRECTSOUND && HAVE_WAV
#include <windows.h>

#include <hltypes/hfile.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "DirectSound_WAV_Source.h"
#include "xal.h"

namespace xal
{
	DirectSound_WAV_Source::DirectSound_WAV_Source(chstr filename) : Source(filename), file(NULL)
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
			wchar_t* filenameW = this->filename.w_str();
			this->file = mmioOpen(filenameW, 0, (MMIO_READ | MMIO_ALLOCBUF));
			delete [] filenameW;
			if (this->file == NULL)
			{
				this->close();
				return false;
			}
			// getting all the info
			MMCKINFO parent;
			memset(&parent, 0, sizeof(MMCKINFO));
			parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
			mmioDescend(this->file, &parent, 0, MMIO_FINDRIFF);
			MMCKINFO child;
			memset(&child, 0, sizeof(MMCKINFO));
			child.fccType = mmioFOURCC('f', 'm', 't', ' ');
			mmioDescend(this->file, &child, &parent, 0);
			mmioRead(this->file, (char*)&this->wavefmt, sizeof(wavefmt));
			if (wavefmt.wFormatTag != WAVE_FORMAT_PCM)
			{
				this->close();
				return false;
			}
			this->channels = wavefmt.nChannels;
			this->samplingRate = wavefmt.nSamplesPerSec;
			this->bitsPerSample = wavefmt.wBitsPerSample;
			mmioAscend(file, &child, 0);
			// find audio data
			child.ckid = mmioFOURCC('d', 'a', 't', 'a');
			mmioDescend(file, &child, &parent, MMIO_FINDCHUNK);
			this->size = child.cksize;
		}
		return result;
	}

	void DirectSound_WAV_Source::close()
	{
		if (this->streamOpen)
		{
			mmioClose(this->file, 0);
		}
	}

	void DirectSound_WAV_Source::rewind()
	{
		if (this->streamOpen)
		{
			// getting all the info
			mmioSeek(this->file, 0, SEEK_SET);
			MMCKINFO parent;
			memset(&parent, 0, sizeof(MMCKINFO));
			parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
			mmioDescend(this->file, &parent, 0, MMIO_FINDRIFF);
			MMCKINFO child;
			memset(&child, 0, sizeof(MMCKINFO));
			child.ckid = mmioFOURCC('d', 'a', 't', 'a');
			mmioDescend(file, &child, &parent, MMIO_FINDCHUNK);
		}
	}

	bool DirectSound_WAV_Source::load(unsigned char* output)
	{
		if (!Source::load(output))
		{
			return false;
		}
		mmioRead(this->file, (char*)output, this->size);
		return true;
	}

	int DirectSound_WAV_Source::loadChunk(unsigned char* output, int size)
	{
		if (!Source::loadChunk(output, size))
		{
			return false;
		}
		return (int)mmioRead(this->file, (char*)output, size);
	}

}
#endif
*/