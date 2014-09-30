/// @file
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the Player for DirectSound.

#ifdef _DIRECTSOUND
#ifndef XAL_DIRECTSOUND_PLAYER_H
#define XAL_DIRECTSOUND_PLAYER_H

#include "Player.h"
#include "xalExport.h"

struct IDirectSoundBuffer;

namespace xal
{
	class Buffer;
	class Sound;

	class xalExport DirectSound_Player : public Player
	{
	public:
		DirectSound_Player(Sound* sound);
		~DirectSound_Player();

	protected:
		IDirectSoundBuffer* dsBuffer;
		int bufferCount;
		int bufferQueued;

		bool _systemIsPlaying();
		unsigned int _systemGetBufferPosition();
		float _systemGetOffset();
		void _systemSetOffset(float value);
		bool _systemPreparePlay();
		void _systemPrepareBuffer();
		void _systemUpdateGain();
		void _systemUpdatePitch();
		void _systemPlay();
		int _systemStop();
		int _systemUpdateStream();

		int _getProcessedBuffersCount();
		int _fillBuffers(int index, int count);
		void _copyBuffer(unsigned char* stream, int size, int count = 1);
		void _copySilence(int size, int count = 1);

	};

}
#endif
#endif