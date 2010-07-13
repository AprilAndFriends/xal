/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
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
	class Category;
	class Source;
	
	class xalExport Sound
	{
	public:
		Sound(chstr name, chstr category, chstr prefix = "");
		virtual ~Sound();

		virtual bool load() = 0;
		void bindSource(Source* source);
		void unbindSource(Source* source);
		virtual void update(unsigned int sourceId) { }
		
		float getSampleOffset();
		virtual unsigned int getBuffer() = 0;
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
		
		Source* play(float fadeTime = 0.0f, bool looping = false);
		Source* replay(float fadeTime = 0.0f, bool looping = false);
		void stop(float fadeTime = 0.0f);
		void stopAll(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		
	protected:
		hstr name;
		hstr filename;
		float duration;
		Category* category;
		harray<Source*> sources;
		
	};

}

#endif
