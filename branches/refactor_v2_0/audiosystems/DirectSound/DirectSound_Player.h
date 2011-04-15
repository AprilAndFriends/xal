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
/// Represents an implementation of the Player for DirectSound.

#if HAVE_DIRECTSOUND
#ifndef XAL_DIRECTSOUND_PLAYER_H
#define XAL_DIRECTSOUND_PLAYER_H

#include "Player.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound2;

	class xalExport DirectSound_Player : public Player
	{
	public:
		DirectSound_Player(Sound2* sound, Buffer* buffer, unsigned int sourceId);
		~DirectSound_Player();

	};

}
#endif
#endif