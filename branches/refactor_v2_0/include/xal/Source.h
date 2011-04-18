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

		float getGain() { return this->gain; }
		void setGain(float value);
		SoundBuffer* getSound() { return this->sound; }
		unsigned int getSourceId() { return this->sourceId; }
		void setSourceId(unsigned int value) { this->sourceId = value; }

		Sound* play(float fadeTime = 0.0f, bool looping = false);
		void stop(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		void stopSoft(float fadeTime = 0.0f, bool pause = false);
		
	protected:
		float gain;
		SoundBuffer* sound;
		unsigned int sourceId;




	public:
		void update(float k);
		
        float getDuration();
        
		void unbind(bool pause = false);
		
		float getSampleOffset() { return this->sampleOffset; }
		unsigned int getBuffer() const;
		bool isBound() { return this->bound; }
		bool isLooping() { return this->looping; }
		bool isPlaying();
		bool isPaused();
		bool isFading();
		bool isFadingIn();
		bool isFadingOut();
		
		Category* getCategory();
		
	protected:
		bool looping;
		bool paused;
		float fadeSpeed;
		float fadeTime;
		float sampleOffset;
		bool bound;
		
	};

}
#endif
