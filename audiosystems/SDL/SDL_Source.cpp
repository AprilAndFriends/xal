/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_SDL
#include <hltypes/hstring.h>

#include "SDL_Source.h"
#include "xal.h"

namespace xal
{
	SDL_Source::SDL_Source(chstr filename) : Source(filename), mixChunk(NULL)
	{
	}

	SDL_Source::~SDL_Source()
	{
		if (this->mixChunk != NULL)
		{
			//Mix_FreeChunk(this->mixChunk);
			this->mixChunk = NULL;
		}
	}

	bool SDL_Source::load(unsigned char* output)
	{
		if (!Source::load(output))
		{
			return false;
		}
		//this->mixChunk = Mix_LoadWAV(this->filename.c_str());
		if (this->mixChunk == NULL)
		{
			//xal::log(Mix_GetError());
			return false;
		}
		return true;
	}

	int SDL_Source::loadChunk(unsigned char* output, int count)
	{
		if (!Source::loadChunk(output, count) == 0)
		{
			return 0;
		}
		//this->mixChunk = Mix_LoadWAV(this->filename.c_str());
		if (this->mixChunk == NULL)
		{
			//xal::log(Mix_GetError());
			return false;
		}
		return 1;
	}

}
#endif
