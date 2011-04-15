/// @file
/// @author  Kresimir Spes
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
/// Provides a buffer for audio data.

#ifndef XAL_BUFFER_H
#define XAL_BUFFER_H

#include <hltypes/hstring.h>

#include "xalExport.h"

namespace xal
{
	class Category;

	class xalExport Buffer
	{
	public:
		Buffer(chstr filename);
		virtual ~Buffer();

		chstr getFilename() { return this->filename; }
		float getDuration() { return this->duration; }

	protected:
		hstr filename;
		float duration;

	};

}

#endif
