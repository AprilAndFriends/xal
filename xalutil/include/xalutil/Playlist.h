/// @file
/// @version 3.6
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
		/// @param[in] repeatAll Whether to loop the Playlist.
		Playlist(bool repeatAll = true);
		/// @brief Destructor.
		~Playlist();
		
		/// @return Gets gain at which the Playlist is played.
		HL_DEFINE_GET(float, gain, Gain);
		/// @return Sets gain at which the Playlist is played.
		void setGain(float value);
		/// @return Gets pitch multiplier.
		HL_DEFINE_GET(float, pitch, Pitch);
		/// @return Sets pitch multiplier.
		void setPitch(float value);
		/// @return Gets whether the Playlist is enabled.
		HL_DEFINE_IS(enabled, Enabled);
		/// @return Sets whether the Playlist is enabled.
		void setEnabled(bool value);
		/// @return True if the Playlist is playing.
		HL_DEFINE_IS(playing, Playing);
		HL_DEFINE_ISSET(repeatAll, RepeatAll);
		/// @return Gets the players.
		HL_DEFINE_GET(harray<Player*>, players, Players);
		/// @return True if the Playlist is paused.
		bool isPaused() const;
		/// @return Get all queued Sound names.
		harray<hstr> getSoundNames() const;
		/// @return Get the current Player playing.
		Player* getCurrentPlayer() const;
		
		/// @brief Updates the Playlist state.
		void update();
		
		/// @brief Clears all Sounds from the Playlist.
		void clear();
		/// @brief Queue a Sound at the end of the Playlist.
		/// @param[in] name Name of the Sound.
		/// @param[in] onlyOnce Play this sound only one time if the list repeats.
		void queueSound(chstr name, bool onlyOnce = false);
		/// @brief Queue an array of Sounds at the end of the Playlist.
		/// @param[in] names Array of Sound names.
		void queueSounds(harray<hstr> names);
		
		/// @brief Starts playing the Playlist.
		/// @param[in] fadeTime How long to fade-in the Playlist.
		/// @note Ignored if the Playlist is already playing.
		void play(float fadeTime = 0.0f);
		/// @brief Stops the Playlist completely.
		/// @param[in] fadeTime How long to fade-out the Playlist.
		void stop(float fadeTime = 0.0f);
		/// @brief Pauses the Playlist completely.
		/// @param[in] fadeTime How long to fade-out the Playlist.
		/// @note Unpause the Playlist with play().
		/// @see play
		void pause(float fadeTime = 0.0f);
		/// @brief Shuffles all Playlist entries randomly.
		void shuffle();
		/// @brief Stops and resets the Playlist to the beginning.
		void reset();
		
	protected:
		/// @brief The gain at which the Sound is played.
		/// @note In general "gain" is not the same as "volume".
		float gain;
		/// @brief The pitch multiplier.
		float pitch;
		/// @brief Whether the Playlist is enabled.
		bool enabled;
		/// @brief Whether the Playlist is playing.
		bool playing;
		/// @brief Whether the Playlist is looped.
		bool repeatAll;
		/// @brief Whether the Playlist is being looped the first time.
		bool firstLoop;
		/// @brief Index of the currently playing Player.
		int index;
		/// @brief Array of Players.
		harray<Player*> players;
		/// @brief Array of Players that should play only once.
		harray<Player*> onlyOncePlayers;

		/// @brief Special update for gain and pitch.
		void _updateParameters();

	};
	
}

#endif
