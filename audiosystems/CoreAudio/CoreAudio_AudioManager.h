/// @file
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
/// Represents an implementation of the AudioManager for CoreAudio.

#if HAVE_COREAUDIO
#ifndef XAL_COREAUDIO_AUDIO_MANAGER_H
#define XAL_COREAUDIO_AUDIO_MANAGER_H

#include <AudioUnit/AudioUnit.h>
#include <CoreServices/CoreServices.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1050
#include <AudioUnit/AUNTComponent.h>
#endif

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"


#define SDL_MAX_PLAYING 32

namespace xal
{
	class Buffer;
	class CoreAudio_Player;
	class Sound;
	class Player;

	class xalExport CoreAudio_AudioManager : public AudioManager
	{
	public:
		friend class CoreAudio_Player;

		CoreAudio_AudioManager(chstr systemName, unsigned long backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~CoreAudio_AudioManager();
		OSStatus mixAudio(void                        *inRefCon,
						  AudioUnitRenderActionFlags  *ioActionFlags,
						  const AudioTimeStamp        *inTimeStamp,
						  UInt32                      inBusNumber,
						  UInt32                      inNumberFrames,
						  AudioBufferList             *ioData);

	protected:
		
		AudioUnit outputAudioUnit;

		Component _findOutputComponent();
		OSStatus _connectAudioUnit();
		
		Player* _createAudioPlayer(Sound* sound, Buffer* buffer);

		static OSStatus _mixAudio(void                        *inRefCon,
								  AudioUnitRenderActionFlags  *ioActionFlags,
								  const AudioTimeStamp        *inTimeStamp,
								  UInt32                      inBusNumber,
								  UInt32                      inNumberFrames,
								  AudioBufferList             *ioData);

	};

}

#endif
#endif