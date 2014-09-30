/// @file
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>
#include <hltypes/hrdir.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Buffer.h"
#include "Category.h"
#include "Sound.h"
#include "Source.h"
#include "xal.h"

namespace xal
{
	Sound::Sound(chstr filename, Category* category, chstr prefix)
	{
		this->filename = filename;
		this->realFilename = this->_findLinkedFile();
		this->category = category;
		this->buffer = xal::mgr->_createBuffer(this);
		// extracting filename without extension and prepending the prefix
		this->name = prefix + hresource::no_extension(hrdir::basename(filename));
	}

	Sound::~Sound()
	{
		xal::mgr->_destroyBuffer(this->buffer);
	}
	
	hstr Sound::_findLinkedFile()
	{
		if (!this->filename.ends_with(".xln"))
		{
			return this->filename;
		}
		if (!hresource::exists(this->filename))
		{
			return this->filename;
		}
		// It's dangerous to go alone! Take this.
		return xal::mgr->findAudioFile(hrdir::normalize(hrdir::join_path(hrdir::basedir(this->filename), hresource::hread(this->filename), false)));
	}

	int Sound::getSize()
	{
		return this->buffer->getSize();
	}

	int Sound::getChannels()
	{
		return this->buffer->getChannels();
	}

	int Sound::getSamplingRate()
	{
		return this->buffer->getSamplingRate();
	}

	int Sound::getBitsPerSample()
	{
		return this->buffer->getBitsPerSample();
	}

	float Sound::getDuration()
	{
		return this->buffer->getDuration();
	}

	Format Sound::getFormat()
	{
		return this->buffer->getFormat();
	}

	bool Sound::isStreamed()
	{
		return this->buffer->isStreamed();
	}

	int Sound::readPcmData(unsigned char** output)
	{
		return Buffer(this).readPcmData(output);
	}

}
