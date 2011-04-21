/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "Source.h"
#include "xal.h"

namespace xal
{
	Source::Source(chstr filename) : size(0), channels(2), samplingRate(44100), bitsPerSample(16),
		duration(0.0f), chunkSize(0), streamed(false)
	{
		this->filename = filename;
	}

	Source::~Source()
	{
	}
	
	bool Source::load(unsigned char** output)
	{
		xal::log("loading dummy file " + this->filename);
		return true;
	}
	
	bool Source::load(unsigned char** output, int size)
	{
		xal::log("loading dummy file " + this->filename);
		return true;
	}
	
}
