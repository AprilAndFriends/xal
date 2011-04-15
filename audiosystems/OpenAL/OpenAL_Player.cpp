/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifndef __APPLE__
#include <AL/al.h>
#else
#include <OpenAL/al.h>
#endif

#include "AudioManager.h"
#include "Category.h"
#include "OpenAL_Player.h"
#include "Sound2.h"

namespace xal
{
	OpenAL_Player::OpenAL_Player(Sound2* sound, Buffer* buffer) :
		Player(sound, buffer)
	{
	}

	OpenAL_Player::~OpenAL_Player()
	{
	}

	void OpenAL_Player::setGain(float gain)
	{
		Player::setGain(gain);
		if (this->sourceId != 0)
		{
			alSourcef(this->sourceId, AL_GAIN, this->gain *
				this->sound->getCategory()->getGain() * xal::mgr->getGlobalGain());
		}
	}

}
