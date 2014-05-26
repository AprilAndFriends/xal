/// @file
/// @author  Boris Mikic
/// @version 3.02
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents a multi playlist.

#ifndef XAL_MULTI_PLAYLIST_H
#define XAL_MULTI_PLAYLIST_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/harray.h>

#include "xalUtilExport.h"

namespace xal
{
	class Playlist;

	class xalUtilExport MultiPlaylist
	{
	public:
		MultiPlaylist();
		~MultiPlaylist();
		
		HL_DEFINE_GET(harray<Playlist*>, playlists, Playlists);
		bool isEnabled();
		void setEnabled(bool value);
		bool isPlaying();
		bool isPaused();

		void registerPlaylist(Playlist* playlist);
		void unregisterPlaylist(Playlist* playlist);

		void clear();
		
		void update();
		
		void play(float fadeTime = 0.0f);
		void stop(float fadeTime = 0.0f);
		void pause(float fadeTime = 0.0f);
		void shuffle();
		void reset();
		
	protected:
		harray<Playlist*> playlists;
	
	};
	
}

#endif
