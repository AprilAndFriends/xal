/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Provides bacic functionality for XAL.

#ifndef XAL_H
#define XAL_H

#include <hltypes/henum.h>
#include <hltypes/hstring.h>

#include "xalExport.h"

/// @brief Name of "No Audio" audiosystem.
#define XAL_AS_DISABLED "Disabled"
/// @brief Name of DirectSound audiosystem.
#define XAL_AS_DIRECTSOUND "DirectSound"
/// @brief Name of OpenAL audiosystem.
#define XAL_AS_OPENAL "OpenAL"
/// @brief Name of OpenSLES audiosystem.
#define XAL_AS_OPENSLES "OpenSLES"
/// @brief Name of SDL audiosystem.
#define XAL_AS_SDL "SDL"
/// @brief Name of XAudio2 audiosystem.
#define XAL_AS_XAUDIO2 "XAudio2"

namespace xal
{
	/// @brief Used for logging display.
	extern hstr logTag;

	/// @class AudioSystemType
	/// @brief Defines how audio sources should be handled.
	HL_ENUM_CLASS_PREFIX_DECLARE(xalExport, AudioSystemType,
	(
		/// @var static const AudioSystemType AudioSystemType::Default
		/// @note The OS-default can be different depending on how XAL was compiled.
		HL_ENUM_DECLARE(AudioSystemType, Default);
		/// @var static const AudioSystemType AudioSystemType::Disabled
		/// @brief No audio.
		HL_ENUM_DECLARE(AudioSystemType, Disabled);
		/// @var static const AudioSystemType AudioSystemType::DirectSound
		/// @brief Use DirectSound.
		HL_ENUM_DECLARE(AudioSystemType, DirectSound);
		/// @var static const AudioSystemType AudioSystemType::OpenAL
		/// @brief Use OpenAL.
		HL_ENUM_DECLARE(AudioSystemType, OpenAL);
		/// @var static const AudioSystemType AudioSystemType::OpenSLES
		/// @brief Use OpenSLES.
		HL_ENUM_DECLARE(AudioSystemType, OpenSLES);
		/// @var static const AudioSystemType AudioSystemType::SDL
		/// @brief Use SDL audio.
		HL_ENUM_DECLARE(AudioSystemType, SDL);
		/// @var static const AudioSystemType AudioSystemType::XAudio2
		/// @brief Use XAudio2.
		HL_ENUM_DECLARE(AudioSystemType, XAudio2);
	));

	/// @brief Initializes XAL.
	/// @param[in] type Type of the audio-system.
	/// @param[in] backendId Special system backend ID needed by some audio systems.
	/// @param[in] threaded Whether update should be handled in a separate thread.
	/// @param[in] updateTime How much time should pass between updates when "threaded" is enabled.
	/// @param[in] deviceName Required by some audio systems.
	/// @note On Win32, backendId is the window handle. On Android, backendId is a pointer to the JavaVM.
	xalFnExport void init(AudioSystemType type, void* backendId, bool threaded = true, float updateTime = 0.01f, chstr deviceName = "");
	/// @brief Destroys XAL.
	xalFnExport void destroy();
	/// @brief Checks if XAL was compiled with a given audio-system available.
	/// @param[in] type Type of the audio-system.
	/// @return True if XAL was compiled with a given audio-system.
	xalFnExport bool hasAudioSystem(AudioSystemType type);

}

#endif
