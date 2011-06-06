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
/// Represents an implementation of the Player for DirectSound.

#if HAVE_DIRECTSOUND
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
		DirectSound_Player(Sound* sound, Buffer* buffer);
		~DirectSound_Player();

		void _update(float k);

	protected:
		IDirectSoundBuffer* dsBuffer;
		/// @brief Playing flag.
		/// @note Due to possible latency in DirectSound, this flag is being used to avoid problems.
		bool playing;
		int bufferCount;

		bool _sysIsPlaying() { return this->playing; }
		float _sysGetOffset();
		void _sysSetOffset(float value);
		bool _sysPreparePlay();
		void _sysPrepareBuffer();
		void _sysUpdateGain();
		void _sysUpdateFadeGain();
		void _sysPlay();
		void _sysStop();
		void _sysUpdateStream();

		int _getProcessedBuffersCount();
		int _fillBuffers(int index, int count);
		void _copyBuffer(int index, unsigned char* stream, int size);

	};

}
#endif
#endif