/// @file
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for OpenSLES.

#ifdef _OPENSLES
#ifndef XAL_OPENSLES_AUDIO_MANAGER_H
#define XAL_OPENSLES_AUDIO_MANAGER_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

#define __CPP_WRAP(obj, method) (*obj)->method(obj)
#define __CPP_WRAP_ARGS(obj, method, ...) (*obj)->method(obj, __VA_ARGS__)

namespace xal
{
	class Buffer;
	class OpenSLES_Player;
	class Player;
	class Sound;

	class xalExport OpenSLES_AudioManager : public AudioManager
	{
	public:
		friend class OpenSLES_Player;

		OpenSLES_AudioManager(void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~OpenSLES_AudioManager();

	protected:
		SLObjectItf engineObject;
		SLEngineItf engine;
		SLObjectItf outputMixObject;

		Player* _createSystemPlayer(Sound* sound);
		
	};
	
}

#endif
#endif