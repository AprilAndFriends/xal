/// @file
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_COREAUDIO

#include <AudioUnit/AudioUnit.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1050
#include <AudioUnit/AUNTComponent.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "CoreAudio_AudioManager.h"
#include "CoreAudio_Player.h"
#include "Source.h"
#include "xal.h"

namespace xal
{
	
	// MARK: -
	// MARK: Constructor and its helper functions
	
#define CA_INIT_ASSERTION(assertion, message) \
	if (!(assertion)) \
	{ \
		xal::log("Unable to initialize CoreAudio: " message); \
		return; \
	}
	
	
	CoreAudio_AudioManager::CoreAudio_AudioManager(chstr systemName, unsigned long backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName)
	{
		xal::log("initializing CoreAudio");
		this->buffer = new unsigned char[1];
		this->bufferSize = 1;
		
		OSErr result = noErr;
		
		// find output audio unit
		Component outputComponent = this->_findOutputComponent();
		CA_INIT_ASSERTION(outputComponent == NULL, "FindNextComponent returned NULL");
		
		// open the output audio unit and initialize it
		result = OpenAComponent (outputComponent, &outputAudioUnit);
		CA_INIT_ASSERTION(result != noErr, "OpenAComponent() failed for output audio unit");
		result = AudioUnitInitialize(outputAudioUnit);
		CA_INIT_ASSERTION(result != noErr, "AudioUnitInitialize() failed for output audio unit");
		
		
		// tell the output audio unit what input will we connect to it
		AudioStreamBasicDescription unitDescription;
		memset(&unitDescription, 0, sizeof(unitDescription));
		unitDescription.mFormatID = kAudioFormatLinearPCM;
		unitDescription.mFormatFlags = kLinearPCMFormatFlagIsPacked;
		unitDescription.mChannelsPerFrame = 2;
		unitDescription.mSampleRate = 44100;
		unitDescription.mBitsPerChannel = 16;
		unitDescription.mFramesPerPacket = 1;
		unitDescription.mBytesPerFrame = unitDescription.mBitsPerChannel * unitDescription.mChannelsPerFrame / 8;
		unitDescription.mBytesPerPacket = unitDescription.mBytesPerFrame * unitDescription.mFramesPerPacket;
		result = AudioUnitSetProperty (outputAudioUnit,
									   kAudioUnitProperty_StreamFormat,
									   kAudioUnitScope_Input,
									   0,
									   &unitDescription,
									   sizeof (unitDescription));
		CA_INIT_ASSERTION(result != noErr, "AudioUnitSetProperty() failed to set input format for output audio unit");
		
		
		// assign callback
		struct AURenderCallbackStruct callback;
		memset(&callback, 0, sizeof(callback));
		callback.inputProc = _mixAudio;
		callback.inputProcRefCon = this;
		result = AudioUnitSetProperty (outputAudioUnit, 
									   kAudioUnitProperty_SetRenderCallback,
									   kAudioUnitScope_Input, 
									   0,
									   &callback, 
									   sizeof(callback));
		CA_INIT_ASSERTION(result != noErr, "AudioUnitSetProperty() failed to set callback for output audio unit");

		this->enabled = true;
		if (threaded)
		{
			this->_setupThread();
		}
	}
	
	
	Component CoreAudio_AudioManager::_findOutputComponent()
	{
		ComponentDescription outputComponentDescription;
		memset(&outputComponentDescription, 0, sizeof(outputComponentDescription));
		outputComponentDescription.componentType = kAudioUnitType_Output;
		outputComponentDescription.componentSubType = kAudioUnitSubType_DefaultOutput;
		outputComponentDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
		outputComponentDescription.componentFlags = 0;
		outputComponentDescription.componentFlagsMask = 0;
		
		return FindNextComponent (NULL, &outputComponentDescription);
	}
	
	// MARK: -
	// MARK: Destructor

	CoreAudio_AudioManager::~CoreAudio_AudioManager()
	{
		xal::log("destroying CoreAudio");
		/*
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		 */
		delete [] this->buffer;
	}
	
	// MARK: -
	// MARK: Rest of audio manager code
	
	Player* CoreAudio_AudioManager::_createAudioPlayer(Sound* sound, Buffer* buffer)
	{
		return new CoreAudio_Player(sound, buffer);
	}

	OSStatus CoreAudio_AudioManager::mixAudio(void                        *inRefCon,
											  AudioUnitRenderActionFlags  *ioActionFlags,
											  const AudioTimeStamp        *inTimeStamp,
											  UInt32                      inBusNumber,
											  UInt32                      inNumberFrames,
											  AudioBufferList             *ioData)
	{
		this->_lock();
		/*
		if (this->bufferSize != length)
		{
			delete [] this->buffer;
			this->buffer = new unsigned char[length];
			this->bufferSize = length;
		}
		memset(this->buffer, 0, this->bufferSize * sizeof(unsigned char));
		bool first = true;
		foreach (Player*, it, this->players)
		{
			((CoreAudio_Player*)(*it))->mixAudio(this->buffer, this->bufferSize, first);
			first = false;
		}
		
		//SDL_MixAudio(stream, this->buffer, this->bufferSize, SDL_MIX_MAXVOLUME);
		 */
		this->_unlock();
		
		return noErr;
	}

	OSStatus CoreAudio_AudioManager::_mixAudio(void                        *inRefCon,
											   AudioUnitRenderActionFlags  *ioActionFlags,
											   const AudioTimeStamp        *inTimeStamp,
											   UInt32                      inBusNumber,
											   UInt32                      inNumberFrames,
											   AudioBufferList             *ioData)
	{
		return ((CoreAudio_AudioManager*)xal::mgr)->mixAudio(inRefCon, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
	}

}
#endif