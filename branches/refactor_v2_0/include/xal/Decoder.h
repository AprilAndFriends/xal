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
/// Provides an interface for decoders.

#ifndef XAL_DECODER_H
#define XAL_DECODER_H

#include "xalExport.h"

namespace xal
{
	class xalExport Decoder
	{
	public:
		Decoder();
		virtual ~Decoder();

		virtual bool decode(unsigned char* input, unsigned char** output) { return false; }
		
	};

}

#endif
