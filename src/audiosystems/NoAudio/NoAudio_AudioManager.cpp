/// @file
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "NoAudio_AudioManager.h"
#include "NoAudio_Player.h"
#include "Sound.h"
#include "xal.h"

namespace xal
{
	NoAudio_AudioManager::NoAudio_AudioManager(void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(backendId, threaded, updateTime, deviceName)
	{
		this->name = XAL_AS_DISABLED;
		hlog::write(xal::logTag, "Initializing NoAudio.");
		this->enabled = false;
	}

	NoAudio_AudioManager::~NoAudio_AudioManager()
	{
		hlog::write(xal::logTag, "Destroying NoAudio.");
	}
	
	hstr NoAudio_AudioManager::findAudioFile(chstr _filename)
	{
		return "";
	}
	
	Sound* NoAudio_AudioManager::_createSound(chstr filename, chstr categoryName, chstr prefix)
	{
		Category* category = this->_getCategory(categoryName);
		Sound* sound = new Sound(filename, category, prefix);
		if (this->sounds.has_key(sound->getName()))
		{
			delete sound;
			return NULL;
		}
		this->sounds[sound->getName()] = sound;
		return sound;
	}

	Player* NoAudio_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new NoAudio_Player(sound);
	}

}
