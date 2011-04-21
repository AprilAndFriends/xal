/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "Source.h"
#include "xal.h"

namespace xal
{
	Buffer::Buffer(chstr filename) : loaded(false), data(NULL), stream(NULL)
	{
		this->filename = filename;
		this->fileSize = hfile::hsize(this->filename);
		this->source = xal::mgr->_createSource(this->filename, this->getFormat());
	}

	Buffer::~Buffer()
	{
		xal::log("destroying buffer " + this->filename);
		delete this->source;
		if (this->data != NULL)
		{
			delete [] this->data;
		}
		if (this->stream != NULL)
		{
			delete [] this->stream;
		}
	}
	
	int Buffer::getSize()
	{
		return this->source->getSize();
	}

	int Buffer::getChannels()
	{
		return this->source->getChannels();
	}

	int Buffer::getSamplingRate()
	{
		return this->source->getSamplingRate();
	}

	int Buffer::getBitsPerSample()
	{
		return this->source->getBitsPerSample();
	}

	float Buffer::getDuration()
	{
		return this->source->getDuration();
	}

	Format Buffer::getFormat()
	{
#if HAVE_M4A
		if (this->filename.ends_with(".m4a"))
		{
			return M4A;
		}
#endif
#if HAVE_OGG
		if (this->filename.ends_with(".ogg"))
		{
			return OGG;
		}
#endif
#if HAVE_MP3
		if (this->filename.ends_with(".mp3"))
		{
			return MP3;
		}
#endif
#if HAVE_SPX
		if (this->filename.ends_with(".spx"))
		{
			return SPX;
		}
#endif
#if HAVE_WAV
		if (this->filename.ends_with(".wav"))
		{
			return WAV;
		}
#endif
		return UNKNOWN;
	}

	bool Buffer::load()
	{
		if (this->loaded)
		{
			return true;
		}
		bool result = false;
		Format format = this->getFormat();
		if (!xal::mgr->isEnabled())
		{
			result = (format != UNKNOWN);
		}
		else
		{
			result = this->source->load(&this->stream);
		}
		if (result)
		{
			this->loaded = result;
		}
		return result;
	}

}