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
	Buffer::Buffer(chstr filename) : duration(0.0f), loaded(false)
	{
		this->filename = filename;
		this->decoder = NULL;
		Format format = this->getFormat();
		switch (format)
		{
#if HAVE_OGG
		case OGG:
			this->decoder = new OGG_Decoder();
			break;
#endif
#if HAVE_M4A
		case M4A:
			this->decoder = new M4A_Decoder();
			break;
#endif
		}
		if (this->decoder == NULL)
		{
			this->decoder = new Decoder();
		}
	}

	Buffer::~Buffer()
	{
		xal::log("destroying buffer " + this->filename);
		delete this->decoder;
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
		bool result = false;
		Format format = this->getFormat();
		if (!xal::mgr->isEnabled())
		{
			result = (format != UNKNOWN);
		}
		else
		{
			//result = this->decoder->decode();
			result = true;
		}
		if (result)
		{
			this->loaded = result;
		}
		return result;
	}
	
}
