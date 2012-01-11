/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_FLAC
#include "FLAC_Source.h"
#include "xal.h"

namespace xal
{
	FLAC_Source::FLAC_Source(chstr filename) : Source(filename)
	{
	}

	FLAC_Source::~FLAC_Source()
	{
	}

	bool FLAC_Source::open()
	{
		this->streamOpen = Source::open();
		if (!this->streamOpen)
		{
			return false;
		}
		// TODO - implement
		return this->streamOpen;
	}

	void FLAC_Source::close()
	{
		if (this->streamOpen)
		{
			// TODO - implement
			this->streamOpen = false;
		}
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
