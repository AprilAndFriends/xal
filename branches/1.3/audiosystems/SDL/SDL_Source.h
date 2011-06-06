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

#if HAVE_SDL
#ifndef XAL_SDL_SOURCE_H
#define XAL_SDL_SOURCE_H
#include <hltypes/hstring.h>

#include "Source.h"
#include "xalExport.h"

struct Mix_Chunk;

namespace xal
{
	class xalExport SDL_Source : public Source
	{
	public:
		SDL_Source(chstr filename);
		~SDL_Source();

		Mix_Chunk* getMixChunk() { return this->mixChunk; }

		bool load(unsigned char* output);
		int loadChunk(unsigned char* output, int size);

	protected:
		Mix_Chunk* mixChunk;

	};

}

#endif
#endif
