/// @file
/// @author  Boris Mikic
/// @version 3.11
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents an audio category.

#ifndef XAL_CATEGORY_H
#define XAL_CATEGORY_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class xalExport Category
	{
	public:
		Category(chstr name, BufferMode bufferMode, SourceMode sourceMode);
		~Category();
		
		HL_DEFINE_GET(hstr, name, Name);
		HL_DEFINE_GET(float, gain, Gain);
		void setGain(float value);
		HL_DEFINE_GET(BufferMode, bufferMode, BufferMode);
		HL_DEFINE_GET(SourceMode, sourceMode, SourceMode);
		bool isStreamed();
		bool isMemoryManaged();
		
	protected:
		hstr name;
		float gain;
		BufferMode bufferMode;
		SourceMode sourceMode;
		
	};

}
#endif
