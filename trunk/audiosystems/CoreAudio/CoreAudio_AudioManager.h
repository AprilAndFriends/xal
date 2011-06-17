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
/// Represents an implementation of the AudioManager for CoreAudio.
/// This implementation uses Audio Queue Services from the
/// Audio Toolbox framework.

#if HAVE_COREAUDIO
#ifndef XAL_COREAUDIO_AUDIO_MANAGER_H
#define XAL_COREAUDIO_AUDIO_MANAGER_H

#include <AudioToolbox/AudioToolbox.h>

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

#define COREAUDIO_MAX_SOURCES 16

namespace xal
{
	class Buffer;
	class CoreAudio_Player;
	class Player;
	class Sound;

	class xalExport CoreAudio_AudioManager : public AudioManager
	{
	public:
		friend class CoreAudio_Player;

		CoreAudio_AudioManager(chstr systemName, unsigned long backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~CoreAudio_AudioManager();
		
	protected:
		/*
		ALCdevice* device;
		ALCcontext* context;
		 */
		unsigned int sourceIds[COREAUDIO_MAX_SOURCES];
		bool allocated[COREAUDIO_MAX_SOURCES];

		Player* _createAudioPlayer(Sound* sound, Buffer* buffer);
		unsigned int _allocateSourceId();
		void _releaseSourceId(unsigned int sourceId);

	};
	
}

#endif
#endif