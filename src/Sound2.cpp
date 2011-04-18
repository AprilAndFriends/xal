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
#include "Sound2.h"
#include "xal.h"

namespace xal
{
	Sound2::Sound2(chstr filename, Category* category, chstr prefix) : buffer(NULL)
	{
		this->filename = filename;
		this->realFilename = this->_findLinkedFile();
		this->category = category;
		this->buffer = new Buffer(this->realFilename);
		// extracting filename without extension and prepending the prefix
		this->name = prefix + hstr(filename).replace("\\", "/").rsplit("/").pop_back().rsplit(".", 1).pop_front();
	}

	Sound2::~Sound2()
	{
		xal::log("destroying sound " + this->name);
		delete this->buffer;
	}
	
	hstr Sound2::_findLinkedFile()
	{
		if (!this->filename.ends_with(".xln"))
		{
			return this->filename;
		}
		if (!hfile::exists(this->filename))
		{
			return this->filename;
		}
		harray<hstr> newFolders = hfile::hread(this->filename).split("/");
		harray<hstr> folders = this->filename.split("/");
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

	Format Sound2::getFormat()
	{
		// TODO - remove?
		return this->buffer->getFormat();
	}

	bool Sound2::load()
	{
		// TODO - remove?
		return this->buffer->load();
	}
	
}
