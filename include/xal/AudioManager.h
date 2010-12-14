/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_AUDIOMANAGER_H
#define XAL_AUDIOMANAGER_H

#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "xalExport.h"

#define XAL_MAX_SOURCES 16

namespace xal
{
	class Category;
	class Sound;
	class SoundBuffer;
	class Source;

	class xalExport AudioManager
	{
	public:
		AudioManager();
		~AudioManager();
		
		void init(chstr deviceName = "", bool threaded = false, float updateTime = 0.01f);
		
		hstr getDeviceName() { return this->deviceName; }
		bool isEnabled();
		float getUpdateTime() { return this->updateTime; }
		float getGlobalGain() { return this->gain; }
		void setGlobalGain(float value);
		Category* getCategoryByName(chstr name);
		void createCategory(chstr name, bool streamed = false, bool dynamicLoad = false);
		float getCategoryGain(chstr category);
		void setCategoryGain(chstr category, float gain);
		
		static void update();
		void update(float k);
		
		Sound* createSource(SoundBuffer* sound, unsigned int sourceId);
		Sound* createSourceApple(SoundBuffer* sound, unsigned int sourceId);
		void destroySource(Sound* source);
		unsigned int allocateSourceId();
		Sound* createSound(chstr filename, chstr categoryName, chstr prefix = "");
		harray<hstr> createSoundsFromPath(chstr path, chstr prefix = "");
		harray<hstr> createSoundsFromPath(chstr path, chstr category, chstr prefix);
		void destroySound(SoundBuffer* sound);
		void destroySoundsWithPrefix(chstr prefix);
		Sound* getSound(chstr name);
		
		void stopAll(float fadeTime = 0.0f);
		void pauseAll(float fadeTime = 0.0f);
		void stopCategory(chstr category, float fadeTime = 0.0f);
		
		void lockUpdate();
		void unlockUpdate();
		
	protected:
		unsigned int sourceIds[XAL_MAX_SOURCES];
		harray<Sound*> sources;
		hstr deviceName;
		float gain;
		hmap<hstr, Category*> categories;
		hmap<hstr, SoundBuffer*> sounds;
		float updateTime;
		hthread* thread;
		bool updating;
		
	};
	
	xalFnExport void init(chstr deviceName = "", bool threaded = true, float updateTime = 0.01f);
	xalFnExport void destroy();
	xalFnExport void setLogFunction(void (*function)(chstr));
	xalFnExport void log(chstr message, chstr prefix = "[xal] ");
	xalFnExport extern xal::AudioManager* mgr;
}

#endif
