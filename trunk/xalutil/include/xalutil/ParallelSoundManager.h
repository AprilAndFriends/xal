/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
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
		
		void addSound(chstr name);
		void updateList();
		void updateList(harray<hstr> names);
		void playAll();
		void pauseAll();
		void stopAll();
		void clear();
		
	protected:
		float fadeTime;
		harray<Player*> players;
		harray<hstr> sounds;
	
	};
	
}

#endif
