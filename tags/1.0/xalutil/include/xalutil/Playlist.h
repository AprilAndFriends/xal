/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_PLAYLISTMANAGER_H
#define XAL_PLAYLISTMANAGER_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include "xalUtilExport.h"

namespace xal
{
	class xalUtilExport Playlist
	{
	public:
		Playlist(bool repeatAll = true);
		~Playlist();
		
		void update();
		
		void play(float fadeTime = 0.0f);
		void stop(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		
		void clear();
		void queueSound(chstr name);
		void queueSounds(harray<hstr> names);
		
		bool isPlaying() { return this->playing; }
		bool isRepeatAll() { return this->repeatAll; }
		void setRepeatAll(bool value) { this->repeatAll = value; }
		harray<hstr> getSounds() { return this->sounds; }
		
	protected:
		bool playing;
		bool repeatAll;
		int index;
		harray<hstr> sounds;
	
	};
	
}

#endif
