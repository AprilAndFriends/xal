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

#include "SDL_AudioManager.h"

namespace xal
{
	SDL_AudioManager::SDL_AudioManager(chstr deviceName, bool threaded, float updateTime) :
		AudioManager(deviceName, threaded, updateTime)
	{
	}

	SDL_AudioManager::~SDL_AudioManager()
	{
	}
	
}
#endif