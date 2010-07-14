/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_SOURCE_H
#define XAL_SOURCE_H

#include "xalExport.h"

#include "Sound.h"

namespace xal
{
	class SoundBuffer;
	
	class xalExport Source : public Sound
	{
	public:
		Source(unsigned int id);
		virtual ~Source();

		void update(float k);
		
		Sound* play(float fadeTime = 0.0f, bool looping = false);
		Sound* replay(float fadeTime = 0.0f, bool looping = false);
		void stop(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		
		void unbind();
		
		unsigned int getId() { return this->id; }
		float getSampleOffset();
		unsigned int getBuffer();
		SoundBuffer* getSound() { return this->sound; }
		void setSound(SoundBuffer* value) { this->sound = value; }
		bool isBound();
		float getGain() { return this->gain; }
		void setGain(float value);
		bool isLooping() { return this->looping; }
		bool isPlaying();
		bool isFading();
		bool isFadingIn();
		bool isFadingOut();
		bool isPaused();
		
	protected:
		unsigned int id;
		float gain;
		bool looping;
		bool paused;
		float fadeSpeed;
		float fadeTime;
		SoundBuffer* sound;
		
	};

}
#endif
