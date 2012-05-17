/// @file
/// @author  Boris Mikic
/// @version 2.62
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hstream.h>
#include <hltypes/hresource.h>

#include "AudioManager.h"
#include "Category.h"
#include "Source.h"
#include "xal.h"

namespace xal
{
	Source::Source(chstr filename, Category* category) : streamOpen(false), size(0), channels(2),
		samplingRate(44100), bitsPerSample(16), duration(0.0f), chunkSize(0), stream(NULL)
	{
		this->filename = filename;
		this->mode = category->getSourceMode();
	}

	Source::~Source()
	{
		if (this->stream != NULL)
		{
			delete this->stream;
		}
	}

	bool Source::open()
	{
		if (!hresource::exists(this->filename))
		{
			xal::log("Error: unable to open " + this->filename);
			return false;
		}
		if (this->stream == NULL)
		{
			hresource* resource = new hresource(this->filename);
			switch (this->mode)
			{
			case DISK:
				this->stream = resource;
				xal::log("    - DISK OPEN");
				break;
			case RAM:
				this->stream = new hstream();
				this->stream->write_raw(*resource);
				delete resource;
				this->stream->rewind();
				xal::log("    - RAM OPEN");
				break;
			}
		}
		else
		{
			this->stream->rewind();
		}
		return true;
	}
	
	void Source::close()
	{
		if (this->streamOpen)
		{
			if (this->mode == DISK)
			{
				xal::log("    - DISK CLOSE");
				delete this->stream;
				this->stream = NULL;
			}
			else
			{
				xal::log("    - RAM CLOSE");
			}
			this->streamOpen = false;
		}
	}
	
	void Source::rewind()
	{
		if (this->streamOpen)
		{
			xal::log("    - REWIND");
			this->stream->rewind();
		}
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
