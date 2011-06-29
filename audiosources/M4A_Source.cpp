/// @file
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_M4A
#include <TargetConditionals.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreFoundation/CoreFoundation.h>
#if !TARGET_OS_IPHONE
#include <CoreServices/CoreServices.h>
#endif

#include "Endianess.h"
#include "AudioManager.h"
#include "M4A_Source.h"
#include "xal.h"

namespace xal
{
	M4A_Source::M4A_Source(chstr filename) : Source(filename), chunkOffset(0), audioFileID(0)
	{
		mFilename = filename;
	}

	M4A_Source::~M4A_Source()
	{
		this->close();
	}

	bool M4A_Source::open()
	{
		this->streamOpen = Source::open();
		if (!this->streamOpen)
		{
			return false;
		}
#if !TARGET_OS_IPHONE
        FSRef fsref;

		if (FSPathMakeRef((Byte *)this->filename.c_str(), &fsref, NULL) == 0) 
		{
			if (ExtAudioFileOpen (&fsref, &audioFileID) == 0) 
			{
				this->_readFileProps();
			}
			else
			{
				xal::log("m4a: error opening file!");
				this->streamOpen = false;
			}
		}
		else
		{
			xal::log("m4a: error creating fsref");
			this->streamOpen = false;
		}
#else
		CFURLRef urlref;
		
		
		if (urlref = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)this->filename.c_str(), this->filename.size(), false))
		{
			if (ExtAudioFileOpenURL(urlref, &audioFileID) == 0) 
			{
				this->_readFileProps();
			}
			else
			{
				xal::log("m4a: error opening file!");
				this->streamOpen = false;
			}
			CFRelease(urlref);
		}
		else
		{
			xal::log("m4a: error creating urlref");
			this->streamOpen = false;
		}
		
#endif
		
		
		
		return this->streamOpen;
	}
	
	void M4A_Source::_readFileProps()
	{
		UInt32 propSize;
		
		/* get original stream description */
		AudioStreamBasicDescription fileStreamDescription;
		propSize = sizeof(fileStreamDescription);
		ExtAudioFileGetProperty(audioFileID, kExtAudioFileProperty_FileDataFormat, &propSize, &fileStreamDescription);
		
		/* UInt64 nFrames; */
		propSize = sizeof(nFrames);
		ExtAudioFileGetProperty(audioFileID,  kExtAudioFileProperty_FileLengthFrames, &propSize, &nFrames);
        
        /* Fill out the desired client format. */
		streamDescription.mFormatID = kAudioFormatLinearPCM;
		streamDescription.mFormatFlags = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsSignedInteger;
		streamDescription.mChannelsPerFrame = 2;
		streamDescription.mSampleRate = 44100;
		streamDescription.mBitsPerChannel = 16;
		streamDescription.mFramesPerPacket = 1;
		streamDescription.mBytesPerFrame = streamDescription.mBitsPerChannel * streamDescription.mChannelsPerFrame / 8;
		streamDescription.mBytesPerPacket = streamDescription.mBytesPerFrame * streamDescription.mFramesPerPacket;
		
		OSStatus res = noErr;
		if (res = ExtAudioFileSetProperty(audioFileID, kExtAudioFileProperty_ClientDataFormat, sizeof(AudioStreamBasicDescription), &streamDescription) != noErr)
		{
			return; // TODO: should return a failure!
		}
		nFrames = nFrames * (streamDescription.mSampleRate / fileStreamDescription.mSampleRate);
		
		this->channels = streamDescription.mChannelsPerFrame;
		this->samplingRate = streamDescription.mSampleRate;
		this->bitsPerSample = 16; // should always be 16 bit data
		this->size = nFrames * streamDescription.mBytesPerFrame;
		this->duration = nFrames / streamDescription.mSampleRate;
		this->chunkOffset = 0;
		
	}

	void M4A_Source::close()
	{
		if (this->streamOpen)
		{
            ExtAudioFileDispose(this->audioFileID);
			this->audioFileID = 0;
			this->streamOpen = false;
		}
	  
	}

	void M4A_Source::rewind()
	{
		if (this->streamOpen)
		{
			
			printf("+ rewind for %s\n", mFilename.c_str());
			//ExtAudioFileSeek(this->audioFileID, 0);
			this->close();
			this->open();
		}
	}

	int M4A_Source::loadChunk(unsigned char* output, int size)
	{
		if (!Source::loadChunk(output, size))
		{
			return false;
		}
        
        AudioBufferList fillBufList;
        fillBufList.mNumberBuffers = 1;
        fillBufList.mBuffers[0].mNumberChannels = streamDescription.mChannelsPerFrame;
        fillBufList.mBuffers[0].mDataByteSize = size;
        fillBufList.mBuffers[0].mData = output;
        
		UInt32 frames = size / streamDescription.mBytesPerFrame;
		UInt32 read = frames; // number of frames, not bytes, to read
		printf("Loading chunk for %s of size %d - framecount %d\n", mFilename.c_str(), size, frames);
		memset(output, 0, size); 
        if(ExtAudioFileRead(this->audioFileID, &read, &fillBufList) != noErr)
		{
			xal::log("m4a could not read a file");
			return 0;
		}
		if(read != frames)
		{
			xal::log(hsprintf("Warning: m4a read size is not equal to requested size (requested %d vs. actually read %d)", (int)frames, (int)read));
		}

#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
		for (int i = 0; i < size; i += 2)
		{
			XAL_NORMALIZE_ENDIAN(*(uint16_t*)(output + i)); // always 16 bit data
		}
#endif	
		
		return read * streamDescription.mBytesPerFrame;
	}

	bool M4A_Source::load(unsigned char* output)
	{
		if (Source::load(output) == 0)
		{
			return 0;
		}
		
		return this->loadChunk(output, this->size);
	}

}
#endif
