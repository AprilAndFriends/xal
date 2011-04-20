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

namespace xal
{
	void xal_writelog(chstr text)
	{
		printf("%s\n", text.c_str());
	}
	void (*gLogFunction)(chstr) = xal_writelog;
	
	void init(chstr systemName, unsigned long backendId, chstr deviceName, bool threaded, float updateTime)
	{
		xal::log("initializing XAL");
		if (systemName == XAL_AS_DISABLED)
		{
			xal::mgr = new AudioManager(XAL_AS_DISABLED, backendId, deviceName, threaded, updateTime);
			xal::log("audio is disabled");
			return;
		}
#if HAVE_OPENAL
		if (systemName == XAL_AS_OPENAL)
		{
			xal::mgr = new OpenAL_AudioManager(systemName, backendId, deviceName, threaded, updateTime);
		}
#endif
#if HAVE_DIRECTSOUND
		if (systemName == XAL_AS_DIRECTSOUND)
		{
			xal::mgr = new DirectSound_AudioManager(systemName, backendId, deviceName, threaded, updateTime);
		}
#endif
#if HAVE_SDL
		if (systemName == XAL_AS_SDL)
		{
			xal::mgr = new SDL_AudioManager(systemName, backendId, deviceName, threaded, updateTime);
		}
#endif
		if (xal::mgr == NULL)
		{
			xal::log("audio system does not exist: " + systemName);
			xal::mgr = new AudioManager(XAL_AS_DISABLED, backendId, deviceName, threaded, updateTime);
			xal::log("audio is disabled");
			return;
		}
		xal::log("audio system created: " + systemName);
	}
	
	void destroy()
	{
		xal::log("destroying XAL");
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
