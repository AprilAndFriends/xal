/// @file
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <stdio.h>

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include "AudioManager.h"
#ifdef _DIRECTSOUND
#include "DirectSound_AudioManager.h"
#endif
#ifdef _OPENAL
#include "OpenAL_AudioManager.h"
#endif
#ifdef _OPENSLES
#include "OpenSLES_AudioManager.h"
#endif
#ifdef _SDL
#include "SDL_AudioManager.h"
#endif
#ifdef _XAUDIO2
#include "XAudio2_AudioManager.h"
#endif
#include "NoAudio_AudioManager.h"
#include "xal.h"

#ifdef _COREAUDIO
#include "CoreAudio_AudioManager.h"
//#include "AVFoundation_AudioManager.h" // TODO: iOS maybe? probably leagacy code
#endif

#ifdef _WIN32
#ifndef _WINRT
	#ifdef _DIRECTSOUND
	#define AS_INTERNAL_DEFAULT AS_DIRECTSOUND
	#elif defined(_SDL)
	#define AS_INTERNAL_DEFAULT AS_SDL
	#elif defined(_OPENAL)
	#define AS_INTERNAL_DEFAULT AS_OPENAL
	#else
	#define AS_INTERNAL_DEFAULT AS_DISABLED
	#endif
#else
	#ifdef _XAUDIO2
	#define AS_INTERNAL_DEFAULT AS_XAUDIO2
	#else
	#define AS_INTERNAL_DEFAULT AS_DISABLED
	#endif
#endif
#elif defined(__APPLE__) && !defined(_IOS)
	#ifdef _COREAUDIO
	#define AS_INTERNAL_DEFAULT AS_COREAUDIO
	#elif defined(_SDL)
	#define AS_INTERNAL_DEFAULT AS_SDL
	#elif defined(_OPENAL)
	#define AS_INTERNAL_DEFAULT AS_OPENAL
	#else
	#define AS_INTERNAL_DEFAULT AS_DISABLED
	#endif
#elif defined(__APPLE__) && defined(_IOS)
	#ifdef _COREAUDIO
	#define AS_INTERNAL_DEFAULT AS_COREAUDIO
	#elif defined(_OPENAL)
	#define AS_INTERNAL_DEFAULT AS_OPENAL
	#elif defined(_AVFOUNDATION)
	#define AS_INTERNAL_DEFAULT AS_AVFOUNDATION
	#else
	#define AS_INTERNAL_DEFAULT AS_DISABLED
	#endif
	//AS_AVFOUNDATION
#elif defined(_UNIX)
	#ifdef _SDL
	#define AS_INTERNAL_DEFAULT AS_SDL
	#elif defined(_OPENAL)
	#define AS_INTERNAL_DEFAULT AS_OPENAL
	#else
	#define AS_INTERNAL_DEFAULT AS_DISABLED
	#endif
#elif defined(_ANDROID)
	#ifdef _OPENSLES
	#define AS_INTERNAL_DEFAULT AS_OPENSLES
	#elif defined(_OPENAL)
	#define AS_INTERNAL_DEFAULT AS_OPENAL
	#else
	#define AS_INTERNAL_DEFAULT AS_DISABLED
	#endif
#else
	#define AS_INTERNAL_DEFAULT AS_DISABLED
#endif

namespace xal
{
	hstr logTag = "xal";

	void init(AudioSystemType type, void* backendId, bool threaded, float updateTime, chstr deviceName)
	{
		hlog::write(xal::logTag, "Initializing XAL.");
		if (type == AS_DEFAULT)
		{
			type = AS_INTERNAL_DEFAULT;
		}
		if (type == AS_DISABLED)
		{
			xal::mgr = new NoAudio_AudioManager(backendId, threaded, updateTime, deviceName);
			hlog::write(xal::logTag, "Audio is disabled.");
			return;
		}
#ifdef _DIRECTSOUND
		if (type == AS_DIRECTSOUND)
		{
			xal::mgr = new DirectSound_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef _OPENAL
		if (type == AS_OPENAL)
		{
			xal::mgr = new OpenAL_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef _OPENSLES
		if (type == AS_OPENSLES)
		{
			xal::mgr = new OpenSLES_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef _SDL
		if (type == AS_SDL)
		{
			xal::mgr = new SDL_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef _XAUDIO2
		if (type == AS_XAUDIO2)
		{
			xal::mgr = new XAudio2_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
/*
#ifdef _IOS
		if (type == AS_AVFOUNDATION)
		{
			xal::mgr = new AVFoundation_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
*/
#ifdef _COREAUDIO
		if (type == AS_COREAUDIO)
		{
			xal::mgr = new CoreAudio_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
		if (xal::mgr == NULL)
		{
			hlog::warn(xal::logTag, "Could not create given audio system!");
			xal::mgr = new NoAudio_AudioManager(backendId, threaded, updateTime, deviceName);
			hlog::warn(xal::logTag, "Audio is disabled.");
			return;
		}
		hlog::write(xal::logTag, "Audio system created: " + xal::mgr->getName());
		// actually starts threading
		xal::mgr->init();
	}
	
	void destroy()
	{
		if (xal::mgr != NULL)
		{
			hlog::write(xal::logTag, "Destroying XAL.");
			xal::mgr->clear();
			delete xal::mgr;
			xal::mgr = NULL;
		}
	}
	
	bool hasAudioSystem(AudioSystemType type)
	{
#ifdef _DIRECTSOUND
		if (type == AS_DIRECTSOUND)
		{
			return true;
		}
#endif
#ifdef _OPENAL
		if (type == AS_OPENAL)
		{
			return true;
		}
#endif
#ifdef _OPENSLES
		if (type == AS_OPENSLES)
		{
			return true;
		}
#endif
#ifdef _SDL
		if (type == AS_SDL)
		{
			return true;
		}
#endif
#ifdef _XAUDIO2
		if (type == AS_XAUDIO2)
		{
			return true;
		}
#endif
		if (type == AS_DISABLED)
		{
			return true;
		}
		return false;
	}

}
