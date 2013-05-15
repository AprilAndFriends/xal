/// @file
/// @author  Boris Mikic
/// @author  Kresimir Spes
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

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
#if !_HL_WINRT
	#ifdef _DIRECTSOUND
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DIRECTSOUND
	#elif defined(_SDL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_SDL
	#elif defined(_OPENAL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#else
	#ifdef _XAUDIO2
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_XAUDIO2
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#endif
#elif defined(__APPLE__) && !defined(_IOS)
	#ifdef _COREAUDIO
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_COREAUDIO
	#elif defined(_SDL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_SDL
	#elif defined(_OPENAL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#elif defined(__APPLE__) && defined(_IOS)
	#ifdef _COREAUDIO
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_COREAUDIO
	#elif defined(_OPENAL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#elif defined(_AVFOUNDATION)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_AVFOUNDATION
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
	//XAL_AS_AVFOUNDATION
#elif defined(_UNIX)
	#ifdef _SDL
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_SDL
	#elif defined(_OPENAL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#elif defined(_ANDROID)
	#ifdef _OPENAL
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
#endif

namespace xal
{
	hstr logTag = "xal";

	void init(chstr systemName, void* backendId, bool threaded, float updateTime, chstr deviceName)
	{
		hlog::write(xal::logTag, "Initializing XAL.");
		hstr name = systemName;
		if (name == XAL_AS_DEFAULT)
		{
			name = XAL_AS_INTERNAL_DEFAULT;
		}
		if (name == XAL_AS_DISABLED)
		{
			xal::mgr = new NoAudio_AudioManager(name, backendId, threaded, updateTime, deviceName);
			hlog::write(xal::logTag, "Audio is disabled.");
			return;
		}
#ifdef _DIRECTSOUND
		if (name == XAL_AS_DIRECTSOUND)
		{
			xal::mgr = new DirectSound_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef _OPENAL
		if (name == XAL_AS_OPENAL)
		{
			xal::mgr = new OpenAL_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef _SDL
		if (name == XAL_AS_SDL)
		{
			xal::mgr = new SDL_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef _XAUDIO2
		if (name == XAL_AS_XAUDIO2)
		{
			xal::mgr = new XAudio2_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
/*
#ifdef _IOS
		if (name == XAL_AS_AVFOUNDATION)
		{
			xal::mgr = new AVFoundation_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
*/
#ifdef _COREAUDIO
		if (name == XAL_AS_COREAUDIO)
		{
			xal::mgr = new CoreAudio_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
		if (xal::mgr == NULL)
		{
			hlog::write(xal::logTag, "Audio system does not exist: " + name);
			xal::mgr = new NoAudio_AudioManager(XAL_AS_DISABLED, backendId, threaded, updateTime, deviceName);
			hlog::write(xal::logTag, "Audio is disabled.");
			return;
		}
		hlog::write(xal::logTag, "Audio system created: " + name);
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
	
	bool hasAudioSystem(chstr name)
	{
#ifdef _DIRECTSOUND
		if (name == XAL_AS_DIRECTSOUND)
		{
			return true;
		}
#endif
#ifdef _OPENAL
		if (name == XAL_AS_OPENAL)
		{
			return true;
		}
#endif
#ifdef _SDL
		if (name == XAL_AS_SDL)
		{
			return true;
		}
#endif
#ifdef _XAUDIO2
		if (name == XAL_AS_XAUDIO2)
		{
			return true;
		}
#endif
		if (name == XAL_AS_DISABLED)
		{
			return true;
		}
		return false;
	}

}
