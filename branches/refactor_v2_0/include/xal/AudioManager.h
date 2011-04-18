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
#if HAVE_OGG
		OGG,
#endif
#if HAVE_M4A
		M4A,
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
		AudioManager(chstr deviceName = "", bool threaded = false, float updateTime = 0.01f);
		virtual ~AudioManager();
		
		bool isEnabled() { return this->enabled; }
		hstr getDeviceName() { return this->deviceName; }
		float getUpdateTime() { return this->updateTime; }
		float getGlobalGain() { return this->gain; }
		void setGlobalGain(float value);

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

		static void update();
		void update(float k);
		
	protected:
		bool enabled;
		hstr deviceName;
		float updateTime;
		float gain;
		hmap<hstr, Category*> categories;
		harray<Player*> players;
		harray<Player*> managedPlayers;
		hmap<hstr, Sound*> sounds;
		
		void _setupThread();
		virtual Player* _createPlayer(Sound* sound, Buffer* buffer);

		//////////////////////////////////////////////

	public:
		
		void stopAll(float fadeTime = 0.0f);
		void pauseAll(float fadeTime = 0.0f);
		void stopCategory(chstr category, float fadeTime = 0.0f);
		
		void lockUpdate();
		void unlockUpdate();

	protected:
		hthread* thread;
		bool updating;
		
	};
	
	xalExport extern xal::AudioManager* mgr;

}

#endif
