/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the Player that does not play any audio.

#ifdef HAVE_XAUDIO2
#ifndef XAL_XAUDIO2_PLAYER_H
#define XAL_XAUDIO2_PLAYER_H

#include "Player.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;

	class xalExport XAudio2_Player : public Player
	{
	public:
		XAudio2_Player(Sound* sound);
		~XAudio2_Player();

	protected:
		bool playing;

		bool _systemIsPlaying();
		void _systemPlay();
		int _systemStop();

	};

}
#endif
#endif
