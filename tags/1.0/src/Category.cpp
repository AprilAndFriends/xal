/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
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
