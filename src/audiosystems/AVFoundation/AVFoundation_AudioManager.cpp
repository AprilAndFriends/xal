/// @file
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for iOS's AVFoundation.

#if 1

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
#include <string.h>

#include <hltypes/exception.h>
#include <hltypes/hdir.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#include "Buffer.h"
#include "Category.h"
#include "AVFoundation_AudioManager.h"
#include "AVFoundation_Player.h"
#include "xal.h"

namespace xal
{
	AVFoundation_AudioManager::AVFoundation_AudioManager(unsigned long backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(backendId, threaded, updateTime, deviceName)
	{
		this->name = XAL_AS_AVFOUNDATION;
		xal::log("initializing AVFoundation");
		this->enabled = true;
	}

	AVFoundation_AudioManager::~AVFoundation_AudioManager()
	{
		xal::log("destroying AVFoundation");
	}
	
	Player* AVFoundation_AudioManager::_createSystemPlayer(Sound* sound, Buffer* buffer)
	{
		return new AVFoundation_Player(sound, buffer);
	}
	

}
#endif

#endif
