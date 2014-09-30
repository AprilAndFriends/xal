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
/// Represents a playlist.

#ifndef XAL_PLAYLIST_H
#define XAL_PLAYLIST_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "xalUtilExport.h"

namespace xal
{
	class Player;

	/// @brief Provides a mechanism for playing Sounds in a queue.
	class xalUtilExport Playlist
	{
	public:
		/// @brief Constructor.
		/// @param[in] repeatAll Whether the loop the Playlist.
		Playlist(bool repeatAll = true);
		/// @brief Destructor.
		~Playlist();
		
		HL_DEFINE_ISSET(enabled, Enabled);
		HL_DEFINE_IS(playing, Playing);
		HL_DEFINE_ISSET(repeatAll, RepeatAll);
		HL_DEFINE_GET(harray<Player*>, players, Players);
		/// @return True if the Playlist is paused.
		bool isPaused();
		/// @return Get all queued Sound names.
		harray<hstr> getSoundNames();
		/// @return Get the current Player playing.
		Player* getCurrentPlayer();
		
		/// @brief Updates the Playlist state.
		void update();
		
		/// @brief Clears all Sounds from the Playlist.
		void clear();
		/// @brief Queue a Sound at the end of the Playlist.
		/// @param[in] name Name of the Sound.
		void queueSound(chstr name);
		/// @brief Queue an array of Sounds at the end of the Playlist.
		/// @param[in] names Array of Sound names.
		void queueSounds(harray<hstr> names);
		
		/// @brief Starts playing the Playlist.
		/// @param[in] fadetime How long to fade-in the Playlist.
		/// @note Ignored if the Playlist is already playing.
		void play(float fadeTime = 0.0f);
		/// @brief Stops the Playlist completely.
		/// @param[in] fadetime How long to fade-out the Playlist.
		void stop(float fadeTime = 0.0f);
		/// @brief Pauses the Playlist completely.
		/// @param[in] fadetime How long to fade-out the Playlist.
		/// @note Unpause the Playlist with play().
		/// @see play
		void pause(float fadeTime = 0.0f);
		/// @brief Shuffles all Playlist entries randomly.
		void shuffle();
		/// @brief Stops and resets the Playlist to the beginning.
		void reset();
		
	protected:
		/// @brief Whether the Playlist is enabled.
		bool enabled;
		/// @brief Whether the Playlist is playing.
		bool playing;
		/// @brief Whether the Playlist is looped.
		bool repeatAll;
		/// @brief Index of the currently playing Player.
		int index;
		/// @brief Array of Players.
		harray<Player*> players;

	};
	
}

#endif
