/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
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
		Source(SoundBuffer* sound, unsigned int sourceId);
		~Source();

		void update(float k);
		
		Sound* play(float fadeTime = 0.0f, bool looping = false);
		void stop(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		void stopSoft(float fadeTime = 0.0f, bool pause = false);
		
		void unbind(bool pause = false);
		
		unsigned int getSourceId() { return this->sourceId; }
		void setSourceId(unsigned int value) { this->sourceId = value; }
		float getSampleOffset() { return this->sampleOffset; }
		unsigned int getBuffer();
		SoundBuffer* getSound() { return this->sound; }
		bool isBound() { return this->bound; }
		float getGain() { return this->gain; }
		void setGain(float value);
		bool isLooping() { return this->looping; }
		bool isPlaying();
		bool isPaused();
		bool isFading();
		bool isFadingIn();
		bool isFadingOut();
		
		Category *getCategory();
		
	protected:
		unsigned int sourceId;
		float gain;
		bool looping;
		bool paused;
		float fadeSpeed;
		float fadeTime;
		int sampleOffset;
		bool bound;
		SoundBuffer* sound;
		
	};

}
#endif
