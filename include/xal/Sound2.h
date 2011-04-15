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

#ifndef XAL_SOUND_H
#define XAL_SOUND_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Category;

	class xalExport Sound2
	{
	public:
		Sound2(chstr filename, Category* category, chstr prefix = "");
		virtual ~Sound2();

		chstr getName() { return this->name; }
		chstr getFilename() { return this->filename; }
		chstr getRealFilename() { return this->realFilename; }
		Category* getCategory() { return this->category; }
		Buffer* getBuffer() { return this->buffer; }
		Format getFormat();

		bool load();
		
	protected:
		hstr name;
		hstr filename;
		hstr realFilename;
		Category* category;
		Buffer* buffer;

		hstr _findLinkedFile();
		
	};

}

#endif
