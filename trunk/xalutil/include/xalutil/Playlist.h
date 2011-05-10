/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_PLAYLIST_H
#define XAL_PLAYLIST_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include "xalUtilExport.h"

namespace xal
{
	class Player;

	class xalUtilExport Playlist
	{
	public:
		Playlist(bool repeatAll = true);
		~Playlist();
		
		bool isEnabled() { return this->enabled; }
		void setEnabled(bool value) { this->enabled = value; }
		bool isPlaying() { return this->playing; }
		bool isRepeatAll() { return this->repeatAll; }
		void setRepeatAll(bool value) { this->repeatAll = value; }
		harray<Player*> getPlayers() { return this->players; }
		harray<hstr> getSoundNames();
		
		void update();
		
		void clear();
		void queueSound(chstr name);
		void queueSounds(harray<hstr> names);
		
		void play(float fadeTime = 0.0f);
		void stop(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		
	protected:
		bool enabled;
		bool playing;
		bool repeatAll;
		int index;
		harray<Player*> players;
	
	};
	
}

#endif
