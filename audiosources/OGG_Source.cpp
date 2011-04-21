/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_OGG
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "OGG_Source.h"
#include "xal.h"

namespace xal
{
	OGG_Source::OGG_Source(chstr filename) : Source(filename)
	{
	}

	OGG_Source::~OGG_Source()
	{
	}

	bool OGG_Source::load(unsigned char** output)
	{
		if (!Source::load(output))
		{
			return false;
		}
		OggVorbis_File oggStream;
		if (ov_fopen((char*)this->filename.c_str(), &oggStream) != 0)
		{
			xal::log("ogg: error opening file!");
			return false;
		}
		vorbis_info* info = ov_info(&oggStream, -1);
		this->channels = info->channels;
		this->samplingRate = info->rate;
		this->bitsPerSample = 16; // always 16 bit data
		int bytes = this->bitsPerSample / 8;
		this->size = (unsigned long)ov_pcm_total(&oggStream, -1) * this->channels * bytes;
		this->duration = ((float)this->size) / (this->samplingRate * this->channels * bytes);
		unsigned int remaining = this->size;
		*output = new unsigned char[this->size];
		bool result = false;
		if (*output != NULL)
		{
			int section;
			unsigned long size = remaining;
			unsigned char* buffer = *output;
			int read;
			while (size > 0)
			{
				read = ov_read(&oggStream, (char*)buffer, size, 0, 2, 1, &section);
				if (read == 0)
				{
					remaining -= size;
					break;
				}
				size -= read;
				buffer += read;
			}
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
			for (int i = 0; i < this->size; i += bytes)
			{
				XAL_NORMALIZE_ENDIAN((uint16_t)((*output)[i])); // always 16 bit data
			}
#endif	
			result = true;
		}
		else
		{
			xal::log("ogg: could not allocate ogg buffer.");
		}
		ov_clear(&oggStream);
		return result;
	}

}
#endif
