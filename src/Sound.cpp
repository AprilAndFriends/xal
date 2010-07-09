/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include <iostream>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "Endianess.h"
#include "Sound.h"
#include "Source.h"
#include "AudioManager.h"

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
	Sound::Sound(chstr filename, chstr category) : duration(0.0f), buffer(0), sources(harray<xal::Source*>())
	{
		this->filename = filename;
		this->name = filename.replace("\\", "/").rsplit("/").pop_back().rsplit(".", 1).pop_front();
		this->category = category;
	}

	Sound::~Sound()
	{
		this->stopAll();
		audioMgr->logMessage("destroying sound: " + this->name);
		if (this->buffer != 0)
		{
			alDeleteBuffers(1, &this->buffer);
		}
	}
	
/******* METHODS *******************************************************/
	
	bool Sound::load()
	{
		if (this->filename.contains(".ogg"))
		{
			return this->_loadOgg();
		}
		return false;
	}

	bool Sound::_loadOgg()
	{
		audioMgr->logMessage("loading ogg sound: " + this->filename);
		alGenBuffers(1, &this->buffer);
		vorbis_info *info;
		OggVorbis_File oggFile;
		if (ov_fopen((char*) filename.c_str(), &oggFile) != 0)
		{
			audioMgr->logMessage("OggSound: Error opening file!");
			return false;
		}
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
			audioMgr->logMessage("OggSound: could not allocate ogg buffer");
		}
		ov_clear(&oggFile);
		return result;
	}

	void Sound::bindSource(Source* source)
	{
		source->setSound(this);
		this->sources += source;
	}
	
	void Sound::unbindSource(Source* source)
	{
		source->setSound(NULL);
		this->sources -= source;
	}
	
	float Sound::getSampleOffset()
	{
		if (this->buffer == 0)
		{
			return 0;
		}
		if (this->sources.size() == 0)
		{
			return 0;
		}
		return this->sources[0]->getSampleOffset();
	}

	void Sound::setGain(float gain)
	{
		if (this->buffer == 0)
		{
			return;
		}
		if (this->sources.size() == 0)
		{
			return;
		}
		this->sources[0]->setGain(gain);
	}

	float Sound::getGain()
	{
		if (this->buffer == 0)
		{
			return 1;
		}
		if (this->sources.size() == 0)
		{
			return 1;
		}
		return this->sources[0]->getGain();
	}

	bool Sound::isPlaying()
	{
		if (this->sources.size() == 0)
		{
			return false;
		}
		for (Source** it = this->sources.iterate(); it; it = this->sources.next())
		{
			if ((*it)->isPlaying())
			{
				return true;
			}
		}
		return false;
	}

	bool Sound::isLooping()
	{
		if (this->sources.size() == 0)
		{
			return false;
		}
		return this->sources[0]->isLooping();
	}

/******* PLAY CONTROLS *************************************************/

	Source* Sound::play(float fadeTime, bool looping)
	{
		if (this->buffer == 0)
		{
			return NULL;
		}
		Source* source = NULL;
		if (this->sources.size() == 0 || this->sources[0]->isPlaying())
		{
			source = audioMgr->allocateSource();
			if (source == NULL)
			{
				return NULL;
			}
			this->bindSource(source);
		}
		else
		{
			source = this->sources[0];
		}
		source->play(fadeTime, looping);
		return source;
	}

	Source* Sound::replay(float fadeTime, bool looping)
	{
		if (this->buffer == 0)
		{
			return NULL;
		}
		Source* source = NULL;
		if (this->sources.size() == 0)
		{
			source = audioMgr->allocateSource();
			if (source == NULL)
			{
				return NULL;
			}
			this->bindSource(source);
		}
		else
		{
			source = this->sources[0];
			source->stop();
		}
		source->replay(fadeTime, looping);
		return source;
	}

	void Sound::stop(float fadeTime)
	{
		if (this->buffer == 0)
		{
			return;
		}
		if (this->sources.size() == 0)
		{
			return;
		}
		this->sources[0]->stop(fadeTime);
	}

	void Sound::stopAll(float fadeTime)
	{
		if (this->buffer == 0)
		{
			return;
		}
		for (Source** it = this->sources.iterate(); it; it = this->sources.next())
		{
			(*it)->stop(fadeTime);
		}
	}
	
	void Sound::pause(float fadeTime)
	{
		if (this->buffer == 0)
		{
			return;
		}
		if (this->sources.size() == 0)
		{
			return;
		}
		this->sources[0]->pause(fadeTime);
	}

}
