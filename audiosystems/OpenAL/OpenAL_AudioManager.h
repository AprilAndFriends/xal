/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for OpenAL.

#ifdef HAVE_OPENAL
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

namespace xal
{
	class Buffer;
	class OpenAL_Player;
	class Player;
	class Sound;

	class xalExport OpenAL_AudioManager : public AudioManager
	{
	public:
		friend class OpenAL_Player;

		OpenAL_AudioManager(chstr systemName, void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~OpenAL_AudioManager();

		void suspendOpenALContext(); // TODO - iOS specific hack, should be removed later
		void resumeOpenALContext(); // TODO - iOS specific hack, should be removed later

	protected:
		ALCdevice* device;
		ALCcontext* context;

		Player* _createSystemPlayer(Sound* sound);
		unsigned int _allocateSourceId();
		void _releaseSourceId(unsigned int sourceId);

	};
	
}

#endif
#endif