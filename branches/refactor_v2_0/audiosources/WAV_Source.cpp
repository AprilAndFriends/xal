/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_WAV
#include <hltypes/hfile.h>

#include "WAV_Source.h"
#include "xal.h"

namespace xal
{
	WAV_Source::WAV_Source(chstr filename) : Source(filename)
	{
	}

	WAV_Source::~WAV_Source()
	{
	}

	bool WAV_Source::load(unsigned char** output)
	{
		if (!Source::load(output))
		{
			return false;
		}
		unsigned char buffer[5] = {0};
		hfile file(this->filename);
		file.read_raw(buffer, 4); // RIFF
		file.read_raw(buffer, 4); // file size
		file.read_raw(buffer, 4); // WAVE
		hstr tag;
		int size = 0;
		short value16;
		int value32;
		while (!file.eof())
		{
			file.read_raw(buffer, 4); // next tag
			tag = (char*)buffer;
			file.read_raw(buffer, 4); // size of the chunk
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
			XAL_NORMALIZE_ENDIAN((uint32_t)*buffer);
#endif
			memcpy(&size, buffer, 4);
            if (tag == "fmt ")
            {
				/// TODO - implement hfile::read_little_endian and hfile::read_big_endian
				// format
				file.read_raw(buffer, 2);
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
				XAL_NORMALIZE_ENDIAN((uint16_t)*buffer);
#endif
				memcpy(&value16, buffer, 2);
				if (size == 16 && value16 == 1)
				{
					// channels
					file.read_raw(buffer, 2);
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
					XAL_NORMALIZE_ENDIAN((uint16_t)*buffer);
#endif
					memcpy(&value16, buffer, 2);
					this->channels = value16;
					// sampling rate
					file.read_raw(buffer, 4);
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
					XAL_NORMALIZE_ENDIAN((uint32_t)*buffer);
#endif
					memcpy(&value32, buffer, 4);
					this->samplingRate = value32;
					// bytes rate
					file.read_raw(buffer, 4);
					// blockalign
					file.read_raw(buffer, 2);
					// bits per sample
					file.read_raw(buffer, 2);
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
					XAL_NORMALIZE_ENDIAN((uint16_t)*buffer);
#endif
					memcpy(&value16, buffer, 2);
					this->bitsPerSample = value16;
					size = 0;
				}
				else // not PCM, some form of compressed format
				{
					size -= 2;
				}
            }
            else if (tag == "data")
            {
				this->size = size;
				*output = new unsigned char[this->size];
				file.read_raw(*output, this->size);
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
				for (int i = 0; i < size; i += 2) // always 16 bit
				{
					XAL_NORMALIZE_ENDIAN((uint16_t)((*output)[i]));
				}
#endif
				size = 0;
            }
			if (size > 0)
			{
				file.seek(size);
			}
		}
		if (this->size == 0)
		{
			return false;
		}
		return true;
	}

}
#endif
