/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_WAV
#include <string.h>

#include <hltypes/hfile.h>

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
		bool result = Source::open();
		if (result)
		{
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
						result = false;
					}
				}
				else if (tag == "data")
				{
					this->size = size;
				}
				if (size > 0)
				{
					file.seek(size);
				}
			}
		}
		this->_findData();
		return result;
	}

	bool WAV_Source::close()
	{
		bool result = Source::close();
		if (result)
		{
			this->file.close();
		}
		return result;
	}

	bool WAV_Source::rewind()
	{
		bool result = Source::rewind();
		if (result)
		{
			this->_findData();
		}
		return result;
	}

	void WAV_Source::_findData()
	{
		this->file.seek(0, hfile::START);
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
			XAL_NORMALIZE_ENDIAN((uint32_t)*buffer);
#endif
			memcpy(&size, buffer, 4);
			if (tag == "data")
			{
				//this->file.seek(-8);
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

	int WAV_Source::loadChunk(unsigned char* output, int count)
	{
		if (Source::loadChunk(output, count) == 0)
		{
			return 0;
		}
		return this->file.read_raw(output, count * STREAM_BUFFER_SIZE);
	}

}
#endif
