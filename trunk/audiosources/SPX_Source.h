/// @file
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Provides a source for OGG format.

#ifdef _FORMAT_SPX
#ifndef XAL_SPX_SOURCE_H
#define XAL_SPX_SOURCE_H

#include <hltypes/hstring.h>

#include "Source.h"
#include "xalExport.h"

namespace xal
{
	class Category;

	class xalExport SPX_Source : public Source
	{
	public:
		SPX_Source(chstr filename, Category* category);
		~SPX_Source();

		bool load(unsigned char* output);

	};

}

#endif
#endif
