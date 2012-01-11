/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hfile.h>

#include "AudioManager.h"
#include "Source.h"
#include "xal.h"

namespace xal
{
	Source::Source(chstr filename) : size(0), channels(2), samplingRate(44100), bitsPerSample(16),
		duration(0.0f), chunkSize(0), streamOpen(false)
	{
		this->filename = filename;
	}

	Source::~Source()
	{
		if (this->streamOpen)
		{
			this->close();
		}
	}

	bool Source::open()
	{
		if (!hfile::exists(this->filename))
		{
			xal::log("ERROR: unable to open " + this->filename);
			return false;
		}
		return true;
	}
	
	bool Source::load(unsigned char* output)
	{
		xal::log("loading file " + this->filename);
		if (!this->streamOpen)
		{
			xal::log("Error: file " + this->filename + " is not open");
			return false;
		}
		return true;
	}
	
	int Source::loadChunk(unsigned char* output, int count)
	{
		if (!this->streamOpen)
		{
			xal::log("Error: file " + this->filename + " is not open");
			return 0;
		}
		return 1;
	}
	
}