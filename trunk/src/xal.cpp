/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include "AudioManager.h"
#ifdef HAVE_DIRECTSOUND
#include "DirectSound_AudioManager.h"
#endif
#ifdef HAVE_OPENAL
#include "OpenAL_AudioManager.h"
#endif
#ifdef HAVE_SDL
#include "SDL_AudioManager.h"
#endif
#include "xal.h"

#if TARGET_OS_MAC
#include "CoreAudio_AudioManager.h"
#if TARGET_OS_IPHONE
#include "AVFoundation_AudioManager.h"
#endif
#endif


#ifdef _WIN32
	#if HAVE_DIRECTSOUND
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DIRECTSOUND
	#elif HAVE_SDL
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_SDL
	#elif HAVE_OPENAL
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#elif defined(__APPLE__) && !TARGET_OS_IPHONE
	#if HAVE_COREAUDIO
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_COREAUDIO
	#elif HAVE_SDL
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_SDL
	#elif HAVE_OPENAL
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#elif defined(__APPLE__) && TARGET_OS_IPHONE
	#if HAVE_COREAUDIO
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_COREAUDIO
	#elif HAVE_OPENAL
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#elif HAVE_AVFOUNDATION
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_AVFOUNDATION
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
	//XAL_AS_AVFOUNDATION
#elif defined(_UNIX)
	#if HAVE_SDL
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_SDL
	#elif HAVE_OPENAL
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
#endif

namespace xal
{
	void xal_writelog(chstr text)
	{
		printf("%s\n", text.c_str());
	}
	void (*gLogFunction)(chstr) = xal_writelog;
	
	void init(chstr systemName, unsigned long backendId, bool threaded, float updateTime, chstr deviceName)
	{
		xal::log("initializing XAL");
		hstr name = systemName;
		if (name == XAL_AS_DEFAULT)
		{
			name = XAL_AS_INTERNAL_DEFAULT;
		}
		if (name == XAL_AS_DISABLED)
		{
			xal::mgr = new AudioManager(name, backendId, threaded, updateTime, deviceName);
			xal::log("audio is disabled");
			return;
		}
#if HAVE_DIRECTSOUND
		if (name == XAL_AS_DIRECTSOUND)
		{
			xal::mgr = new DirectSound_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
#if HAVE_OPENAL
		if (name == XAL_AS_OPENAL)
		{
			xal::mgr = new OpenAL_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
#if HAVE_SDL
		if (name == XAL_AS_SDL)
		{
			xal::mgr = new SDL_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
#if TARGET_OS_IPHONE
		if (name == XAL_AS_AVFOUNDATION)
		{
			xal::mgr = new AVFoundation_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
#if HAVE_COREAUDIO
		if (name == XAL_AS_COREAUDIO) 
		{
			xal::mgr = new CoreAudio_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
		if (xal::mgr == NULL)
		{
			xal::log("audio system does not exist: " + name);
			xal::mgr = new AudioManager(XAL_AS_DISABLED, backendId, threaded, updateTime, deviceName);
			xal::log("audio is disabled");
			return;
		}
		xal::log("audio system created: " + name);
		// actually starts threading
		xal::mgr->init();
	}
	
	void destroy()
	{
		xal::log("destroying XAL");
		xal::mgr->clear();
		delete xal::mgr;
	}
	
	void log(chstr message, chstr prefix)
	{
		gLogFunction(prefix + message);
	}
	
	void setLogFunction(void (*function)(chstr))
	{
		gLogFunction = function;
	}

	bool hasAudioSystem(chstr name)
	{
#if HAVE_DIRECTSOUND
		if (name == XAL_AS_DIRECTSOUND)
		{
			return true;
		}
#endif
#if HAVE_OPENAL
		if (name == XAL_AS_OPENAL)
		{
			return true;
		}
#endif
#if HAVE_SDL
		if (name == XAL_AS_SDL)
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
