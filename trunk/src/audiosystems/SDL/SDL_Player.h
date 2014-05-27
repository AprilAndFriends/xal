/// @file
/// @version 3.11
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the Player for SDL.

#ifdef _SDL
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
		SDL_Player(Sound* sound);
		~SDL_Player();

		bool mixAudio(unsigned char* stream, int length, bool first);

	protected:
		bool playing;
		int position;
		float currentGain;
		unsigned char circleBuffer[STREAM_BUFFER];
		int readPosition;
		int writePosition;

		void _update(float timeDelta);

		inline bool _systemIsPlaying() { return this->playing; }
		unsigned int _systemGetBufferPosition();
		float _systemGetOffset();
		void _systemSetOffset(float value);
		bool _systemPreparePlay();
		void _systemPrepareBuffer();
		void _systemUpdateGain();
		void _systemPlay();
		int _systemStop();
		int _systemUpdateStream();

		int _fillBuffer(int size);
		void _getData(int size, unsigned char** data1, int* size1, unsigned char** data2, int* size2);

	};

}
#endif
#endif