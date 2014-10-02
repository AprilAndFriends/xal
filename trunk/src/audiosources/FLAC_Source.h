/// @file
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Provides a source for FLAC format.

#ifdef _FORMAT_FLAC
#ifndef XAL_FLAC_SOURCE_H
#define XAL_FLAC_SOURCE_H

#include <hltypes/hstring.h>

#include "Source.h"
#include "xalExport.h"

namespace xal
{
	class xalExport FLAC_Source : public Source
	{
	public:
		FLAC_Source(chstr filename, SourceMode sourceMode, BufferMode bufferMode);
		~FLAC_Source();

		bool open();
		void close();
		void rewind();
		bool load(hstream& output);
		int loadChunk(hstream& output, int size = STREAM_BUFFER_SIZE);

	};

}

#endif
#endif
