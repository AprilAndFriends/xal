/// @file
/// @author  Boris Mikic
/// @version 2.63
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_OGG
#include <stdio.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstream.h>

#include "Endianess.h"
#include "AudioManager.h"
#include "OGG_Source.h"
#include "xal.h"

namespace xal
{
	static int section; // a small optimization

	size_t _dataRead(void* data, size_t size, size_t count, void* dataSource)
	{
		hsbase* stream = (hsbase*)dataSource;
		return stream->read_raw(data, size * count);
	}

	int _dataSeek(void* dataSource, ogg_int64_t offset, int whence)
	{
		hsbase::SeekMode mode = hsbase::CURRENT;
		switch (whence)
		{
		case SEEK_CUR:
			mode = hsbase::CURRENT;
			break;
		case SEEK_SET:
			mode = hsbase::START;
			break;
		case SEEK_END:
			mode = hsbase::END;
			break;
		}
		((hsbase*)dataSource)->seek((long)offset, mode);
		return 0;
	}

	long _dataTell(void* dataSource)
	{
		return ((hsbase*)dataSource)->position();
	}

	OGG_Source::OGG_Source(chstr filename, Category* category) : Source(filename, category)
	{
	}

	OGG_Source::~OGG_Source()
	{
		this->close();
	}

	bool OGG_Source::open()
	{
		this->streamOpen = Source::open();
		if (!this->streamOpen)
		{
			return false;
		}
		// setting the special callbacks
		ov_callbacks callbacks;
		callbacks.read_func = &_dataRead;
		callbacks.seek_func = &_dataSeek;
		callbacks.close_func = NULL;
		callbacks.tell_func = &_dataTell;
		if (ov_open_callbacks((void*)this->stream, &this->oggStream, NULL, 0, callbacks) == 0)
		{
			vorbis_info* info = ov_info(&this->oggStream, -1);
			this->channels = info->channels;
			this->samplingRate = info->rate;
			this->bitsPerSample = 16; // always 16 bit data
			int bytes = this->bitsPerSample / 8;
			this->size = (int)ov_pcm_total(&this->oggStream, -1) * this->channels * bytes;
			this->duration = (float)this->size / (this->samplingRate * this->channels * bytes);
		}
		else
		{
			xal::log("ogg: error reading data!");
			this->close();
		}
		return this->streamOpen;
	}

	void OGG_Source::rewind()
	{
		if (this->streamOpen)
		{
			ov_pcm_seek(&this->oggStream, 0);
		}
	}

	bool OGG_Source::load(unsigned char* output)
	{
		if (!Source::load(output))
		{
			return false;
		}
		unsigned long remaining = this->size;
		char* buffer = (char*)output;
		int read;
		while (remaining > 0)
		{
			read = ov_read(&this->oggStream, buffer, remaining, 0, 2, 1, &section);
			if (read == 0)
			{
				break;
			}
			remaining -= read;
			buffer += read;
		}
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
		for_iter_step (i, 0, this->size, 2)
		{
			XAL_NORMALIZE_ENDIAN(*(uint16_t*)(output + i)); // always 16 bit data
		}
#endif	
		return true;
	}

	int OGG_Source::loadChunk(unsigned char* output, int size)
	{
		if (Source::loadChunk(output, size) == 0)
		{
			return 0;
		}
		int remaining = size;
		char* buffer = (char*)output;
		int read;
		while (remaining > 0)
		{
			read = ov_read(&this->oggStream, buffer, remaining, 0, 2, 1, &section);
			if (read == 0)
			{
				break;
			}
			remaining -= read;
			buffer += read;
		}
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
		for_iter_step (i, 0, this->size, 2)
		{
			XAL_NORMALIZE_ENDIAN(*(uint16_t*)(output + i)); // always 16 bit data
		}
#endif	
		return (size - remaining);
	}

}
#endif
