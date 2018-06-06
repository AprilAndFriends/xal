/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for CoreAudio.

#ifdef _COREAUDIO
#ifndef XAL_COREAUDIO_AUDIO_MANAGER_H
#define XAL_COREAUDIO_AUDIO_MANAGER_H

#include <CoreAudio/al.h>
#include <CoreAudio/alc.h>
#include <TargetConditionals.h>

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

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

		CoreAudio_AudioManager(void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~CoreAudio_AudioManager();

		void suspendCoreAudioContext();
		bool resumeCoreAudioContext();
		bool _resumeCoreAudioContext();

	protected:
		ALCdevice* device;
		ALCcontext* context;
		int numActiveSources;
		bool pendingResume;

		Player* _createSystemPlayer(Sound* sound);
		unsigned int _allocateSourceId();
		void _releaseSourceId(unsigned int sourceId);
		void _suspendAudio();
		void _resumeAudio();
		void _update(float timeDelta);
		void initCoreAudio();
		void destroyCoreAudio();
		void resetCoreAudio();
		
	};
	
}
#endif
#endif