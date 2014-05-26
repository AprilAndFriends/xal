/// @file
/// @version 3.12
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>
#include <hltypes/hresource.h>

#include "AudioManager.h"
#include "Category.h"
#include "Source.h"
#include "xal.h"

namespace xal
{
	Source::Source(chstr filename, Category* category) : streamOpen(false), size(0), channels(2),
		samplingRate(44100), bitsPerSample(16), duration(0.0f), stream(NULL)
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
			hlog::error(xal::logTag, "Unable to open: " + this->filename);
			return false;
		}
		if (this->stream == NULL)
		{
			hresource* resource = new hresource(this->filename);
			switch (this->mode)
			{
			case DISK:
				this->stream = resource;
				break;
			case RAM:
				this->stream = new hstream();
				this->stream->write_raw(*resource);
				delete resource;
				this->stream->rewind();
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
				delete this->stream;
				this->stream = NULL;
			}
			this->streamOpen = false;
		}
	}
	
	void Source::rewind()
	{
		if (this->streamOpen)
		{
			this->stream->rewind();
		}
	}
	
	bool Source::load(unsigned char* output)
	{
		hlog::write(xal::logTag, "Loading file: " + this->filename);
		if (!this->streamOpen)
		{
			hlog::error(xal::logTag, "File not open: " + this->filename);
			return false;
		}
		return true;
	}
	
	int Source::loadChunk(unsigned char* output, int count)
	{
		if (!this->streamOpen)
		{
			hlog::error(xal::logTag, "File not open: " + this->filename);
			return 0;
		}
		return 1; // means that "something" was read
	}
	
}
