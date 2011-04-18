/// @file
/// @author  Kresimir Spes
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

#if HAVE_OGG
#include "OGG_Decoder.h"
#endif

#include "Buffer.h"
#include "Category.h"
#include "Decoder.h"
#include "xal.h"

namespace xal
{
	Buffer::Buffer(chstr filename) : loaded(false), data(NULL), stream(NULL)
	{
		this->filename = filename;
		this->fileSize = hfile::hsize(this->filename);
		this->decoder = NULL;
		Format format = this->getFormat();
		switch (format)
		{
#if HAVE_OGG
		case OGG:
			this->decoder = new OGG_Decoder(filename);
			break;
#endif
#if HAVE_M4A
		case M4A:
			this->decoder = new M4A_Decoder(filename);
			break;
#endif
		default:
			this->decoder = new Decoder(filename);
			break;
		}
	}

	Buffer::~Buffer()
	{
		xal::log("destroying buffer " + this->filename);
		delete this->decoder;
		if (this->data != NULL)
		{
			delete [] this->data;
		}
		if (this->stream != NULL)
		{
			delete [] this->stream;
		}
	}
	
	unsigned int Buffer::getSize()
	{
		return this->decoder->getSize();
	}

	unsigned int Buffer::getChannels()
	{
		return this->decoder->getChannels();
	}

	long Buffer::getRate()
	{
		return this->decoder->getRate();
	}

	float Buffer::getDuration()
	{
		return this->decoder->getDuration();
	}

	Format Buffer::getFormat()
	{
#if HAVE_OGG
		if (this->filename.ends_with(".ogg"))
		{
			return OGG;
		}
#endif
#if HAVE_M4A
		if (this->filename.ends_with(".m4a"))
		{
			return M4A;
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
			result = this->decoder->load(&this->data);
			if (result)
			{
				result = this->decoder->decode(this->data, &this->stream);
			}
		}
		if (result)
		{
			this->loaded = result;
		}
		return result;
	}

}
