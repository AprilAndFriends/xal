/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_SOUNDBUFFER_H
#define XAL_SOUNDBUFFER_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include "xalExport.h"

#include "Sound.h"

namespace xal
{
	class Category;
	class Source;
	
	class xalExport SoundBuffer : public Sound
	{
	public:
		SoundBuffer(chstr name, chstr category, chstr prefix = "");
		virtual ~SoundBuffer();

		bool load();
		
		void bindSource(Source* source);
		void unbindSource(Source* source);
		void lock();
		void unlock();
		bool isLocked();
		
		float getSampleOffset();
		virtual unsigned int getBuffer() = 0;
		void setSourceId(unsigned int value) { this->sourceId = value; }
		chstr getName() { return this->name; }
		float getDuration() { return this->duration; }
		Category* getCategory() { return this->category; }
		void setCategory(Category* value) { this->category = value; }
		float getGain();
		void setGain(float value);
		bool isLooping();
		bool isPlaying();
		bool isFading();
		bool isFadingIn();
		bool isFadingOut();
		bool isPaused();
		bool isOgg();
		
		Sound* play(float fadeTime = 0.0f, bool looping = false);
		void stop(float fadeTime = 0.0f);
		void stopAll(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		
	protected:
		hstr name;
		hstr filename;
		float duration;
		unsigned int sourceId;
		Category* category;
		harray<Source*> sources;
		
		virtual bool _loadOgg() = 0;
		
	};

}

#endif
