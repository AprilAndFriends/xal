/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_OPENAL_AUDIOMANAGER_H
#define XAL_OPENAL_AUDIOMANAGER_H

#ifndef __APPLE__
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <TargetConditionals.h>
#endif

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class xalExport OpenAL_AudioManager : public AudioManager
	{
	public:
		OpenAL_AudioManager(chstr deviceName = "", bool threaded = false, float updateTime = 0.01f);
		~OpenAL_AudioManager();
		
		bool isEnabled();

	protected:
		ALCdevice* device;
		ALCcontext* context;

	};
	
}

#endif
