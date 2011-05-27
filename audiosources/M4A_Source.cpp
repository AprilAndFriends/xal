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
			if (AudioFileOpen (&fsref, fsRdPerm, 0, &audioFileID) == 0) 
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
#endif
		
		
		
		return this->streamOpen;
	}
	
	void M4A_Source::_readFileProps()
	{
		UInt32 propSize;
		AudioStreamBasicDescription streamDescription;
		propSize = sizeof(streamDescription);
		AudioFileGetProperty(audioFileID, kAudioFilePropertyDataFormat, &propSize, &streamDescription);
		
		UInt64 nPackets;
		propSize = sizeof(nPackets);
		AudioFileGetProperty(audioFileID, kAudioFilePropertyAudioDataPacketCount, &propSize, &nPackets);
		
		UInt64 nBytes;
		propSize = sizeof(nBytes);
		AudioFileGetProperty(audioFileID, kAudioFilePropertyAudioDataByteCount, &propSize, &nBytes);
		
		this->channels = streamDescription.mChannelsPerFrame;
		this->samplingRate = streamDescription.mSampleRate;
		this->bitsPerSample = 16; // should always be 16 bit data
		this->size = nBytes;
		this->duration = (nPackets * streamDescription.mFramesPerPacket) / streamDescription.mSampleRate;
		
		this->chunkOffset = 0;
		
	}

	void M4A_Source::close()
	{
		
		if (this->streamOpen)
		{
			AudioFileClose(this->audioFileID);
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
		UInt32 read = this->size;
		if(AudioFileReadBytes(this->audioFileID, false, 0, &read, output) != 0)
		{
			xal::log("m4a could not read a file");
			return false;
		}
		if(read != this->size)
		{
			xal::log(hsprintf("Warning: m4a read size is not equal to requested size (requested %d vs. actually read %d)", this->size, read));
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
		
		if (Source::loadChunk(output, size) == 0)
		{
			return 0;
		}
		
		UInt32 read = size;
		if(AudioFileReadBytes(this->audioFileID, false, 0, &read, output) != 0)
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
		
	}

}
#endif
