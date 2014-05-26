/// @file
/// @version 3.02
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Manages playing of sounds on a global scale.

#ifndef XAL_PARALLELSOUNDMANAGER_H
#define XAL_PARALLELSOUNDMANAGER_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
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
		
		HL_DEFINE_GETSET(float, fadeTime, FadeTime);
		HL_DEFINE_GETSET(harray<hstr>, soundQueue, SoundQueue);
		harray<hstr> getPlayingSounds();

		void stopSoundsWithPrefix(chstr prefix);		
		void queueSound(chstr name);
		void removeSound(chstr name);

		void updateList();
		void updateList(harray<hstr> names);
		void playAll();
		void pauseAll();
		void stopAll();
		void clear();
		
	protected:
		float fadeTime;
		harray<Player*> players;
		harray<hstr> soundQueue;
	
	};
	
}

#endif
