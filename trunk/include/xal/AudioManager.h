/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_AUDIOMANAGER_H
#define XAL_AUDIOMANAGER_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include <map>
#include "xalExport.h"

#define XAL_MAX_SOURCES 16

namespace xal
{
	class Category;
	class Mutex;
	class Sound;
	class SoundBuffer;
	class Source;
	class Thread;

	class xalExport AudioManager
	{
	public:
		AudioManager();
		~AudioManager();
		
		void init(chstr deviceName = "", bool threaded = true, float updateTime = 0.01f);
		
		void logMessage(chstr message);
		hstr getDeviceName() { return this->deviceName; }
		bool isEnabled();
		float getUpdateTime() { return this->updateTime; }
		Mutex* getMutex() { return this->mutex; }
		float getGlobalGain() { return this->gain; }
		void setGlobalGain(float value);
		Category* getCategoryByName(chstr name);
		void createCategory(chstr name, bool streamed = false);
		float getCategoryGain(chstr category);
		void setCategoryGain(chstr category, float gain);
		
		void update();
		void update(float k);
		
		Source* createSource(SoundBuffer* sound, unsigned int sourceId);
		void destroySource(Source* source);
		unsigned int allocateSourceId();
		Sound* createSound(chstr filename, chstr categoryName, chstr prefix = "");
		harray<hstr> createSoundsFromPath(chstr path, chstr prefix = "");
		harray<hstr> createSoundsFromPath(chstr path, chstr category, chstr prefix);
		void destroySound(SoundBuffer* sound);
		void destroySoundsWithPrefix(chstr prefix);
		Sound* getSound(chstr name);
		
		void stopAll(float fadeTime = 0.0f);
		void stopCategory(chstr category, float fadeTime = 0.0f);
		
	protected:
		unsigned int sourceIds[XAL_MAX_SOURCES];
		harray<Source*> sources;
		hstr deviceName;
		float gain;
		std::map<hstr, Category*> categories;
		std::map<hstr, SoundBuffer*> sounds;
		Mutex* mutex;
		Thread* thread;
		float updateTime;
		
	};
	
	xalFnExport void init(chstr deviceName = "", bool threaded = true, float updateTime = 0.01f);
	xalFnExport void destroy();
	xalFnExport void setLogFunction(void (*function)(chstr));
	xalFnExport extern xal::AudioManager* mgr;
}

#endif
