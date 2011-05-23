/// @file
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for AVFoundation.

#if 1

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
#ifndef XAL_AVFOUNDATION_AUDIO_MANAGER_H
#define XAL_AVFOUNDATION_AUDIO_MANAGER_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class AVFoundation_Player;
	class Player;
	class Sound;

	class xalExport AVFoundation_AudioManager : public AudioManager
	{
	public:
		friend class AVFoundation_Player;

		AVFoundation_AudioManager(chstr systemName, unsigned long backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~AVFoundation_AudioManager();
		
	protected:

		Player* _createAudioPlayer(Sound* sound, Buffer* buffer);

	};
	
}

#endif
#endif

#endif