/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_DIRECTSOUND_AUDIOMANAGER_H
#define XAL_DIRECTSOUND_AUDIOMANAGER_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "DirectSound_AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class xalExport DirectSound_AudioManager : public AudioManager
	{
	public:
		DirectSound_AudioManager(chstr deviceName = "", bool threaded = false, float updateTime = 0.01f);
		~DirectSound_AudioManager();
		
	};

}

#endif
