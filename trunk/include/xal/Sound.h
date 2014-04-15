/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents a virtual entry of audio data in the sound system.

#ifndef XAL_SOUND_H
#define XAL_SOUND_H

#include <hltypes/hltypesUtil.h>
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
		~Sound();

		HL_DEFINE_GET(hstr, name, Name);
		HL_DEFINE_GET(hstr, filename, Filename);
		HL_DEFINE_GET(hstr, realFilename, RealFilename);
		HL_DEFINE_GET(Category*, category, Category);
		HL_DEFINE_GET(Buffer*, buffer, Buffer);

		int getSize();
		int getChannels();
		int getSamplingRate();
		int getBitsPerSample();
		float getDuration();
		Format getFormat();
		bool isStreamed();

		int readPcmData(unsigned char** output);

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
