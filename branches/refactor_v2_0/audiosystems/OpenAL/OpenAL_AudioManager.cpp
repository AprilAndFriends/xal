/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <hltypes/exception.h>
#include <hltypes/hdir.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>
#include <hltypes/util.h>

#ifndef __APPLE__
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <TargetConditionals.h>
#endif

#include "OpenAL_AudioManager.h"
#include "Category.h"
#include "SimpleSound.h"
#include "SoundBuffer.h"
#include "Source.h"
#include "StreamSound.h"
#include "xal.h"

#if TARGET_OS_IPHONE
#include "SourceApple.h"
#endif

namespace xal
{
	OpenAL_AudioManager::OpenAL_AudioManager(chstr deviceName, bool threaded, float updateTime) :
		AudioManager(deviceName, threaded, updateTime), device(NULL), context(NULL)
	{
		xal::log("initializing OpenAL");
		ALCdevice* currentDevice = alcOpenDevice(deviceName.c_str());
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			return;
		}
		this->deviceName = alcGetString(currentDevice, ALC_DEVICE_SPECIFIER);
		xal::log("audio device: " + this->deviceName);
		ALCcontext* currentContext = alcCreateContext(currentDevice, NULL);
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			return;
		}
		alcMakeContextCurrent(currentContext);
		if (alcGetError(currentDevice) != ALC_NO_ERROR)
		{
			return;
		}
		alGenSources(XAL_MAX_SOURCES, this->sourceIds);
		this->device = currentDevice;
		this->context = currentContext;
		this->deviceName = deviceName;
		if (threaded)
		{
			xal::log("starting thread management");
			this->updateTime = updateTime;
			this->updating = true;
			this->thread = new hthread(&AudioManager::update);
			this->thread->start();
			this->updating = false;
		}
	}

	OpenAL_AudioManager::~OpenAL_AudioManager()
	{
		xal::log("destroying OpenAL");
		if (this->device)
		{
			alDeleteSources(XAL_MAX_SOURCES, this->sourceIds);
			alcMakeContextCurrent(NULL);
			alcDestroyContext(this->context);
			alcCloseDevice(this->device);
		}
	}
	
	bool OpenAL_AudioManager::isEnabled()
	{
		return (this->device != NULL);
	}
	
}
