/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Provides a source for WAV format when using DirectSound.
/*
#if HAVE_DIRECTSOUND && HAVE_WAV
#ifndef XAL_DIRECTSOUND_WAV_SOURCE_H
#define XAL_DIRECTSOUND_WAV_SOURCE_H
#include <dsound.h>
#include <windows.h>

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Source.h"
#include "xalExport.h"

namespace xal
{
	class xalExport DirectSound_WAV_Source : public Source
	{
	public:
		DirectSound_WAV_Source(chstr filename);
		~DirectSound_WAV_Source();

		WAVEFORMATEX getWavefmt() { return this->wavefmt; }

		bool open();
		void close();
		void rewind();
		bool load(unsigned char* output);
		int loadChunk(unsigned char* output, int size = STREAM_BUFFER_SIZE);

	protected:
		WAVEFORMATEX wavefmt;
		HMMIO file;
		
	};

}

#endif
#endif
*/