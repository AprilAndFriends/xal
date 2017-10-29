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
/// Represents an audio category.

#ifndef XAL_CATEGORY_H
#define XAL_CATEGORY_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class AudioManager;
	class Player;

	/// @brief Defines an audio category which makes audio file organization easier.
	class xalExport Category
	{
	public:
		friend class AudioManager;
		friend class Player;

		/// @brief Constructor.
		/// @param[in] name Category name.
		/// @param[in] bufferMode How to handle the intermediate Buffer of the Sound.
		/// @param[in] sourceMode How to handle the Source of the Sound.
		Category(chstr name, BufferMode bufferMode, SourceMode sourceMode);
		/// @brief Destructor.
		~Category();
		
		/// @brief Gets name.
		HL_DEFINE_GET(hstr, name, Name);
		/// @brief Gets gain.
		float getGain();
		/// @brief Sets gain.
		void setGain(float value);
		/// @brief Gets the buffer-handling mode.
		HL_DEFINE_GET(BufferMode, bufferMode, BufferMode);
		/// @brief Gets the source-handling mode.
		HL_DEFINE_GET(SourceMode, sourceMode, SourceMode);
		/// @return True if gain is fading.
		bool isGainFading();
		/// @return True if Sounds in this Category are streamed.
		bool isStreamed() const;
		/// @return True if Sounds in this Category have their data managed by the system.
		bool isMemoryManaged() const;

		/// @brief Fades the gain to another value.
		/// @param[in] gainTarget The value to which the gain should be changed.
		/// @param[in] fadeTime Time how long the gain fade should take.
		void fadeGain(float gainTarget, float fadeTime = 1.0f);

	protected:
		/// @brief Category name.
		hstr name;
		/// @brief Category gain.
		float gain;
		/// @brief Category gain fade target.
		float gainFadeTarget;
		/// @brief Category gain fade speed.
		float gainFadeSpeed;
		/// @brief Category gain fade time.
		float gainFadeTime;
		/// @brief How to handle the intermediate Buffer of the Sound.
		BufferMode bufferMode;
		/// @brief sourceMode How to handle the Source of the Sound.
		SourceMode sourceMode;
		
		/// @note This method is not thread-safe and is for internal usage only.
		float _getGain() const;
		/// @note This method is not thread-safe and is for internal usage only.
		void _setGain(float value);
		/// @note This method is not thread-safe and is for internal usage only.
		bool _isGainFading() const;

		/// @brief Updates the Category.
		/// @param[in] timeDelta Time since the last update.
		/// @note This method is not thread-safe and is for internal usage only.
		void _update(float timeDelta);

	};

}
#endif
