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
/// Represents an implementation of the AudioManager for OpenAL.

#if 0

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
#ifndef XAL_AVFOUNDATION_AUDIO_MANAGER_H
#define XAL_AVFOUNDATION_AUDIO_MANAGER_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

#define OPENAL_MAX_SOURCES 16

namespace xal
{
	class Buffer;
	class OpenAL_Player;
	class Player;
	class Sound;

	class xalExport OpenAL_AudioManager : public AudioManager
	{
	public:
		friend class OpenAL_Player;

		OpenAL_AudioManager(chstr systemName, unsigned long backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~OpenAL_AudioManager();
		
	protected:
		ALCdevice* device;
		ALCcontext* context;
		unsigned int sourceIds[OPENAL_MAX_SOURCES];
		bool allocated[OPENAL_MAX_SOURCES];

		Player* _createAudioPlayer(Sound* sound, Buffer* buffer);
		unsigned int _allocateSourceId();
		void _releaseSourceId(unsigned int sourceId);

	};
	
}

#endif
#endif

#endif