/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Provides an interface for audiomanagers.

#ifndef XAL_AUDIO_MANAGER_H
#define XAL_AUDIO_MANAGER_H

#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "xalExport.h"

namespace xal
{
	enum Format
	{
		//WAV,
#if HAVE_M4A
		M4A,
#endif
#if HAVE_OGG
		OGG,
#endif
#if HAVE_SPX
		SPX,
#endif
		UNKNOWN
	};

	class Buffer;
	class Category;
	class Player;
	class Sound;

	class xalExport AudioManager
	{
	public:
		AudioManager(unsigned long backendId, chstr deviceName = "", bool threaded = false, float updateTime = 0.01f);
		virtual ~AudioManager();
		void clear();
		
		bool isEnabled() { return this->enabled; }
		hstr getDeviceName() { return this->deviceName; }
		float getUpdateTime() { return this->updateTime; }
		float getGlobalGain() { return this->gain; }
		void setGlobalGain(float value);

		static void update();
		void update(float k);
		void lockUpdate();
		void unlockUpdate();

		Category* createCategory(chstr name, bool streamed = false, bool dynamicLoad = false);
		Category* getCategoryByName(chstr name);
		float getCategoryGain(chstr category);
		void setCategoryGain(chstr category, float gain);
		
		Sound* createSound(chstr filename, chstr categoryName, chstr prefix = "");
		harray<hstr> createSoundsFromPath(chstr path, chstr prefix = "");
		harray<hstr> createSoundsFromPath(chstr path, chstr category, chstr prefix);
		void destroySound(Sound* sound);
		void destroySoundsWithPrefix(chstr prefix);

		Player* createPlayer(chstr name);
		void destroyPlayer(Player* player);

		void play(chstr name, float fadeTime = 0.0f, bool looping = false, float gain = 1.0f);
		void stop(chstr name, float fadeTime = 0.0f);
		void stopFirst(chstr name, float fadeTime = 0.0f);
		void stopAll(float fadeTime = 0.0f);
		// TODO - pauses everything and resumes the right players / pauses and resumes entire audio module
		//void pauseAll(float fadeTime = 0.0f);
		//void resumeAll(float fadeTime = 0.0f);
		void stopCategory(chstr name, float fadeTime = 0.0f);
		bool isAnyPlaying(chstr name);
		bool isAnyFading(chstr name);
		bool isAnyFadingIn(chstr name);
		bool isAnyFadingOut(chstr name);

	protected:
		bool enabled;
		unsigned long backendId;
		hstr deviceName;
		float updateTime;
		float gain;
		hmap<hstr, Category*> categories;
		harray<Player*> players;
		harray<Player*> managedPlayers;
		hmap<hstr, Sound*> sounds;
		hthread* thread;
		bool updating;
		
		void _setupThread();
		Player* _createManagedPlayer(chstr name);
		void _destroyManagedPlayer(Player* player);

		virtual Player* _createAudioPlayer(Sound* sound, Buffer* buffer);
		
	};
	
	xalExport extern xal::AudioManager* mgr;

}

#endif
