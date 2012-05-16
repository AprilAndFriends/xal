/// @file
/// @author  Boris Mikic
/// @version 2.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/exception.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Category.h"

namespace xal
{
	Category::Category(chstr name, HandlingMode sourceMode, HandlingMode bufferMode, bool memoryManaged) : gain(1.0f)
	{
		this->name = name;
		// allowed (source, buffer)
		// FULL, FULL
		// FULL, LAZY
		// FULL, ON_DEMAND
		// FULL, STREAMED
		// LAZY, LAZY
		// LAZY, ON_DEMAND
		// LAZY, STREAMED
		// ON_DEMAND, ON_DEMAND
		// ON_DEMAND, STREAMED
		// STREAMED, STREAMED
		this->sourceMode = sourceMode;
		this->bufferMode = bufferMode;
		this->memoryManaged = memoryManaged;
		// TODO - this has to be refactored, throwing an exception in a constructor is horrible
		if (this->bufferMode < this->sourceMode)
		{
			throw hl_exception("cannot create category " + this->name + ", combination of load-mode and decode-mode invalid");
		}
	}

	Category::~Category()
	{
	}

	bool Category::isStreamed()
	{
		return (this->sourceMode == STREAMED || this->bufferMode == STREAMED);
	}

}
