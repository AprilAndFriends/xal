/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if HAVE_SDL
#include <SDL/SDL.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "Source.h"
#include "xal.h"

#if HAVE_SDL
#include "SDL_AudioManager.h"
#endif

namespace xal
{
	Buffer::Buffer(chstr filename, HandlingMode loadMode, HandlingMode decodeMode) :
		loaded(false), decoded(false)
	{
		this->filename = filename;
		this->fileSize = hfile::hsize(this->filename);
		this->loadMode = loadMode;
		this->decodeMode = decodeMode;
		this->source = xal::mgr->_createSource(this->filename, this->getFormat());
		this->streamSize = 0;
		this->stream = NULL;
		if (xal::mgr->isEnabled())
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
#if HAVE_M4A
		if (this->filename.ends_with(".m4a"))
		{
			return M4A;
		}
#endif
#if HAVE_MP3
		if (this->filename.ends_with(".mp3"))
		{
			return MP3;
		}
#endif
#if HAVE_OGG
		if (this->filename.ends_with(".ogg"))
		{
			return OGG;
		}
#endif
#if HAVE_SPX
		if (this->filename.ends_with(".spx"))
		{
			return SPX;
		}
#endif
#if HAVE_WAV
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
#if HAVE_SDL
			if (xal::mgr->getName() == XAL_AS_SDL)
			{
				this->_convertStream();
			}
#endif
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
			while (looping && this->streamSize < size)
			{
				this->source->rewind();
				size -= this->streamSize;
				this->streamSize += this->source->loadChunk(&this->stream[this->streamSize], size);
			}
#if HAVE_SDL
			if (xal::mgr->getName() == XAL_AS_SDL)
			{
				this->_convertStream();
			}
#endif
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

#if HAVE_SDL
	void Buffer::_convertStream()
	{
		SDL_AudioSpec format = ((SDL_AudioManager*)xal::mgr)->getFormat();
		int srcFormat = (this->getBitsPerSample() == 16 ? AUDIO_S16 : AUDIO_S8);
		int srcChannels = this->getChannels();
		int srcSamplingRate = this->getSamplingRate();
		if (srcFormat != format.format || srcChannels != format.channels || srcSamplingRate != format.freq)
		{
			SDL_AudioCVT cvt;
			int result = SDL_BuildAudioCVT(&cvt, srcFormat, srcChannels, srcSamplingRate, format.format, format.channels, format.freq);
			if (result == -1)
			{
				xal::log("ERROR: Could not build converter " + this->filename);
				return;
			}
			cvt.buf = (Uint8*)(new unsigned char[this->streamSize * cvt.len_mult]); // making sure the conversion buffer is large enough
			memcpy(cvt.buf, this->stream, this->streamSize * sizeof(unsigned char));
			cvt.len = this->streamSize;
			result = SDL_ConvertAudio(&cvt);
			if (result == -1)
			{
				xal::log("ERROR: Could not convert audio " + this->filename);
				return;
			}
			int newSize = hround(cvt.len * cvt.len_ratio);
			if (this->streamSize != newSize) // stream has to be resized
			{
				this->streamSize = newSize;
				delete this->stream;
				this->stream = new unsigned char[this->streamSize];
			}
			memcpy(this->stream, cvt.buf, this->streamSize * sizeof(unsigned char));
			delete [] cvt.buf;
		}
	}
#endif

}
