/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Provides an interface for audio sources.

#ifndef XAL_SOURCE_H
#define XAL_SOURCE_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Category;

	class xalExport Source
	{
	public:
		Source(chstr filename, Category* category);
		virtual ~Source();

		HL_DEFINE_GET(hstr, filename, Filename);
		HL_DEFINE_GET(SourceMode, mode, Mode);
		HL_DEFINE_IS(streamOpen, Open);
		HL_DEFINE_GET(int, size, Size);
		HL_DEFINE_GET(int, chunkSize, ChunkSize);
		HL_DEFINE_GET(int, channels, Channels);
		HL_DEFINE_GET(int, samplingRate, SamplingRate);
		HL_DEFINE_GET(int, bitsPerSample, BitsPerSample);
		HL_DEFINE_GET(float, duration, Duration);

		virtual bool open();
		virtual void close();
		virtual void rewind();
		virtual bool load(unsigned char* output);
		virtual int loadChunk(unsigned char* output, int size = STREAM_BUFFER_SIZE);
		
	protected:
		hstr filename;
		SourceMode mode;
		bool streamOpen;
		int size;
		int channels;
		int samplingRate;
		int bitsPerSample;
		float duration;
		int chunkSize;
		hsbase* stream;

	};

}

#endif
