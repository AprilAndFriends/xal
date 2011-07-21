/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic,                    *
				   Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_SOURCEAPPLE_H
#define XAL_SOURCEAPPLE_H

#include "xalExport.h"

#include "Sound.h"

namespace xal
{
	class SoundBuffer;
	class xalExport SourceApple : public Sound
	{
	public:
		SourceApple(SoundBuffer* sound, unsigned int sourceId);
		virtual ~SourceApple();


		
		
		void update(float k);
		
		void stopSoft(float fadeTime = 0.0f, bool pause = false);
		
		void unbind(bool pause = false);
		
		unsigned int getSourceId() { return this->sourceId; }
		void setSourceId(unsigned int value) { this->sourceId = value; }
		float getSampleOffset() { return this->sampleOffset; }
		unsigned int getBuffer() const;
		SoundBuffer* getSound() { return this->sound; }
		bool isBound() { return this->bound; }
		
		// minimum required implementation
		float getGain() { return this->gain; }
		void setGain(float value);
		bool isLooping() { return this->looping; }
		bool isPlaying();
		bool isPaused();
		bool isFading();
		bool isFadingIn();
		bool isFadingOut();
		Sound* play(float fadeTime = 0.0f, bool looping = false);
		void stop(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
        float getDuration();
		
		void* _rebuildPlayer(); // if we get -50 status error in SourceApple, player needs rebuilding. returns AVAudioPlayer* as void*
		void* _getPlayerAsVoidPtr() const;
		
	protected:
		unsigned int sourceId;
		float gain;
		bool looping;
		bool paused;
		float fadeSpeed;
		float fadeTime;
		float sampleOffset;
		bool bound;
		SoundBuffer* sound;
		
		void *avAudioPlayer_Void;
		void *avAudioPlayerDelegate_Void;
		
	};

}
#endif
