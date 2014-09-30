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
/// Represents a multi-playlist.

#ifndef XAL_MULTI_PLAYLIST_H
#define XAL_MULTI_PLAYLIST_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/harray.h>

#include "xalUtilExport.h"

namespace xal
{
	class Playlist;

	/// @brief Provides a mechanism for playing multiple Playlists at the same time.
	class xalUtilExport MultiPlaylist
	{
	public:
		/// @brief Constructor.
		MultiPlaylist();
		/// @brief Destructor.
		~MultiPlaylist();
		
		HL_DEFINE_GET(harray<Playlist*>, playlists, Playlists);
		/// @return True if the MultiPlaylist is enabled.
		bool isEnabled();
		/// @brief Enables or disables the MultiPlaylist.
		/// @param[in] value True or false.
		void setEnabled(bool value);
		/// @return True if the MultiPlaylist is playing.
		bool isPlaying();
		/// @return True if the MultiPlaylist is paused.
		bool isPaused();

		void registerPlaylist(Playlist* playlist);
		void unregisterPlaylist(Playlist* playlist);

		/// @brief Updates the MultiPlaylist state.
		void update();
		
		/// @brief Clears all Playlists from the MultiPlaylist.
		void clear();
		
		/// @brief Starts playing the MultiPlaylist.
		/// @param[in] fadetime How long to fade-in the MultiPlaylist.
		/// @note Ignored if the MultiPlaylist is already playing.
		void play(float fadeTime = 0.0f);
		/// @brief Stops the MultiPlaylist completely.
		/// @param[in] fadetime How long to fade-out the MultiPlaylist.
		void stop(float fadeTime = 0.0f);
		/// @brief Pauses the MultiPlaylist completely.
		/// @param[in] fadetime How long to fade-out the MultiPlaylist.
		/// @note Unpause the MultiPlaylist with play().
		/// @see play
		void pause(float fadeTime = 0.0f);
		/// @brief Shuffles all Playlist entries within the MultiPlaylist randomly.
		void shuffle();
		/// @brief Stops and resets the MultiPlaylist to the beginning.
		void reset();
		
	protected:
		/// @brief Array of Playlists.
		harray<Playlist*> playlists;
	
	};
	
}

#endif
