/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_DIRECTSOUND
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
#endif