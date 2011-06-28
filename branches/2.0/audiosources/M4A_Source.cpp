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
        printf("Working with %s\n", this->filename.c_str());
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
		
		/* AudioStreamBasicDescription streamDescription; */
		propSize = sizeof(streamDescription);
		ExtAudioFileGetProperty(audioFileID, kExtAudioFileProperty_FileDataFormat, &propSize, &streamDescription);
		
		/* UInt64 nFrames; */
		propSize = sizeof(nFrames);
		ExtAudioFileGetProperty(audioFileID,  kExtAudioFileProperty_FileLengthFrames, &propSize, &nFrames);
        
        
		// Fix bits per channel
		streamDescription.mBitsPerChannel = 16;
		
        // Fix derived values
        streamDescription.mBytesPerFrame = streamDescription.mBytesPerPacket = (streamDescription.mBitsPerChannel >> 3) * streamDescription.mChannelsPerFrame;
        streamDescription.mFramesPerPacket = 1;
		printf("bytes per packet: %d\n", streamDescription.mBytesPerPacket);
		printf("bits per channel: %d\n", streamDescription.mBitsPerChannel);
		printf("channels per frame: %d\n", streamDescription.mChannelsPerFrame);
        
        
		/*
		UInt64 nBytes;
		propSize = sizeof(nBytes);
		ExtAudioFileGetProperty(audioFileID, kAudioFilePropertyAudioDataByteCount, &propSize, &nBytes);
		*/
		this->channels = streamDescription.mChannelsPerFrame;
		this->samplingRate = streamDescription.mSampleRate;
		this->bitsPerSample = 16; // should always be 16 bit data
        /*
		this->size = nBytes;
         */

        this->size = nFrames * streamDescription.mBytesPerFrame;
		printf("Bytes: %d\n", this->size);
        printf("Bytes per frame: %d\n", streamDescription.mBytesPerFrame);
        printf("nFrames: %d\n", nFrames);
		this->duration = nFrames / streamDescription.mSampleRate;
		printf("Duration: %g\n", this->duration);
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
        if(ExtAudioFileRead(this->audioFileID, &read, &fillBufList) != 0)
//		if(AudioFileReadBytes(this->audioFileID, false, 0, &read, output) != 0)
		{
			xal::log("m4a could not read a file");
			return false;
		}
		printf("m4asource load read %d frames\n", (int)read);
		if(read != nFrames)
		{
			xal::log(hsprintf("Warning: m4a read size is not equal to requested size (requested %d vs. actually read %d)", (int)nFrames, (int)read));
			printf("Warning: m4a read size is not equal to requested size (requested %d vs. actually read %d)", (int)nFrames, (int)read);
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
