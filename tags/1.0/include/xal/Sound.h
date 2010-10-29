/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_SOUND_H
#define XAL_SOUND_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include "xalExport.h"

namespace xal
{
	class xalExport Sound
	{
	public:
		Sound();
		virtual ~Sound();
		
		virtual void update(float k) { }

		virtual float getGain() = 0;
		virtual void setGain(float value) = 0;
		virtual bool isLooping() = 0;
		virtual bool isPlaying() = 0;
		virtual bool isFading() = 0;
		virtual bool isFadingIn() = 0;
		virtual bool isFadingOut() = 0;
		virtual bool isPaused() = 0;
		
		virtual Sound* play(float fadeTime = 0.0f, bool looping = false) = 0;
		virtual void stop(float fadeTime = 0.0f) = 0;
		virtual void pause(float fadeTime = 0.0f) = 0;
		
		void lock() { this->locked = true; }
		void unlock() { this->locked = false; }
		bool isLocked() { return this->locked; }
		
	protected:
		bool locked;
		
	};

}

#endif
