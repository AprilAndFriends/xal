/// @file
/// @author  Boris Mikic
/// @version 2.61
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hresource.h>

#include "AudioManager.h"
#include "Category.h"
#include "Source.h"
#include "xal.h"

namespace xal
{
	Source::Source(chstr filename, Category* category) : streamOpen(false), size(0), channels(2),
		samplingRate(44100), bitsPerSample(16), duration(0.0f), chunkSize(0)
	{
		this->filename = filename;
		this->mode = category->getSourceMode();
	}

	Source::~Source()
	{
		if (this->streamOpen)
		{
			xal::log("Warning: " + this->filename + " was not closed by subclass in destructor!");
		}
	}

	bool Source::open()
	{
		if (!hresource::exists(this->filename))
		{
			xal::log("Error: unable to open " + this->filename);
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
