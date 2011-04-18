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
	class Sound2;

	class xalExport OpenAL_Player : public Player
	{
	public:
		OpenAL_Player(Sound2* sound, Buffer* buffer, unsigned int sourceId);
		virtual ~OpenAL_Player();

		virtual void setGain(float value);
		unsigned int getSourceId() { return this->sourceId; }
		void setSourceId(unsigned int value) { this->sourceId = value; }

		void play(float fadeTime = 0.0f, bool looping = false);
		void stop(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		void stopSoft(float fadeTime = 0.0f, bool pause = false);

	protected:
		unsigned int sourceId;
		unsigned int bufferId;
		bool paused;
		bool looping;
		float fadeSpeed;
		float fadeTime;

	};

}
#endif
#endif