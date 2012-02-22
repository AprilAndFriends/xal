/// @file
/// @author  Boris Mikic
/// @version 2.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for DirectSound.

#ifdef HAVE_DIRECTSOUND
#ifndef XAL_DIRECTSOUND_AUDIO_MANAGER_H
#define XAL_DIRECTSOUND_AUDIO_MANAGER_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

struct IDirectSound;

namespace xal
{
	class Buffer;
	class Player;
	class Sound;
	class Source;

	class xalExport DirectSound_AudioManager : public AudioManager
	{
	public:
		IDirectSound* dsDevice;

		DirectSound_AudioManager(chstr systemName, unsigned long backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~DirectSound_AudioManager();

	protected:
		Player* _createSystemPlayer(Sound* sound, Buffer* buffer);
		Source* _createSource(chstr filename, Format format);

	};

}

#endif
#endif