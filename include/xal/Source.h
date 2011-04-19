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
/// Provides an interface for audio sources.

#ifndef XAL_DECODER_H
#define XAL_DECODER_H

#include <hltypes/hstring.h>

#include "xalExport.h"

namespace xal
{
	class xalExport Source
	{
	public:
		Source(chstr filename);
		virtual ~Source();

		int getSize() { return this->size; }
		int getChannels() { return this->channels; }
		int getRate() { return this->rate; }
		float getDuration() { return this->duration; }

		virtual bool load(unsigned char** output);
		virtual bool decode(unsigned char* input, unsigned char** output);
		
	protected:
		hstr filename;
		int size;
		int channels;
		int rate;
		float duration;
		bool streamed;
		int chunkSize;

	};

}

#endif
