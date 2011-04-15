/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "Buffer.h"
#include "Player.h"
#include "Sound2.h"

namespace xal
{
	Player::Player(Sound2* sound, Buffer* buffer)
	{
		this->sound = sound;
		this->buffer = buffer;
	}

	Player::~Player()
	{
	}

	void Player::setGain(float gain)
	{
		this->gain = gain;
	}

}
