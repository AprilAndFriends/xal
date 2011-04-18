/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for OpenAL.

#if HAVE_OPENAL
#ifndef XAL_OPENAL_AUDIO_MANAGER_H
#define XAL_OPENAL_AUDIO_MANAGER_H

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

#define XAL_MAX_SOURCES 16

namespace xal
{
	class Buffer;
	class Player;
	class Sound;

	class xalExport OpenAL_AudioManager : public AudioManager
	{
	public:
		OpenAL_AudioManager(chstr deviceName = "", bool threaded = false, float updateTime = 0.01f);
		~OpenAL_AudioManager();
		
		unsigned int _allocateSourceId(); // TODO - should be protected

	protected:
		ALCdevice* device;
		ALCcontext* context;
		unsigned int sourceIds[XAL_MAX_SOURCES];

		Player* _createAudioPlayer(Sound* sound, Buffer* buffer);

	};
	
}

#endif
#endif