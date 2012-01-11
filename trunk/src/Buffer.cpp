/// @file
/// @author  Boris Mikic
/// @version 2.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "Source.h"

namespace xal
{
	Buffer::Buffer(chstr filename, HandlingMode loadMode, HandlingMode decodeMode) :
		loaded(false), decoded(false)
	{
		this->filename = filename;
		this->fileSize = hfile::hsize(this->filename);
		this->loadMode = loadMode;
		this->decodeMode = decodeMode;
		this->streamSize = 0;
		this->stream = NULL;
		this->source = xal::mgr->_createSource(this->filename, this->getFormat());
		if (xal::mgr->isEnabled() && this->getFormat() != UNKNOWN)
		{
			switch (this->loadMode)
			{
			case FULL:
				this->prepare();
				break;
			case LAZY:
				break;
			case ON_DEMAND:
				break;
			case STREAMED:
				this->streamSize = STREAM_BUFFER_COUNT * STREAM_BUFFER_SIZE;
				this->stream = new unsigned char[this->streamSize];
				break;
			}
		}
	}

	Buffer::~Buffer()
	{
		if (this->stream != NULL)
		{
			delete [] this->stream;
		}
		delete this->source;
	}
	
	int Buffer::getSize()
	{
		this->prepare();
		return this->source->getSize();
	}

	int Buffer::getChannels()
	{
		this->prepare();
		return this->source->getChannels();
	}

	int Buffer::getSamplingRate()
	{
		this->prepare();
		return this->source->getSamplingRate();
	}

	int Buffer::getBitsPerSample()
	{
		this->prepare();
		return this->source->getBitsPerSample();
	}

	float Buffer::getDuration()
	{
		this->prepare();
		return this->source->getDuration();
	}

	Format Buffer::getFormat()
	{
#ifdef HAVE_FLAC
		if (this->filename.ends_with(".flac"))
		{
			return FLAC;
		}
#endif
#ifdef HAVE_M4A
		if (this->filename.ends_with(".m4a"))
		{
			return M4A;
		}
#endif
#ifdef HAVE_MIDI
		if (this->filename.ends_with(".mid"))
		{
			return MIDI;
		}
#endif
#ifdef HAVE_OGG
		if (this->filename.ends_with(".ogg"))
		{
			return OGG;
		}
#endif
#ifdef HAVE_SPX
		if (this->filename.ends_with(".spx"))
		{
			return SPX;
		}
#endif
#ifdef HAVE_WAV
		if (this->filename.ends_with(".wav"))
		{
			return WAV;
		}
#endif
		return UNKNOWN;
	}

	bool Buffer::isStreamed()
	{
		return (this->loadMode == STREAMED || this->decodeMode == STREAMED);
	}

	void Buffer::prepare()
	{
		if (this->loaded)
		{
			return;
		}
		if (!xal::mgr->isEnabled())
		{
			this->loaded = true;
			return;
		}
		if (!this->isStreamed())
		{
			this->loaded = true;
			this->source->open();
			if (this->stream == NULL)
			{
				this->streamSize = this->source->getSize();
				this->stream = new unsigned char[this->streamSize];
			}
			this->source->load(this->stream);
			xal::mgr->_convertStream(this, &this->stream, &this->streamSize);
			return;
		}
		if (!this->source->isOpen())
		{
			this->source->open();
		}
	}

	int Buffer::load(bool looping, int size)
	{
		if (!xal::mgr->isEnabled())
		{
			return 0;
		}
		if (this->isStreamed() && this->source->isOpen())
		{
			this->streamSize = this->source->loadChunk(this->stream, size);
			size -= this->streamSize;
			if (size > 0)
			{
				if (!looping)
				{
					// fill rest of buffer with silence so systems that depends on buffered chunks don't get messed up
					memset(&this->stream[this->streamSize], 0, size * sizeof(unsigned char));
				}
				else
				{
					int read = 0;
					while (size > 0)
					{
						this->source->rewind();
						read = this->source->loadChunk(&this->stream[this->streamSize], size);
						size -= read;
						this->streamSize += read;
					}
				}
			}
			xal::mgr->_convertStream(this, &this->stream, &this->streamSize);
		}
		return this->streamSize;
	}

	void Buffer::release()
	{
		if (this->decodeMode == xal::ON_DEMAND)
		{
			if (this->stream != NULL)
			{
				delete [] this->stream;
				this->stream = NULL;
			}
		}
		if (this->loadMode == xal::ON_DEMAND)
		{
			this->source->close();
			this->loaded = false;
		}
	}

	void Buffer::free()
	{
		if (this->stream != NULL)
		{
			delete [] this->stream;
			this->stream = NULL;
		}
		this->source->close();
		this->loaded = false;
	}

	void Buffer::rewind()
	{
		this->source->rewind();
	}


}
