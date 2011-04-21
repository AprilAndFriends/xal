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
/// Provides a buffer for audio data.

#ifndef XAL_BUFFER_H
#define XAL_BUFFER_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Category;
	class Source;

	class xalExport Buffer
	{
	public:
		Buffer(chstr filename);
		virtual ~Buffer();

		chstr getFilename() { return this->filename; }
		unsigned char* getStream() { return this->stream; }
		Source* getSource() { return this->source; }

		unsigned int getSize();
		unsigned int getChannels();
		int getSamplingRate();
		int getBitsPerSample();
		float getDuration();
		Format getFormat();

		bool load();
		
	protected:
		hstr filename;
		unsigned int fileSize;
		bool loaded;
		unsigned char* data;
		unsigned char* stream;
		Source* source;

	};

}

#endif
