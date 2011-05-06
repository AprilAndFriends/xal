/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Category.h"

namespace xal
{
	Category::Category(chstr name, HandlingMode loadMode, HandlingMode decodeMode) : gain(1.0f)
	{
		this->name = name;
		// allowed
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
		this->loadMode = loadMode;
		this->decodeMode = decodeMode;
		// TODO - leave it like this?
		if (this->decodeMode < this->loadMode)
		{
			throw ("cannot create category " + this->name + ", combination of load-mode and decode-mode invalid").c_str();
		}
	}

	Category::~Category()
	{
	}

	bool Category::isStreamed()
	{
		return (this->loadMode == STREAMED || this->decodeMode == STREAMED);
	}

}
