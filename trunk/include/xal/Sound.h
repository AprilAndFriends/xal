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
/// Represents a virtual entry of audio data in the sound system.

#ifndef XAL_SOUND_H
#define XAL_SOUND_H

#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Buffer;
	class Category;

	class xalExport Sound
	{
	public:
		Sound(chstr filename, Category* category, chstr prefix = "");
		virtual ~Sound();

		chstr getName() { return this->name; }
		chstr getFilename() { return this->filename; }
		chstr getRealFilename() { return this->realFilename; }
		Category* getCategory() { return this->category; }
		Buffer* getBuffer() { return this->buffer; }
		bool isStreamed();
		Format getFormat();

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
