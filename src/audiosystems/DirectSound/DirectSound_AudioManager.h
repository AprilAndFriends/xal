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
/// Represents an implementation of the AudioManager for DirectSound.

#ifdef _DIRECTSOUND
#ifndef XAL_DIRECTSOUND_AUDIO_MANAGER_H
#define XAL_DIRECTSOUND_AUDIO_MANAGER_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

struct IDirectSound;
struct _DSCAPS;

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
		_DSCAPS* dsCaps;

		DirectSound_AudioManager(void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~DirectSound_AudioManager();

	protected:
		Player* _createSystemPlayer(Sound* sound);
		Source* _createSource(chstr filename, Category* category, Format format);

	};

}

#endif
#endif