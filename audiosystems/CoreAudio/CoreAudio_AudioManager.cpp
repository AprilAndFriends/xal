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
	
#define CA_INIT_ASSERTION_EX(assertion, message, returnvalue) \
	if (!(assertion)) \
	{ \
		xal::log("Unable to initialize CoreAudio: " message); \
		return returnvalue; \
	}
	
#define CA_INIT_ASSERTION(assertion, message) CA_INIT_ASSERTION_EX(assertion, message, /*void*/)
	
	
	CoreAudio_AudioManager::CoreAudio_AudioManager(chstr systemName, unsigned long backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(systemName, backendId, threaded, updateTime, deviceName)
	{
		xal::log("initializing CoreAudio");
		
		OSErr result = noErr;
		
		// find output audio unit
		Component outputComponent = this->_findOutputComponent();
		CA_INIT_ASSERTION(outputComponent != NULL, "FindNextComponent returned NULL");
		
		// open the output audio unit and initialize it
		result = OpenAComponent (outputComponent, &outputAudioUnit);
		CA_INIT_ASSERTION(result == noErr, "OpenAComponent() failed for output audio unit");
		result = AudioUnitInitialize(outputAudioUnit);
		CA_INIT_ASSERTION(result == noErr, "AudioUnitInitialize() failed for output audio unit");
		
		// tell output audio unit what we will connect to it,
		// and assign the callback to generate the data
		result = this->_connectAudioUnit();
		CA_INIT_ASSERTION(result == noErr, "_connectAudioUnit() failed");
		
		// start!
		result = AudioOutputUnitStart(outputAudioUnit);
		CA_INIT_ASSERTION(result == noErr, "AudioUnitOutputStart() failed");
		
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
	
	OSStatus CoreAudio_AudioManager::_connectAudioUnit()
	{
		OSStatus result;
		
		// tell the output audio unit what input will we connect to it
		AudioStreamBasicDescription unitDescription;
		memset(&unitDescription, 0, sizeof(unitDescription));
		unitDescription.mFormatID = kAudioFormatLinearPCM;
		unitDescription.mFormatFlags = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsSignedInteger;
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
		CA_INIT_ASSERTION_EX(result == noErr, "AudioUnitSetProperty() failed to set input format for output audio unit", result);
		
		
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
		CA_INIT_ASSERTION_EX(result == noErr, "AudioUnitSetProperty() failed to set callback for output audio unit", result);

		return noErr;
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
		 
		delete [] this->buffer;
		 */
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
		
		size_t length;
		int i;
		void *ptr;
		AudioBuffer *abuf;
		
		// hack:
		// we will render only one buffer.
		ioData->mNumberBuffers = 1;
		
		for (i = 0; i < ioData->mNumberBuffers; i++) {
			abuf = &ioData->mBuffers[i];
			length = abuf->mDataByteSize;
			ptr = abuf->mData;
			
			memset(ptr, 0, length);
			
			bool first = true;
			foreach (Player*, it, this->players)
			{
				((CoreAudio_Player*)(*it))->mixAudio((unsigned char*)ptr, length, first);
				first = false;
			}
		}
		
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