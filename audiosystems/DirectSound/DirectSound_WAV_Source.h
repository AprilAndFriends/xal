/// @file
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Provides a source for WAV format when using DirectSound.
/*
#if defined(_DIRECTSOUND) && defined(_FORMAT_WAV)
#ifndef XAL_DIRECTSOUND_WAV_SOURCE_H
#define XAL_DIRECTSOUND_WAV_SOURCE_H
#include <dsound.h>

#include <hltypes/hstring.h>
#include <hltypes/hplatform.h>

#include "AudioManager.h"
#include "Source.h"
#include "xalExport.h"

namespace xal
{
	class xalExport DirectSound_WAV_Source : public Source
	{
	public:
		DirectSound_WAV_Source(chstr filename, Source::Mode mode);
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