/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Provides a source for WAV format.

#ifdef _FORMAT_WAV
#ifndef XAL_WAV_SOURCE_H
#define XAL_WAV_SOURCE_H

#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Source.h"
#include "xalExport.h"

namespace xal
{
	class Category;

	class xalExport WAV_Source : public Source
	{
	public:
		WAV_Source(chstr filename, Category* category);
		~WAV_Source();

		bool open();
		void rewind();
		bool load(unsigned char* output);
		int loadChunk(unsigned char* output, int size = STREAM_BUFFER_SIZE);

	protected:
		void _findData();

	};

}

#endif
#endif
