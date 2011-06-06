/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Provides a source for WAMP3V format.

#if HAVE_MP3
#ifndef XAL_MP3_SOURCE_H
#define XAL_MP3_SOURCE_H

#include <hltypes/hstring.h>

#include "Source.h"
#include "xalExport.h"

namespace xal
{
	class xalExport MP3_Source : public Source
	{
	public:
		MP3_Source(chstr filename);
		~MP3_Source();

		bool load(unsigned char* output);

	};

}

#endif
#endif
