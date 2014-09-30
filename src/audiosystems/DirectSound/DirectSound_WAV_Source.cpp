/// @file
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/*
#if defined(_DIRECTSOUND) && defined(_FORMAT_WAV)
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "DirectSound_WAV_Source.h"
#include "xal.h"

namespace xal
{
	DirectSound_WAV_Source::DirectSound_WAV_Source(chstr filename, Source::Mode mode) :
		Source(filename, mode), file(NULL)
	{
	}

	DirectSound_WAV_Source::~DirectSound_WAV_Source()
	{
		this->close();
	}

	bool DirectSound_WAV_Source::open()
	{
		bool result = Source::open();
		if (result)
		{
			this->file = mmioOpen(this->filename.w_str().c_str(), 0, (MMIO_READ | MMIO_ALLOCBUF));
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