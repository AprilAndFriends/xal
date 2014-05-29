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

	/// @brief Provides a for playing multiple Sounds parallely.
	class xalUtilExport ParallelSoundManager
	{
	public:
		/// @brief Constructor.
		/// @param[in] fadetime How long to fade-in and fade-out the Sounds.
		ParallelSoundManager(float fadeTime = 1.0f);
		/// @brief Destructor.
		~ParallelSoundManager();
		
		HL_DEFINE_GETSET(float, fadeTime, FadeTime);
		HL_DEFINE_GETSET(harray<hstr>, soundQueue, SoundQueue);
		/// @return All currently playing Sound names.
		harray<hstr> getPlayingSounds();

		/// @brief Stops all sounds that have a certain prefix in their name.
		/// @param[in] prefix The prefix for the Sounds.
		void stopSoundsWithPrefix(chstr prefix);		
		/// @brief Queues a Sound.
		/// @param[in] name Name of the Sound.
		void queueSound(chstr name);
		/// @brief Queues a Sound.
		/// @param[in] name Name of the Sound.
		void removeSound(chstr name);

		/// @brief Updates playing list with curren queue.
		/// @note Empties queue;
		void updateList();
		/// @brief Updates playing list with curren queue.
		/// @param[in] names List of Sound names.
		/// @note Empties queue;
		void updateList(harray<hstr> names);
		/// @brief Starts playing all Sound that are currently registered.
		void playAll();
		/// @brief Stops all Sounds completely.
		void stopAll();
		/// @brief Pauses all Sounds completely.
		/// @note Unpause the Playlist with play().
		void pauseAll();
		/// @brief Clears the current queue.
		void clear();
		
	protected:
		/// @brief How long to fade-in the Sounds.
		float fadeTime;
		/// @brief Players for the playing Sounds.
		harray<Player*> players;
		/// @brief List of Sound names for playing.
		harray<hstr> soundQueue;
	
	};
	
}

#endif
