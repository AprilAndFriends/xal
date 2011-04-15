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

#include "Buffer.h"
#include "Category.h"
#include "xal.h"

namespace xal
{
	Buffer::Buffer(chstr filename) : duration(0.0f)
	{
		this->filename = filename;
	}

	Buffer::~Buffer()
	{
		xal::log("destroying buffer " + this->filename);
	}
	
}
