/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
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
	class Category;
	class xalExport Sound
	{
	public:
		Sound();
		virtual ~Sound();
		
		virtual void update(float k) { }

		// pure virtual
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
		virtual void stopSoft(float fadeTime = 0.0f, bool pause = false) = 0;
		
		// with default impl
		virtual void lock() { this->locked = true; }
		virtual void unlock() { this->locked = false; }
		virtual bool isLocked() { return this->locked; }
		
		virtual Category* getCategory() { return NULL; }
		virtual int getSampleOffset() { return 0; }
		virtual unsigned int getSourceId() { return 0; }
		
	protected:
		bool locked;
		
	};

}

#endif
