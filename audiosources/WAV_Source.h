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
/// Provides a source for WAV format.

#if HAVE_WAV
#ifndef XAL_WAV_SOURCE_H
#define XAL_WAV_SOURCE_H

#include <hltypes/hstring.h>

#include "Source.h"
#include "xalExport.h"

namespace xal
{
	class xalExport WAV_Source : public Source
	{
	public:
		WAV_Source(chstr filename);
		~WAV_Source();

		bool load(unsigned char** output);

	};

}

#endif
#endif
