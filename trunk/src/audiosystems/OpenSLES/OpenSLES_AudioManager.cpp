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
/// Represents an implementation of the AudioManager for OpenSLES.

#ifdef _OPENSLES
#include <string.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <hltypes/exception.h>
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "Buffer.h"
#include "Category.h"
#include "OpenSLES_AudioManager.h"
#include "OpenSLES_Player.h"
#include "Sound.h"
#include "xal.h"

namespace xal
{
	OpenSLES_AudioManager::OpenSLES_AudioManager(void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(backendId, threaded, updateTime, deviceName), engineObject(NULL), engine(NULL), outputMixObject(NULL)
	{
		this->name = XAL_AS_OPENSLES;
		hlog::write(xal::logTag, "Initializing OpenSLES.");
		SLresult result;
		// creating engine
		result = slCreateEngine(&this->engineObject, 0, NULL, 0, NULL, NULL);
		if (result != SL_RESULT_SUCCESS)
		{
			hlog::error(xal::logTag, "Could not create engine object!");
			return;
		}
		result = __CPP_WRAP_ARGS(this->engineObject, Realize, SL_BOOLEAN_FALSE);
		if (result != SL_RESULT_SUCCESS)
		{
			hlog::error(xal::logTag, "Could not realize engine object!");
			__CPP_WRAP(this->engineObject, Destroy);
			this->engineObject = NULL;
			return;
		}
		result = __CPP_WRAP_ARGS(this->engineObject, GetInterface, SL_IID_ENGINE, &this->engine);
		if (result != SL_RESULT_SUCCESS)
		{
			hlog::error(xal::logTag, "Could not get engine interface!");
			__CPP_WRAP(this->engineObject, Destroy);
			this->engineObject = NULL;
			return;
		}
		// creating output mix
		result = __CPP_WRAP_ARGS(this->engine, CreateOutputMix, &this->outputMixObject, 0, NULL, NULL);
		if (result != SL_RESULT_SUCCESS)
		{
			hlog::error(xal::logTag, "Could not create output mix object!");
			return;
		}
		result = __CPP_WRAP_ARGS(this->outputMixObject, Realize, SL_BOOLEAN_FALSE);
		if (result != SL_RESULT_SUCCESS)
		{
			hlog::error(xal::logTag, "Could not realize output mix object!");
			__CPP_WRAP(this->outputMixObject, Destroy);
			return;
		}
		this->enabled = true;
	}

	OpenSLES_AudioManager::~OpenSLES_AudioManager()
	{
		hlog::write(xal::logTag, "Destroying OpenSLES.");
		if (this->outputMixObject != NULL)
		{
			__CPP_WRAP(this->outputMixObject, Destroy);
			this->outputMixObject = NULL;
		}
		this->engine = NULL;
		if (this->engineObject != NULL)
		{
			__CPP_WRAP(this->engineObject, Destroy);
			this->engineObject = NULL;
		}
	}
	
	Player* OpenSLES_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new OpenSLES_Player(sound);
	}
	
}
#endif
