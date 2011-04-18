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

#if HAVE_M4A
#include "M4A_Source.h"
#endif
#if HAVE_OGG
#include "OGG_Source.h"
#endif
#if HAVE_SPX
#include "SPX_Source.h"
#endif

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
		Format format = this->getFormat();
		switch (format)
		{
#if HAVE_M4A
		case M4A:
			this->source = new M4A_Source(filename);
			break;
#endif
#if HAVE_OGG
		case OGG:
			this->source = new OGG_Source(filename);
			break;
#endif
#if HAVE_SPX
		case SPX:
			this->source = new SPX_Source(filename);
			break;
#endif
		default:
			this->source = new Source(filename);
			break;
		}
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
	
	unsigned int Buffer::getSize()
	{
		return this->source->getSize();
	}

	unsigned int Buffer::getChannels()
	{
		return this->source->getChannels();
	}

	long Buffer::getRate()
	{
		return this->source->getRate();
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
#if HAVE_SPX
		if (this->filename.ends_with(".spx"))
		{
			return SPX;
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
			result = this->source->load(&this->data);
			if (result)
			{
				result = this->source->decode(this->data, &this->stream);
			}
		}
		if (result)
		{
			this->loaded = result;
		}
		return result;
	}

}
