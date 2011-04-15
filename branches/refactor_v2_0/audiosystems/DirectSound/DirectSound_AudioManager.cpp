/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/hstring.h>

#include "DirectSound_AudioManager.h"

namespace xal
{
	DirectSound_AudioManager::DirectSound_AudioManager(chstr deviceName, bool threaded, float updateTime) :
		AudioManager(deviceName, threaded, updateTime)
	{
	}

	DirectSound_AudioManager::~DirectSound_AudioManager()
	{
	}
	
}
