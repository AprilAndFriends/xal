/// @file
/// @author  Boris Mikic
/// @version 2.0
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
#include "Sound.h"
#include "xal.h"

namespace xal
{
	Sound::Sound(chstr filename, Category* category, chstr prefix)
	{
		this->filename = filename;
		this->realFilename = this->_findLinkedFile();
		this->category = category;
		this->buffer = xal::mgr->_createBuffer(this->realFilename, category->getLoadMode(), category->getDecodeMode());
		// extracting filename without extension and prepending the prefix
		this->name = prefix + filename.replace("\\", "/").rsplit("/", -1, false).pop_back().rsplit(".", 1, false).pop_front();
	}

	Sound::~Sound()
	{
		delete this->buffer;
	}
	
	hstr Sound::_findLinkedFile()
	{
		if (!this->filename.ends_with(".xln"))
		{
			return this->filename;
		}
		if (!hfile::exists(this->filename))
		{
			return this->filename;
		}
		harray<hstr> newFolders = hfile::hread(this->filename).split("/", -1, true);
		harray<hstr> folders = this->filename.split("/", -1, true);
		folders.pop_back();
		foreach (hstr, it, newFolders)
		{
			if ((*it) != "..")
			{
				folders += (*it);
			}
			else
			{
				folders.pop_back();
			}
		}
		return folders.join("/");
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
		return this->category->isStreamed();
	}

}
