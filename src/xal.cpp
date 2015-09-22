/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <stdio.h>

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>
#include <hltypes/hversion.h>
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
	#ifdef _SDL
		#define AS_INTERNAL_DEFAULT AS_SDL
	#elif defined(_OPENAL)
		#define AS_INTERNAL_DEFAULT AS_OPENAL
	#else
		#define AS_INTERNAL_DEFAULT AS_DISABLED
	#endif
#elif defined(__APPLE__) && defined(_IOS)
	#ifdef _OPENAL
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
#elif defined(_UNIX)
	#ifdef _SDL
		#define AS_INTERNAL_DEFAULT AS_SDL
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

	static hversion version(3, 4, 0);

	void init(AudioSystemType type, void* backendId, bool threaded, float updateTime, chstr deviceName)
	{
		hlog::write(logTag, "Initializing XAL: " + version.toString());
		if (type == AS_DEFAULT)
		{
			type = AS_INTERNAL_DEFAULT;
		}
		if (type == AS_DISABLED)
		{
			xal::manager = new NoAudio_AudioManager(backendId, threaded, updateTime, deviceName);
			hlog::write(logTag, "Audio is disabled.");
			return;
		}
#ifdef _DIRECTSOUND
		if (type == AS_DIRECTSOUND)
		{
			xal::manager = new DirectSound_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef _OPENAL
		if (type == AS_OPENAL)
		{
			xal::manager = new OpenAL_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef _OPENSLES
		if (type == AS_OPENSLES)
		{
			xal::manager = new OpenSLES_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef _SDL
		if (type == AS_SDL)
		{
			xal::manager = new SDL_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef _XAUDIO2
		if (type == AS_XAUDIO2)
		{
			xal::manager = new XAudio2_AudioManager(backendId, threaded, updateTime, deviceName);
		}
#endif
		if (xal::manager == NULL)
		{
			hlog::warn(logTag, "Could not create given audio system!");
			xal::manager = new NoAudio_AudioManager(backendId, threaded, updateTime, deviceName);
			hlog::warn(logTag, "Audio is disabled.");
			return;
		}
		hlog::write(logTag, "Audio system created: " + xal::manager->getName());
		// actually starts threading
		xal::manager->init();
	}
	
	void destroy()
	{
		if (xal::manager != NULL)
		{
			hlog::write(logTag, "Destroying XAL.");
			xal::manager->clear();
			delete xal::manager;
			xal::manager = NULL;
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
