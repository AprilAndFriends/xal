/// @file
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_M4A
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

#include "Endianess.h"
#include "AudioManager.h"
#include "M4A_Source.h"
#include "xal.h"

namespace xal
{
	M4A_Source::M4A_Source(chstr filename) : Source(filename), chunkOffset(0), audioFileID(0)
	{
	}

	M4A_Source::~M4A_Source()
	{
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
		/*
		 // TODO port over to ExtAudio
		CFURLRef urlref;
		
		
		if (urlref = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)this->filename.c_str(), this->filename.size(), false))
		{
			if (AudioFileOpenURL (urlref,  kAudioFileReadPermission, 0, &audioFileID) == 0) 
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
		 */
		this->streamOpen = false;
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
			// ExtAudioFile API has seek function
			// AudioFile API does not have it, sadly.
			this->close();
			this->open();
		}
	}

	bool M4A_Source::load(unsigned char* output)
	{
		if (!Source::load(output))
		{
			return false;
		}
        
        AudioBufferList fillBufList;
        fillBufList.mNumberBuffers = 1;
        fillBufList.mBuffers[0].mNumberChannels = streamDescription.mChannelsPerFrame;
        fillBufList.mBuffers[0].mDataByteSize = this->size;
        fillBufList.mBuffers[0].mData = output;
        
		UInt32 read = nFrames; // number of frames, not bytes, to read
		memset(output, 0, this->size); 
        if(ExtAudioFileRead(this->audioFileID, &read, &fillBufList) != noErr)
		{
			xal::log("m4a could not read a file");
			return false;
		}
		if(read != nFrames)
		{
			xal::log(hsprintf("Warning: m4a read size is not equal to requested size (requested %d vs. actually read %d)", (int)nFrames, (int)read));
		}

#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
		for (int i = 0; i < this->size; i += 2)
		{
			XAL_NORMALIZE_ENDIAN(*(uint16_t*)(output + i)); // always 16 bit data
		}
#endif	
		return true;
	}

	int M4A_Source::loadChunk(unsigned char* output, int size)
	{
		printf("m4a source loadchunk not ported\n");
#warning M4A source loadChunk unported to ext audio api
#if 0
		if (Source::loadChunk(output, size) == 0)
		{
			return 0;
		}
		
		UInt32 read = size;
		if(AudioFileReadBytes(this->audioFileID, false, chunkOffset, &read, output) != 0)
		{
			xal::log("m4a could not read a file");
			return false;
		}
		chunkOffset += read;
		
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
		for (int i = 0; i < read; i += 2)
		{
			XAL_NORMALIZE_ENDIAN(*(uint16_t*)(output + i)); // always 16 bit data
		}
#endif	
		return read;
#endif
        return 0;
	}

}
#endif
