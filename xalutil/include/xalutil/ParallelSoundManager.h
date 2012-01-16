/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Manages playing of sounds on a global scale.

#ifndef XAL_PARALLELSOUNDMANAGER_H
#define XAL_PARALLELSOUNDMANAGER_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "xalUtilExport.h"

namespace xal
{
	class Player;

	class xalUtilExport ParallelSoundManager
	{
	public:
		ParallelSoundManager(float fadeTime = 1.0f);
		~ParallelSoundManager();
		
		float getFadeTime() { return this->fadeTime; }
		void setFadeTime(float value) { this->fadeTime = value; }
		harray<hstr> getSoundQueue() { return this->soundQueue; }
		void setSoundQueue(harray<hstr> value) { this->soundQueue = value; }
		
		void queueSound(chstr name);
		void updateList();
		void updateList(harray<hstr> names);
		void playAll();
		void pauseAll();
		void stopAll();
		void clear();
		
		DEPRECATED_ATTRIBUTE void addSound(chstr name) { this->queueSound(name); }

	protected:
		float fadeTime;
		harray<Player*> players;
		harray<hstr> soundQueue;
	
	};
	
}

#endif
