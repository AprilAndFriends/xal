/// @file
/// @author  Boris Mikic
/// @version 2.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents a playlist.

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
		bool isPaused();
		harray<hstr> getSoundNames();
		Player* getCurrentPlayer();
		
		void update();
		
		void clear();
		void queueSound(chstr name);
		void queueSounds(harray<hstr> names);
		
		void play(float fadeTime = 0.0f);
		void stop(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		void shuffle();
		void reset();
		
	protected:
		bool enabled;
		bool playing;
		bool repeatAll;
		int index;
		harray<Player*> players;
	
	};
	
}

#endif
