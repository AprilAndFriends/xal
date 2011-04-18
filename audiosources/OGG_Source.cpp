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
		xal::log("loading ogg sound " + this->filename);
		*output = NULL;
		OggVorbis_File oggStream;
		if (ov_fopen((char*)this->filename.c_str(), &oggStream) != 0)
		{
			xal::log("ogg: error opening file!");
			return false;
		}
		ov_clear(&oggStream);
		return true;
	}

	bool OGG_Source::decode(unsigned char* input, unsigned char** output)
	{
#if HAVE_OGG
		xal::log("decoding ogg sound " + this->filename);
		OggVorbis_File oggStream;
		if (ov_fopen((char*)this->filename.c_str(), &oggStream) != 0)
		{
			xal::log("ogg: error opening file!");
			return false;
		}
		//alGenBuffers(1, &this->buffer);
		vorbis_info* info = ov_info(&oggStream, -1);
		this->channels = info->channels;
		this->rate = info->rate;
		this->size = (unsigned long)ov_pcm_total(&oggStream, -1) * this->channels * 2; // always 16 bit data
		this->duration = ((float)this->size) / (this->rate * this->channels * 2);
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

#ifdef __BIG_ENDIAN__
			for (uint16_t* p = (uint16_t*)data; (unsigned char*)p < buffer; p++)
			{
				XAL_NORMALIZE_ENDIAN(*p);
			}
#endif	
			//alBufferData(this->buffer, (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data, length, info->rate);
			//delete [] data;
			result = true;
		}
		else
		{
			xal::log("ogg: could not allocate ogg buffer.");
		}
		ov_clear(&oggStream);
		return result;
#else
#warning HAVE_OGG is not defined to 1. No OGG support.
		xal::log("no ogg support built in, cannot load " + this->filename);
		return false;
#endif
	}

}
#endif
