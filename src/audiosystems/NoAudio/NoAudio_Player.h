/// @file
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the Player that does not play any audio.

#ifndef XAL_NOAUDIO_PLAYER_H
#define XAL_NOAUDIO_PLAYER_H

#include "Player.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;

	class xalExport NoAudio_Player : public Player
	{
	public:
		NoAudio_Player(Sound* sound);
		~NoAudio_Player();

	protected:
		bool playing;

		bool _systemIsPlaying();
		void _systemPlay();
		int _systemStop();

	};

}
#endif
