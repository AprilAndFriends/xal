/// @file
/// @version 3.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager that does not play any audio.

#ifndef XAL_NOAUDIO_AUDIO_MANAGER_H
#define XAL_NOAUDIO_AUDIO_MANAGER_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Sound;
	class Player;

	class xalExport NoAudio_AudioManager : public AudioManager
	{
	public:
		NoAudio_AudioManager(void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~NoAudio_AudioManager();

		hstr findAudioFile(chstr _filename) const;

	protected:
		Sound* _createSound(chstr filename, chstr categoryName, chstr prefix = "");
		Sound* _createSound(chstr name, chstr categoryName, unsigned char* data, int size, int channels, int samplingRate, int bitsPerSample);
		Player* _createSystemPlayer(Sound* sound);

	};

}

#endif
