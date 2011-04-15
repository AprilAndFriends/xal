/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
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
		///////////////////////////////////////////////

		chstr getName() { return this->name; }
		chstr getFilename() { return this->filename; }
		chstr getVirtualFilename() { return this->virtualFilename; }
		Category* getCategory() { return this->category; }
		float getDuration() { return this->duration; }

	protected:
		hstr name;
		hstr filename;
		hstr virtualFilename;
		Category* category;
		float duration;

		hstr _findLinkedFile();

		///////////////////////////////////////////////

	public:

		void destroySources();

		bool load();
		
		void bindSource(Sound* source);
		void unbindSource(Sound* source);
		void lock();
		void unlock();
		bool isLocked();
		
		float getSampleOffset();
		virtual unsigned int getBuffer() const = 0;
		void setSourceId(unsigned int value) { this->sourceId = value; }
		void setCategory(Category* value) { this->category = value; }
		float getGain();
		void setGain(float value);
		bool isLooping();
		bool isPlaying();
		bool isPaused();
		bool isFading();
		bool isFadingIn();
		bool isFadingOut();
		
		bool isLoaded() { return this->loaded; }
		bool isValidBuffer() const;
		bool isLink();
		bool isOgg();
		bool isM4a();
		
		virtual Sound* play(float fadeTime = 0.0f, bool looping = false);
		void stop(float fadeTime = 0.0f);
		void stopAll(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		void stopSoft(float fadeTime = 0.0f, bool pause = false); // pause argument is ignored
		

	protected:
		unsigned int sourceId;
		bool loaded;
		harray<Sound*> sources;
		
		
		virtual bool _loadOgg() = 0;
		
	};

}

#endif
