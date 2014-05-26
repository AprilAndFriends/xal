/// @file
/// @version 3.11
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

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

	void Category::setGain(float value)
	{
		xal::mgr->_lock();
		this->gain = value;
		xal::mgr->_setGlobalGain(xal::mgr->globalGain); // updates all Player instances with the new category gain
		xal::mgr->_unlock();
	}

	bool Category::isStreamed()
	{
		return (this->bufferMode == STREAMED);
	}

	bool Category::isMemoryManaged()
	{
		return (this->bufferMode == MANAGED);
	}

}
