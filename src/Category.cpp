/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hstring.h>

#include "Category.h"

namespace xal
{
	Category::Category(chstr name, bool streamed, bool dynamicLoad) : gain(1.0f)
	{
		this->name = name;
		this->streamed = streamed;
		this->dynamicLoad = dynamicLoad;
	}

	Category::~Category()
	{
	}

}
