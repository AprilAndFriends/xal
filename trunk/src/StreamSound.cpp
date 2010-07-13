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
#include "StreamSound.h"

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

	StreamSound::StreamSound(chstr filename, chstr category, chstr prefix) :
		Sound(filename, category, prefix)
	{
		for (int i = 0; i < BUFFER_COUNT; i++)
		{
			this->buffers[i] = 0;
		}
	}

	StreamSound::~StreamSound()
	{
		this->stopAll();
		if (this->buffers[0] != 0)
		{
			alDeleteBuffers(2, this->buffers);
		}
		ov_clear(&this->oggStream);
	}
	
/******* METHODS *******************************************************/

	unsigned int StreamSound::getBuffer()
	{
		return this->buffers[0];
	}

	void StreamSound::update(unsigned int sourceId)
	{
		int processed;
		alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &processed);
		unsigned int buffer;
		while (processed > 0)
		{
			alSourceUnqueueBuffers(sourceId, 1, &buffer);
			this->_updateStream(buffer);
			alSourceQueueBuffers(sourceId, 1, &buffer);
			processed--;
		}
	}
	
	void StreamSound::_updateStream(unsigned int buffer)
	{
		char data[BUFFER_SIZE];
		int  size = 0;
		int  section;
		int  result;
	 
		while(size < BUFFER_SIZE)
		{
			result = ov_read(&this->oggStream, data + size, BUFFER_SIZE - size, 0, 2, 1, &section);
		
			if (result > 0)
				size += result;
			else if(result == 0)
				break;
			//else
			//	throw oggString(result);
		}
		
		if (size == 0)
			return;
	 
		alBufferData(buffer, (this->vorbisInfo->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
			data, size, this->vorbisInfo->rate);
	}
 
    bool StreamSound::load()
	{
		if (!xal::mgr->isEnabled())
		{
			return true;
		}
		if (this->filename.contains(".ogg"))
		{
			return this->_loadOgg();
		}
		return false;
	}

	bool StreamSound::_loadOgg()
	{
		xal::mgr->logMessage("Audio Manager: Loading ogg stream sound: " + this->filename);
		if (ov_fopen((char*)this->filename.c_str(), &this->oggStream) != 0)
		{
			xal::mgr->logMessage("OggSound: Error opening file!");
			return false;
		}
		alGenBuffers(BUFFER_COUNT, this->buffers);
		this->vorbisInfo = ov_info(&this->oggStream, -1);
		unsigned long len = ov_pcm_total(&this->oggStream, -1) * this->vorbisInfo->channels * 2; // always 16 bit data
		unsigned char *data = new unsigned char[len];
		bool result = false;
		if (data != NULL)
		{
			/*
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
			this->duration = ((float)len) / (info->rate * info->channels * 2);
			*/
			delete [] data;
			result = true;
		}
		else
		{
			xal::mgr->logMessage("OggSound: Could not allocate ogg buffer!");
		}
		return result;
	}

}