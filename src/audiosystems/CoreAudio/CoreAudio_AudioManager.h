/// @file
/// @version 3.14
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an implementation of the AudioManager for CoreAudio.

#if _COREAUDIO
#ifndef XAL_COREAUDIO_AUDIO_MANAGER_H
#define XAL_COREAUDIO_AUDIO_MANAGER_H

#include <AudioUnit/AudioUnit.h>

#ifndef _IOS
// iOS contains newer API that exists on OS X 10.6, but
// not on earlier OS. So let's use older APIs.
#include <CoreServices/CoreServices.h>
#define AudioComponent Component
#define AudioComponentInstanceNew OpenAComponent
#define AudioComponentDescription ComponentDescription
#define AudioComponentFindNext FindNextComponent
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1050
#include <AudioUnit/AUNTComponent.h>
#endif

// for audio converter
#include <AudioToolbox/AudioToolbox.h>

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

		CoreAudio_AudioManager(void* backendId, bool threaded = false, float updateTime = 0.01f, chstr deviceName = "");
		~CoreAudio_AudioManager();
		OSStatus mixAudio(void                        *inRefCon,
						  AudioUnitRenderActionFlags  *ioActionFlags,
						  const AudioTimeStamp        *inTimeStamp,
						  UInt32                      inBusNumber,
						  UInt32                      inNumberFrames,
						  AudioBufferList             *ioData);

	protected:
		
		AudioUnit outputAudioUnit;
		AudioStreamBasicDescription unitDescription;
		
		AudioComponent _findOutputComponent();
		OSStatus _connectAudioUnit();
		
		Player* _createSystemPlayer(Sound* sound);

		static OSStatus _mixAudio(void                        *inRefCon,
								  AudioUnitRenderActionFlags  *ioActionFlags,
								  const AudioTimeStamp        *inTimeStamp,
								  UInt32                      inBusNumber,
								  UInt32                      inNumberFrames,
								  AudioBufferList             *ioData);
		
		void _convertStream(Buffer* buffer, unsigned char** stream, int *streamSize);
		static OSStatus _converterComplexInputDataProc(AudioConverterRef inAudioConverter,
													   UInt32* ioNumberDataPackets,
													   AudioBufferList* ioData,
													   AudioStreamPacketDescription** ioDataPacketDescription,
													   void* inUserData);

	private:
		AudioBuffer *_converter_currentBuffer;
		AudioStreamBasicDescription _converter_currentInputDescription;
	};

}

#endif
#endif