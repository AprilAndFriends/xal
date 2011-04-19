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
/// Provides an interface to play and control audio data.

#if HAVE_OPENAL
#ifndef XAL_OPENAL_PLAYER_H
#define XAL_OPENAL_PLAYER_H

#include "Player.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;

	class xalExport OpenAL_Player : public Player
	{
	public:
		OpenAL_Player(Sound* sound, Buffer* buffer);
		virtual ~OpenAL_Player();

		virtual void setGain(float value);
		unsigned int getSourceId() { return this->sourceId; }
		void setSourceId(unsigned int value) { this->sourceId = value; }

		bool isPlaying();

	protected:
		unsigned int sourceId;
		unsigned int bufferId;

		float _sysGetOffset();
		void _sysSetOffset(float value);
		bool _sysPreparePlay();
		void _sysPrepareBuffer(int channels, int rate, unsigned char* stream, int size);
		void _sysUpdateFadeGain();
		void _sysPlay();
		void _sysStop();

	};

}
#endif
#endif