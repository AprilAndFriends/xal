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
/// Represents an implementation of the Player for SDL.

#if HAVE_SDL
#ifndef XAL_SDL_PLAYER_H
#define XAL_SDL_PLAYER_H

#include "Player.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;

	class xalExport SDL_Player : public Player
	{
	public:
		SDL_Player(Sound* sound, Buffer* buffer, unsigned int sourceId);
		~SDL_Player();

	};

}
#endif
#endif