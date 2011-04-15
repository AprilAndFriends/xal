/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include "AudioManager.h"
#include "OpenAL_AudioManager.h"
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
		xal::mgr = new OpenAL_AudioManager(deviceName, threaded, updateTime);
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
