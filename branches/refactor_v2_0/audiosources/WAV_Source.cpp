/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_WAV
#include "WAV_Source.h"
#include "xal.h"

namespace xal
{
	WAV_Source::WAV_Source(chstr filename) : Source(filename)
	{
	}

	WAV_Source::~WAV_Source()
	{
	}

	bool WAV_Source::load(unsigned char** output)
	{
		xal::log("loading wav sound " + this->filename);
		return true;
	}

}
#endif
