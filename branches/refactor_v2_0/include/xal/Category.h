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
/// Represents an audio category.

#ifndef XAL_CATEGORY_H
#define XAL_CATEGORY_H

#include <hltypes/hstring.h>
#include "xalExport.h"

namespace xal
{
	class xalExport Category
	{
	public:
		Category(chstr name, bool streamed = false, bool dynamicLoad = false);
		~Category();
		
		hstr getName() { return this->name; }
		bool isStreamed() { return this->streamed; }
		bool isDynamicLoad() { return this->dynamicLoad; }
		float getGain() { return this->gain; }
		void setGain(float value) { this->gain = value; }
		
	protected:
		hstr name;
		bool streamed;
		bool dynamicLoad;
		float gain;
		
	};

}
#endif
