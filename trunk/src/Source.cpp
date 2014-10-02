/// @file
/// @version 3.3
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
#include "Source.h"
#include "xal.h"

namespace xal
{
	Source::Source(chstr filename, SourceMode sourceMode, BufferMode bufferMode) : streamOpen(false),
		size(0), channels(2), samplingRate(44100), bitsPerSample(16), duration(0.0f), stream(NULL)
	{
		this->filename = filename;
		this->sourceMode = sourceMode;
		this->bufferMode = bufferMode;
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
			this->close();
			return this->streamOpen;
		}
		if (this->stream == NULL)
		{
			hresource* resource = new hresource(this->filename);
			if (this->sourceMode == RAM || this->bufferMode == ASYNC)
			{
				this->stream = new hstream();
				this->stream->write_raw(*resource);
				delete resource;
				this->stream->rewind();
			}
			else // if sourceMode == DISK
			{
				this->stream = resource;
			}
		}
		else
		{
			this->stream->rewind();
		}
		this->streamOpen = true;
		return this->streamOpen;
	}

	void Source::close()
	{
		if (this->streamOpen)
		{
			if (this->sourceMode == DISK)
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
	
	bool Source::load(hstream& output)
	{
		hlog::write(xal::logTag, "Loading file: " + this->filename);
		if (!this->streamOpen)
		{
			hlog::error(xal::logTag, "File not open: " + this->filename);
			return false;
		}
		return true;
	}
	
	int Source::loadChunk(hstream& output, int size)
	{
		if (!this->streamOpen)
		{
			hlog::error(xal::logTag, "File not open: " + this->filename);
			return 0;
		}
		return 1; // means that "something" was read
	}
	
}
