/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
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
