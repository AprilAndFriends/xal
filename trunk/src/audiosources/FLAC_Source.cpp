/// @file
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _FORMAT_FLAC
#include "FLAC_Source.h"
#include "xal.h"

namespace xal
{
	FLAC_Source::FLAC_Source(chstr filename, SourceMode sourceMode, BufferMode bufferMode) : Source(filename, sourceMode, bufferMode)
	{
	}

	FLAC_Source::~FLAC_Source()
	{
		this->close();
	}

	bool FLAC_Source::decode()
	{
		if (!this->streamOpen)
		{
			return false;
		}
		if (this->decoded)
		{
			return true;
		}
		// TODO - implement
		return Source::decode();
	}

	void FLAC_Source::close()
	{
		if (this->streamOpen)
		{
			this->streamOpen = false;
			// TODO - implement
		}
		Source::close();
	}

	void FLAC_Source::rewind()
	{
		if (this->streamOpen)
		{
			// TODO - implement
		}
	}

	bool FLAC_Source::load(unsigned char* output)
	{
		if (!Source::load(output))
		{
			return false;
		}
		// TODO - implement
		return true;
	}

	int FLAC_Source::loadChunk(unsigned char* output, int size)
	{
		if (Source::loadChunk(output, size) == 0)
		{
			return 0;
		}
		// TODO - implement
		return 0;
	}

}
#endif
