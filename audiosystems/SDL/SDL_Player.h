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
		SDL_Player(Sound* sound, Buffer* buffer);
		~SDL_Player();

		bool isPlaying();
		void setGain(float value);

	protected:
		int channelId;
		//IDirectSoundBuffer* dsBuffer;
		/// @brief Playing flag.
		/// @note Due to possible latency in DirectSound, this flag is being used to avoid problems.
		//bool playing;

		float _sysGetOffset();
		void _sysSetOffset(float value);
		bool _sysPreparePlay();
		void _sysPrepareBuffer();
		void _sysUpdateFadeGain();
		void _sysPlay();
		void _sysStop();

	};

}
#endif
#endif