/// @file
/// @author  Boris Mikic
/// @version 2.7
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Buffer.h"
#include "NoAudio_Player.h"
#include "NoAudio_AudioManager.h"
#include "Sound.h"
#include "xal.h"

namespace xal
{
	NoAudio_Player::NoAudio_Player(Sound* sound) : Player(sound), playing(false)
	{
	}

	NoAudio_Player::~NoAudio_Player()
	{
	}

	bool NoAudio_Player::_systemIsPlaying()
	{
		return this->playing;
	}

	void NoAudio_Player::_systemPlay()
	{
		this->playing = true;
	}

	int NoAudio_Player::_systemStop()
	{
		this->playing = false;
		return 0;
	}

}
