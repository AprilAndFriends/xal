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
	
	void init(chstr deviceName, bool threaded, float updateTime)
	{
		xal::log("initializing XAL");
		if (deviceName == "nosound")
		{
			xal::mgr = new AudioManager(deviceName, threaded, updateTime);
			xal::log("audio is disabled");
			return;
		}
#if HAVE_OPENAL
		xal::mgr = new OpenAL_AudioManager(deviceName, threaded, updateTime);
#elif HAVE_DIRECTSOUND
		xal::mgr = new DirectSound_AudioManager(deviceName, threaded, updateTime);
#elif HAVE_SDL
		xal::mgr = new SDL_AudioManager(deviceName, threaded, updateTime);
#else
		xal::mgr = new AudioManager(deviceName, threaded, updateTime);
		xal::log("audio is disabled");
#endif
	}
	
	void destroy()
	{
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
