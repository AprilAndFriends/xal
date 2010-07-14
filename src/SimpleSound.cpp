/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "SimpleSound.h"

#include <iostream>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#ifndef __APPLE__
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	SimpleSound::SimpleSound(chstr filename, chstr category, chstr prefix) :
		buffer(0), Sound(filename, category, prefix)
	{
	}

	SimpleSound::~SimpleSound()
	{
		this->stopAll();
		if (this->buffer != 0)
		{
			alDeleteBuffers(1, &this->buffer);
		}
	}
	
/******* METHODS *******************************************************/

	bool SimpleSound::load()
	{
		if (!xal::mgr->isEnabled())
		{
			return true;
		}
		if (this->isOgg())
		{
			return this->_loadOgg();
		}
		return false;
	}

	bool SimpleSound::_loadOgg()
	{
		xal::mgr->logMessage("XAL: Loading ogg sound: " + this->filename);
		vorbis_info *info;
		OggVorbis_File oggFile;
		if (ov_fopen((char*)this->filename.c_str(), &oggFile) != 0)
		{
			xal::mgr->logMessage("Ogg: Error opening file!");
			return false;
		}
		alGenBuffers(1, &this->buffer);
		info = ov_info(&oggFile, -1);
		unsigned long len = ov_pcm_total(&oggFile, -1) * info->channels * 2; // always 16 bit data
		unsigned char *data = new unsigned char[len];
		bool result = false;
		if (data != NULL)
		{
			int bs = -1;
			unsigned long todo = len;
			unsigned char *bufpt = data;
			while (todo > 0)
			{
				int read = ov_read(&oggFile, (char*)bufpt, todo, 0, 2, 1, &bs);
				if (!read)
				{
					len -= todo;
					break;
				}
				todo -= read;
				bufpt += read;
			}

#ifdef __BIG_ENDIAN__
			for (uint16_t* p = (uint16_t*)data; (unsigned char*)p < bufpt; p++)
			{
				NORMALIZE_ENDIAN(*p);
			}
#endif	
			alBufferData(this->buffer, (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data, len, info->rate);
			this->duration = ((float)len) / (info->rate * info->channels * 2);
			delete [] data;
			result = true;
		}
		else
		{
			xal::mgr->logMessage("OggSound: could not allocate ogg buffer");
		}
		ov_clear(&oggFile);
		return result;
	}

}