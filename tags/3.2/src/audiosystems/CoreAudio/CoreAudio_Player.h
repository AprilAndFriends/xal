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
/// Represents an implementation of the Player for CoreAudio.

#if _COREAUDIO
#ifndef XAL_COREAUDIO_PLAYER_H
#define XAL_COREAUDIO_PLAYER_H

#include "Player.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;

	class xalExport CoreAudio_Player : public Player
	{
	public:
		CoreAudio_Player(Sound* sound);
		~CoreAudio_Player();

		void mixAudio(unsigned char* stream, int length, bool first);

	protected:
		bool playing;
		int position;
		float currentGain;
		unsigned char circleBuffer[STREAM_BUFFER];
		int readPosition;
		int writePosition;

		void _update(float timeDelta);

		inline bool _systemIsPlaying() { return this->playing; }
		float _systemGetOffset();
		void _systemSetOffset(float value);
		bool _systemPreparePlay();
		void _systemPrepareBuffer();
		void _systemUpdateGain(float gain);
		void _systemPlay();
		int _systemStop();
		int _systemUpdateStream();

		int _fillBuffer(int size);
		void _getData(int size, unsigned char** data1, int* size1, unsigned char** data2, int* size2);

	};

}
#endif
#endif