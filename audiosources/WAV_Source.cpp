/// @file
/// @author  Boris Mikic
/// @version 2.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_WAV
#include <string.h>

#include <hltypes/hresource.h>

#include "AudioManager.h"
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

	bool WAV_Source::open()
	{
		this->streamOpen = Source::open();
		if (!this->streamOpen)
		{
			return false;
		}
		this->file.open(this->filename);
		unsigned char buffer[5] = {0};
		this->file.read_raw(buffer, 4); // RIFF
		this->file.read_raw(buffer, 4); // file size
		this->file.read_raw(buffer, 4); // WAVE
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
			XAL_NORMALIZE_ENDIAN(*(uint32_t*)buffer);
#endif
			memcpy(&size, buffer, 4);
			if (tag == "fmt ")
			{
				/// TODO - implement hresource::read_little_endian and hresource::read_big_endian
				// format
				file.read_raw(buffer, 2);
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
				XAL_NORMALIZE_ENDIAN(*(uint16_t*)buffer);
#endif
				memcpy(&value16, buffer, 2);
				if (size == 16 && value16 == 1)
				{
					// channels
					file.read_raw(buffer, 2);
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
					XAL_NORMALIZE_ENDIAN(*(uint16_t*)buffer);
#endif
					memcpy(&value16, buffer, 2);
					this->channels = value16;
					// sampling rate
					file.read_raw(buffer, 4);
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
					XAL_NORMALIZE_ENDIAN(*(uint32_t*)buffer);
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
					XAL_NORMALIZE_ENDIAN(*(uint16_t*)buffer);
#endif
					memcpy(&value16, buffer, 2);
					this->bitsPerSample = value16;
					size = 0;
				}
				else // not PCM, some form of compressed format
				{
					size -= 2;
					this->streamOpen = false;
					this->file.close();
					break;
				}
			}
			else if (tag == "data")
			{
				this->size += size;
			}
			if (size > 0)
			{
				this->file.seek(size);
			}
		}
		this->duration = (float)this->size / (this->samplingRate * this->channels * this->bitsPerSample / 8);
		this->_findData();
		return this->streamOpen;
	}

	void WAV_Source::close()
	{
		if (this->streamOpen)
		{
			this->file.close();
		}
	}

	void WAV_Source::rewind()
	{
		if (this->streamOpen)
		{
			this->_findData();
		}
	}

	void WAV_Source::_findData()
	{
		this->file.open(this->filename);
		unsigned char buffer[5] = {0};
		this->file.read_raw(buffer, 4); // RIFF
		this->file.read_raw(buffer, 4); // file size
		this->file.read_raw(buffer, 4); // WAVE
		hstr tag;
		int size = 0;
		while (!file.eof())
		{
			file.read_raw(buffer, 4); // next tag
			tag = (char*)buffer;
			file.read_raw(buffer, 4); // size of the chunk
#ifdef __BIG_ENDIAN__ // TODO - this should be tested properly
			XAL_NORMALIZE_ENDIAN(*(uint32_t*)buffer);
#endif
			memcpy(&size, buffer, 4);
			if (tag == "data")
			{
				break;
			}
			if (size > 0)
			{
				this->file.seek(size);
			}
		}
	}

	bool WAV_Source::load(unsigned char* output)
	{
		if (!Source::load(output))
		{
			return false;
		}
		this->file.read_raw(output, this->size);
		return true;
	}

	int WAV_Source::loadChunk(unsigned char* output, int size)
	{
		if (Source::loadChunk(output, size) == 0)
		{
			return 0;
		}
		return this->file.read_raw(output, size);
	}

}
#endif
