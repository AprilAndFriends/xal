/// @file
/// @author  Boris Mikic
/// @version 2.61
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
	Category::Category(chstr name, BufferMode bufferMode, SourceMode sourceMode) : gain(1.0f)
	{
		this->name = name;
		this->bufferMode = bufferMode;
		this->sourceMode = sourceMode;
	}

	Category::~Category()
	{
	}

	bool Category::isStreamed()
	{
		return (this->bufferMode == STREAMED);
	}

	bool Category::isMemoryManaged()
	{
		return (this->bufferMode == MANAGED || this->bufferMode == LAZY_MANAGED);
	}

}
