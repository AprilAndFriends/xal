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

#ifndef XAL_PLAYER_H
#define XAL_PLAYER_H

#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound2;

	class xalExport Player
	{
	public:
		Player(Sound2* sound, Buffer* buffer);
		virtual ~Player();

		float getGain() { return this->gain; }
		virtual void setGain(float value);
		Sound2* getSound() { return this->sound; }
		float getDuration();

		Category* getCategory();

		virtual bool isPlaying() { return false; }
		bool isPaused();
		bool isFading();
		bool isFadingIn();
		bool isFadingOut();
		bool isLooping() { return this->looping; }

		void update(float k);
		
		void play(float fadeTime = 0.0f, bool looping = false);
		void stop(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		void stopSoft(float fadeTime = 0.0f, bool pause = false);

	protected:
		Sound2* sound;
		Buffer* buffer;
		float gain;
		bool paused;
		bool looping;
		float fadeSpeed;
		float fadeTime;

		virtual void _sysSetBuffer(unsigned int channels, unsigned int rate, unsigned char* stream, unsigned int size) { }
		virtual void _sysUpdateFadeGain() { }
		virtual void _sysPlay() { }
		virtual void _sysStop() { }

	};

}
#endif
