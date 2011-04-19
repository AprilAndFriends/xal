/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_SDL
#include "Buffer.h"
#include "SDL_Player.h"
#include "Sound.h"

namespace xal
{
	SDL_Player::SDL_Player(Sound* sound, Buffer* buffer) :
		Player(sound, buffer)
	{
	}

	SDL_Player::~SDL_Player()
	{
	}

}
#endif