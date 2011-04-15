/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_DIRECTSOUND
#include "Buffer.h"
#include "DirectSound_Player.h"
#include "Sound2.h"

namespace xal
{
	DirectSound_Player::DirectSound_Player(Sound2* sound, Buffer* buffer) :
		Player(sound, buffer)
	{
	}

	DirectSound_Player::~DirectSound_Player()
	{
	}

}
#endif