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

	protected:
		float gain;
		Sound2* sound;
		Buffer* buffer;

	};

}
#endif
