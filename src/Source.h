/// @file
/// @version 4.0
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
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Utility.h"
#include "xalExport.h"

namespace xal
{
	class Category;

	class xalExport Source
	{
	public:
		Source(chstr filename, SourceMode sourceMode, BufferMode bufferMode);
		virtual ~Source();

		HL_DEFINE_GET(hstr, filename, Filename);
		HL_DEFINE_GET(SourceMode, sourceMode, SourceMode);
		HL_DEFINE_GET(BufferMode, bufferMode, BufferMode);
		HL_DEFINE_IS(streamOpen, Open);
		HL_DEFINE_GET(int, size, Size);
		HL_DEFINE_GET(int, channels, Channels);
		HL_DEFINE_GET(int, samplingRate, SamplingRate);
		HL_DEFINE_GET(int, bitsPerSample, BitsPerSample);
		HL_DEFINE_GET(float, duration, Duration);
		int getRamSize() const;

		virtual bool open();
		virtual void close();
		virtual void rewind();
		virtual bool load(hstream& output);
		virtual int loadChunk(hstream& output, int size = STREAM_BUFFER_SIZE);
		
	protected:
		hstr filename;
		SourceMode sourceMode;
		BufferMode bufferMode;
		bool streamOpen;
		int size;
		int channels;
		int samplingRate;
		int bitsPerSample;
		float duration;
		hsbase* stream;

	};

}

#endif
