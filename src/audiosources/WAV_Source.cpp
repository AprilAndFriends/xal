/// @file
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _FORMAT_WAV
#include <string.h>

#include <hltypes/hresource.h>

#include "AudioManager.h"
#include "WAV_Source.h"
#include "xal.h"

namespace xal
{
	WAV_Source::WAV_Source(chstr filename, SourceMode sourceMode, BufferMode bufferMode) : Source(filename, sourceMode, bufferMode)
	{
	}

	WAV_Source::~WAV_Source()
	{
		this->close();
	}

	bool WAV_Source::open()
	{
		if (!this->streamOpen)
		{
			return false;
		}
		unsigned char buffer[5] = {0};
		this->stream->readRaw(buffer, 4); // RIFF
		this->stream->readRaw(buffer, 4); // file size
		this->stream->readRaw(buffer, 4); // WAVE
		hstr tag;
		int size = 0;
		short value16;
		int value32;
		while (!this->stream->eof())
		{
			this->stream->readRaw(buffer, 4); // next tag
			tag = (char*)buffer;
			this->stream->readRaw(buffer, 4); // size of the chunk
			memcpy(&size, buffer, 4);
			if (tag == "fmt ")
			{
				// format
				this->stream->readRaw(buffer, 2);
				memcpy(&value16, buffer, 2);
				if (size == 16 && value16 == 1)
				{
					// channels
					this->stream->readRaw(buffer, 2);
					memcpy(&value16, buffer, 2);
					this->channels = value16;
					// sampling rate
					this->stream->readRaw(buffer, 4);
					memcpy(&value32, buffer, 4);
					this->samplingRate = value32;
					// bytes rate
					this->stream->readRaw(buffer, 4);
					// blockalign
					this->stream->readRaw(buffer, 2);
					// bits per sample
					this->stream->readRaw(buffer, 2);
					memcpy(&value16, buffer, 2);
					this->bitsPerSample = value16;
					size = 0;
				}
				else // not PCM, some form of compressed format
				{
					size -= 2;
					this->close();
					break;
				}
			}
			else if (tag == "data")
			{
				this->size += size;
			}
			if (size > 0)
			{
				this->stream->seek(size);
			}
		}
		this->duration = (float)this->size / (this->samplingRate * this->channels * this->bitsPerSample / 8);
		this->_findData();
		return this->streamOpen;
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
		this->stream->rewind();
		unsigned char buffer[5] = {0};
		this->stream->readRaw(buffer, 4); // RIFF
		this->stream->readRaw(buffer, 4); // file size
		this->stream->readRaw(buffer, 4); // WAVE
		hstr tag;
		int size = 0;
		while (!this->stream->eof())
		{
			this->stream->readRaw(buffer, 4); // next tag
			tag = (char*)buffer;
			this->stream->readRaw(buffer, 4); // size of the chunk
			memcpy(&size, buffer, 4);
			if (tag == "data")
			{
				break;
			}
			if (size > 0)
			{
				this->stream->seek(size);
			}
		}
	}

	bool WAV_Source::load(hstream& output)
	{
		if (Source::load(output) == 0)
		{
			return false;
		}
		int written = output.writeRaw(this->stream, this->size);
		if (written > 0)
		{
			output.seek(-written);
			return true;
		}
		return false;
	}

	int WAV_Source::loadChunk(hstream& output, int size)
	{
		if (Source::loadChunk(output, size) == 0)
		{
			return 0;
		}
		int written = output.writeRaw(this->stream, size);
		if (written > 0)
		{
			output.seek(-written);
		}
		return written;
	}

}
#endif
