/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "Decoder.h"
#include "xal.h"

namespace xal
{
	Decoder::Decoder(chstr filename) : size(0), channels(0), rate(0), duration(0.0f)
	{
		this->filename = filename;
	}

	Decoder::~Decoder()
	{
	}
	
	bool Decoder::load(unsigned char** output)
	{
		xal::log("loading dummy file " + this->filename);
		return true;
	}
	
	bool Decoder::decode(unsigned char* input, unsigned char** output)
	{
		xal::log("decoding dummy file " + this->filename);
		return true;
	}
	
}
