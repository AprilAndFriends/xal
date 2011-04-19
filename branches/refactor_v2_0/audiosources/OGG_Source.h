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
/// Provides a source for OGG format.

#if HAVE_OGG
#ifndef XAL_OGG_SOURCE_H
#define XAL_OGG_SOURCE_H

#include <hltypes/hstring.h>

#include "Source.h"
#include "xalExport.h"

namespace xal
{
	class xalExport OGG_Source : public Source
	{
	public:
		OGG_Source(chstr filename);
		~OGG_Source();

		bool load(unsigned char** output);
		bool decode(unsigned char* input, unsigned char** output, int size);
		bool decode(unsigned char* input, unsigned char** output);

	};

}

#endif
#endif
