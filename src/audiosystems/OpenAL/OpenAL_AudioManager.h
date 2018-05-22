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
/// Represents an implementation of the AudioManager for OpenAL.

#ifdef _OPENAL
#ifndef XAL_OPENAL_AUDIO_MANAGER_H
#define XAL_OPENAL_AUDIO_MANAGER_H

#ifndef __APPLE__
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <TargetConditionals.h>
#endif

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

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

		OpenAL_AudioManager(void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~OpenAL_AudioManager();

		void suspendOpenALContext();
		bool resumeOpenALContext();
		bool _resumeOpenALContext();

	protected:
		ALCdevice* device;
		ALCcontext* context;
		int numActiveSources;
#ifdef _IOS // iOS exception handling dealing with Audio Session interruptions
		bool pendingResume;
#endif

		Player* _createSystemPlayer(Sound* sound);
		unsigned int _allocateSourceId();
		void _releaseSourceId(unsigned int sourceId);
#ifdef _IOS // iOS exception handling dealing with Audio Session interruptions
		void _suspendAudio();
		void _resumeAudio();
		void _update(float timeDelta);
#endif
		void initOpenAL();
		void destroyOpenAL();
		void resetOpenAL();
		
	};
	
}
#endif
#endif