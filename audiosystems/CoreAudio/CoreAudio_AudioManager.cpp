/// @file
/// @version 3.14
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if _COREAUDIO

#include <AudioUnit/AudioUnit.h>
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1050
#include <AudioUnit/AUNTComponent.h>
#endif
#include <hltypes/hlog.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include <hltypes/hltypesUtil.h>

#include "CoreAudio_AudioManager.h"
#include "CoreAudio_Player.h"
#include "Source.h"
#include "Buffer.h"
#include "xal.h"


#define COREAUDIO_MANUALLY_SET_OUTPUT_UNIT_PROPERTIES 0

namespace xal
{
	
	// MARK: -
	// MARK: Constructor and its helper functions
	
#define CA_INIT_ASSERTION_EX(assertion, message, returnvalue) \
	if (!(assertion)) \
	{ \
		hlog::write(logTag, "Unable to initialize CoreAudio: " message); \
		return returnvalue; \
	}
	
#define CA_INIT_ASSERTION(assertion, message) CA_INIT_ASSERTION_EX(assertion, message, /*void*/)
	
	CoreAudio_AudioManager::CoreAudio_AudioManager(void* backendId, bool threaded, float updateTime, chstr deviceName) :
		AudioManager(backendId, threaded, updateTime, deviceName)
	{
		this->name = XAL_AS_COREAUDIO;
		hlog::write(logTag, "initializing CoreAudio");
		
		// set up threads before moving on
//		if (threaded)
//		{
//			this->_setupThread();
//		}
		
		OSErr result = noErr;
		
		// find output audio unit
		AudioComponent outputComponent = this->_findOutputComponent();
		CA_INIT_ASSERTION(outputComponent != NULL, "FindNextComponent returned NULL");
		
		// open the output audio unit and initialize it
		result = AudioComponentInstanceNew (outputComponent, &outputAudioUnit);
		CA_INIT_ASSERTION(result == noErr, "OpenAComponent() failed for output audio unit");
		result = AudioUnitInitialize(outputAudioUnit);
		CA_INIT_ASSERTION(result == noErr, "AudioUnitInitialize() failed for output audio unit");
		
		// tell output audio unit what we will connect to it,
		// and assign the callback to generate the data
		result = this->_connectAudioUnit();
		CA_INIT_ASSERTION(result == noErr, "_connectAudioUnit() failed");
		
		// read back what we got
		UInt32 unitDescriptionSize = sizeof(unitDescription);
		result = AudioUnitGetProperty (outputAudioUnit,
									   kAudioUnitProperty_StreamFormat,
									   kAudioUnitScope_Input,
									   0,
									   &unitDescription,
									   &unitDescriptionSize);
		CA_INIT_ASSERTION(result == noErr, "AudioUnitGetProperty() failed to read input format for output audio unit");
		
		// start!
		// // FIXME // //
		// move to init() method which needs to be
		// launched AFTER constructor.
		// some coreaudio-callbacked code needs to use xal::mgr
		result = AudioOutputUnitStart(outputAudioUnit);
		CA_INIT_ASSERTION(result == noErr, "AudioUnitOutputStart() failed");
		this->enabled = true;
	}
	
	
	AudioComponent CoreAudio_AudioManager::_findOutputComponent()
	{
		AudioComponentDescription outputComponentDescription;
		memset(&outputComponentDescription, 0, sizeof(outputComponentDescription));
		outputComponentDescription.componentType = kAudioUnitType_Output;
#ifndef _IOS
		outputComponentDescription.componentSubType = kAudioUnitSubType_DefaultOutput;
#else
		outputComponentDescription.componentSubType = kAudioUnitSubType_RemoteIO;
#endif
		outputComponentDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
		outputComponentDescription.componentFlags = 0;
		outputComponentDescription.componentFlagsMask = 0;
		
		return AudioComponentFindNext (NULL, &outputComponentDescription);
	}
	
	OSStatus CoreAudio_AudioManager::_connectAudioUnit()
	{
		OSStatus result;
		
		// tell the output audio unit what input will we connect to it
		// also, fill member variable with format description
		/* AudioStreamBasicDescription unitDescription; */
		memset(&unitDescription, 0, sizeof(unitDescription));

#if COREAUDIO_MANUALLY_SET_OUTPUT_UNIT_PROPERTIES
		unitDescription.mFormatID = kAudioFormatLinearPCM;
		unitDescription.mFormatFlags = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsSignedInteger;
		unitDescription.mChannelsPerFrame = 2;
		unitDescription.mSampleRate = 44100;
		unitDescription.mBitsPerChannel = 16;
		unitDescription.mFramesPerPacket = 1;
		unitDescription.mBytesPerFrame = unitDescription.mBitsPerChannel * unitDescription.mChannelsPerFrame / 8;
		unitDescription.mBytesPerPacket = unitDescription.mBytesPerFrame * unitDescription.mFramesPerPacket;		
		CA_INIT_ASSERTION_EX(result == noErr, "AudioUnitSetProperty() failed to set input format for output audio unit", result);
#else
		// read the audio unit output properties.
		// serve it the same format on input.
		// (conversion will be performed using Audio Converter Services.)
		
		UInt32 descriptionSize = sizeof(unitDescription);
		result = AudioUnitGetProperty (outputAudioUnit,
									   kAudioUnitProperty_StreamFormat,
									   kAudioUnitScope_Output,
									   0,
									   &unitDescription,
									   &descriptionSize);
		CA_INIT_ASSERTION_EX(result == noErr, "AudioUnitGetProperty() failed to get output format for output audio unit", result);
#endif
		
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
		hlog::write(logTag, "destroying CoreAudio");
		/*
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		 
		delete [] this->buffer;
		 */
	}
	
	// MARK: -
	// MARK: Rest of audio manager code
	
	Player* CoreAudio_AudioManager::_createSystemPlayer(Sound* sound)
	{
		return new CoreAudio_Player(sound);
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
		
		for (i = 0; i < ioData->mNumberBuffers; i++)
		{
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
		if(!xal::mgr)
			return 0;
		return ((CoreAudio_AudioManager*)xal::mgr)->mixAudio(inRefCon, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
	}
	
	void CoreAudio_AudioManager::_convertStream(Buffer* buffer, unsigned char** stream, int *streamSize)
	{
		if (buffer->getBitsPerSample() != unitDescription.mBitsPerChannel || 
			buffer->getChannels() != unitDescription.mChannelsPerFrame || 
			buffer->getSamplingRate() != unitDescription.mSampleRate)
		{
			// describe the input format's description
			AudioStreamBasicDescription inputDescription;
			memset(&inputDescription, 0, sizeof(inputDescription));
			inputDescription.mFormatID = kAudioFormatLinearPCM;
			inputDescription.mFormatFlags = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsSignedInteger;
			inputDescription.mChannelsPerFrame = buffer->getChannels();
			inputDescription.mSampleRate = buffer->getSamplingRate();
			inputDescription.mBitsPerChannel = buffer->getBitsPerSample();
			inputDescription.mBytesPerFrame = (inputDescription.mBitsPerChannel * inputDescription.mChannelsPerFrame) / 8;
			inputDescription.mFramesPerPacket = 1; //*streamSize / inputDescription.mBytesPerFrame;
			inputDescription.mBytesPerPacket = inputDescription.mBytesPerFrame * inputDescription.mFramesPerPacket;
			
			// copy conversion output format's description from the
			// output audio unit's description.
			// then adjust framesPerPacket to match the input we'll be passing.
			
			// framecount of our input stream is based on the input bytecount.
			// output stream will have same number of frames, but different
			// number of bytes.
			AudioStreamBasicDescription outputDescription = unitDescription;
			outputDescription.mFramesPerPacket = 1; //inputDescription.mFramesPerPacket;
			outputDescription.mBytesPerPacket = outputDescription.mBytesPerFrame * outputDescription.mFramesPerPacket;
			
			// create an audio converter
			AudioConverterRef audioConverter;
			OSStatus acCreationResult = AudioConverterNew(&inputDescription, &outputDescription, &audioConverter);
			if(!audioConverter)
			{
				// bail out
				free(*stream);
				*streamSize = 0;
				*stream = (unsigned char*)malloc(0);
				return;
			}
			
			// calculate number of bytes required for output of input stream.
			// allocate buffer of adequate size.
			UInt32 outputBytes = outputDescription.mBytesPerPacket * (*streamSize / inputDescription.mBytesPerPacket); // outputDescription.mFramesPerPacket * outputDescription.mBytesPerFrame;
			unsigned char *outputBuffer = (unsigned char*)malloc(outputBytes);
			memset(outputBuffer, 0, outputBytes);
			
			// describe input data we'll pass into converter
			AudioBuffer inputBuffer;
			inputBuffer.mNumberChannels = inputDescription.mChannelsPerFrame;
			inputBuffer.mDataByteSize = *streamSize;
			inputBuffer.mData = *stream;
			
			// describe output data buffers into which we can receive data.
			AudioBufferList outputBufferList;
			outputBufferList.mNumberBuffers = 1;
			outputBufferList.mBuffers[0].mNumberChannels = outputDescription.mChannelsPerFrame;
			outputBufferList.mBuffers[0].mDataByteSize = outputBytes;
			outputBufferList.mBuffers[0].mData = outputBuffer;
			
			// set output data packet size
			UInt32 outputDataPacketSize = outputBytes / outputDescription.mBytesPerPacket;
			
			// fill class members with data that we'll pass into
			// the InputDataProc
			_converter_currentBuffer = &inputBuffer;
			_converter_currentInputDescription = inputDescription;
			
			// convert
			OSStatus result = AudioConverterFillComplexBuffer(audioConverter, /* AudioConverterRef inAudioConverter */
															  CoreAudio_AudioManager::_converterComplexInputDataProc, /* AudioConverterComplexInputDataProc inInputDataProc */
															  this, /* void *inInputDataProcUserData */
															  &outputDataPacketSize, /* UInt32 *ioOutputDataPacketSize */
															  &outputBufferList, /* AudioBufferList *outOutputData */
															  NULL /* AudioStreamPacketDescription *outPacketDescription */
															  );
			
			// change "stream" to describe our output buffer.
			// even if error occured, we'd rather have silence than unconverted audio.
			free(*stream);
			*stream = outputBuffer;
			*streamSize = outputBytes;
			
			// dispose of the audio converter
			AudioConverterDispose(audioConverter);
		}
	}
	
	
	OSStatus CoreAudio_AudioManager::_converterComplexInputDataProc(AudioConverterRef inAudioConverter,
																	UInt32* ioNumberDataPackets,
																	AudioBufferList* ioData,
																	AudioStreamPacketDescription** ioDataPacketDescription,
																	void* inUserData)
	{
		if(ioDataPacketDescription)
		{
			hlog::write(logTag, "_converterComplexInputDataProc cannot provide input data; it doesn't know how to provide packet descriptions");
			*ioDataPacketDescription = NULL;
			*ioNumberDataPackets = 0;
			ioData->mNumberBuffers = 0;
			return 501;
		}
		
		CoreAudio_AudioManager *self = (CoreAudio_AudioManager*)inUserData;
		
		ioData->mNumberBuffers = 1;
		ioData->mBuffers[0] = *(self->_converter_currentBuffer);
		
		*ioNumberDataPackets = ioData->mBuffers[0].mDataByteSize / self->_converter_currentInputDescription.mBytesPerPacket;
		return 0;
	}
}
#endif