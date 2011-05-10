/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>

#include "AudioManager.h"
#include "DirectSound_AudioManager.h"
#include "OpenAL_AudioManager.h"
#include "SDL_AudioManager.h"
#include "xal.h"

#ifdef _WIN32
	#if HAVE_DIRECTSOUND
	#define XAL_AS_DEFAULT XAL_AS_DIRECTSOUND
	#elif HAVE_SDL
	#define XAL_AS_DEFAULT XAL_AS_SDL
	#elif HAVE_OPENAL
	#define XAL_AS_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_DEFAULT XAL_AS_DISABLED
	#endif
#elif defined(__APPLE__)
	#if HAVE_SDL
	#define XAL_AS_DEFAULT XAL_AS_SDL
	#elif HAVE_OPENAL
	#define XAL_AS_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_DEFAULT XAL_AS_DISABLED
	#endif
#elif defined(_UNIX)
	#if HAVE_SDL
	#define XAL_AS_DEFAULT XAL_AS_SDL
	#elif HAVE_OPENAL
	#define XAL_AS_DEFAULT XAL_AS_OPENAL
	#else
	#define XAL_AS_DEFAULT XAL_AS_DISABLED
	#endif
#else
	#define XAL_AS_DEFAULT XAL_AS_DISABLED
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
		if (name == "")
		{
			name = XAL_AS_DEFAULT;
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
		if (xal::mgr == NULL)
		{
			xal::log("audio system does not exist: " + name);
			xal::mgr = new AudioManager(XAL_AS_DISABLED, backendId, threaded, updateTime, deviceName);
			xal::log("audio is disabled");
			return;
		}
		xal::log("audio system created: " + name);
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

}
