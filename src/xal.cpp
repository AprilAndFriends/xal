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
#ifdef HAVE_ANDROID
#include "Android_AudioManager.h"
#endif
#ifdef HAVE_DIRECTSOUND
#include "DirectSound_AudioManager.h"
#endif
#ifdef HAVE_OPENAL
#include "OpenAL_AudioManager.h"
#endif
#ifdef HAVE_SDL
#include "SDL_AudioManager.h"
#endif
#ifdef HAVE_XAUDIO2
#include "XAudio2_AudioManager.h"
#endif
#include "NoAudio_AudioManager.h"
#include "xal.h"
/*
#if TARGET_OS_MAC
#include "CoreAudio_AudioManager.h"
#if TARGET_OS_IPHONE
#include "AVFoundation_AudioManager.h"
#endif
#endif
*/

#ifdef _WIN32
#if !_HL_WINRT
	#ifdef HAVE_DIRECTSOUND
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DIRECTSOUND
	#elif defined(HAVE_SDL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_SDL
	#elif defined(HAVE_OPENAL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#else
	#ifdef HAVE_XAUDIO2
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_XAUDIO2
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#endif
#elif defined(__APPLE__) && !TARGET_OS_IPHONE
	#ifdef HAVE_COREAUDIO
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_COREAUDIO
	#elif defined(HAVE_SDL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_SDL
	#elif defined(HAVE_OPENAL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#elif defined(__APPLE__) && TARGET_OS_IPHONE
	#ifdef HAVE_COREAUDIO
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_COREAUDIO
	#elif defined(HAVE_OPENAL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#elif defined(HAVE_AVFOUNDATION)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_AVFOUNDATION
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
	//XAL_AS_AVFOUNDATION
#elif defined(_UNIX)
	#ifdef HAVE_SDL
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_SDL
	#elif defined(HAVE_OPENAL)
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#elif defined(_ANDROID)
	#ifdef HAVE_ANDROID
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_ANDROID
	#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
	#endif
#else
	#define XAL_AS_INTERNAL_DEFAULT XAL_AS_DISABLED
#endif

namespace xal
{
	hstr logTag = "xal";

	void log(chstr message, chstr prefix)
	{
		hlog::write(xal::logTag, message);
	}
	
	void setLogFunction(void (*function)(chstr))
	{
	}

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
#ifdef HAVE_ANDROID
		if (name == XAL_AS_ANDROID)
		{
			xal::mgr = new Android_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef HAVE_DIRECTSOUND
		if (name == XAL_AS_DIRECTSOUND)
		{
			xal::mgr = new DirectSound_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef HAVE_OPENAL
		if (name == XAL_AS_OPENAL)
		{
			xal::mgr = new OpenAL_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef HAVE_SDL
		if (name == XAL_AS_SDL)
		{
			xal::mgr = new SDL_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
#ifdef HAVE_XAUDIO2
		if (name == XAL_AS_XAUDIO2)
		{
			xal::mgr = new XAudio2_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
/*
#if TARGET_OS_IPHONE
		if (name == XAL_AS_AVFOUNDATION)
		{
			xal::mgr = new AVFoundation_AudioManager(name, backendId, threaded, updateTime, deviceName);
		}
#endif
*/
#ifdef HAVE_COREAUDIO
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
#ifdef HAVE_ANDROID
		if (name == XAL_AS_ANDROID)
		{
			return true;
		}
#endif
#ifdef HAVE_DIRECTSOUND
		if (name == XAL_AS_DIRECTSOUND)
		{
			return true;
		}
#endif
#ifdef HAVE_OPENAL
		if (name == XAL_AS_OPENAL)
		{
			return true;
		}
#endif
#ifdef HAVE_SDL
		if (name == XAL_AS_SDL)
		{
			return true;
		}
#endif
#ifdef HAVE_XAUDIO2
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
