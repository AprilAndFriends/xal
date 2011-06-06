/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_MP3
#include "MP3_Source.h"
#include "xal.h"

namespace xal
{
	MP3_Source::MP3_Source(chstr filename) : Source(filename)
	{
	}

	MP3_Source::~MP3_Source()
	{
	}

	bool MP3_Source::load(unsigned char* output)
	{
		if (!Source::load(output, size))
		{
			return false;
		}
		return true;
	}

}
#endif
