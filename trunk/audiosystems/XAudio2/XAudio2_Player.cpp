/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_XAUDIO2
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Buffer.h"
#include "XAudio2_Player.h"
#include "XAudio2_AudioManager.h"
#include "Sound.h"
#include "xal.h"

namespace xal
{
	XAudio2_Player::XAudio2_Player(Sound* sound) : Player(sound), playing(false)
	{
	}

	XAudio2_Player::~XAudio2_Player()
	{
	}

	bool XAudio2_Player::_systemIsPlaying()
	{
		return this->playing;
	}

	void XAudio2_Player::_systemPlay()
	{
		this->playing = true;
	}

	int XAudio2_Player::_systemStop()
	{
		this->playing = false;
		return 0;
	}

}
#endif
