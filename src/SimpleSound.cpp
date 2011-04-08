/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <iostream>

#if HAVE_OGG
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#endif
#if HAVE_SPX
#include <speex/speex.h>
#endif
#ifndef __APPLE__
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#include <hltypes/hfile.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Category.h"
#include "SimpleSound.h"
#include "Endianess.h"

#define FRAME_SIZE 640

namespace xal
{
/******* CONSTRUCT / DESTRUCT ******************************************/

	SimpleSound::SimpleSound(chstr fileName, chstr category, chstr prefix) :
		SoundBuffer(fileName, category, prefix), buffer(0)
	{
#ifdef HAVE_SPX
		this->spxStream = NULL;
#endif
	}

	SimpleSound::~SimpleSound()
	{
		this->destroySources();
		if (this->buffer != 0)
		{
			alDeleteBuffers(1, &this->buffer);
		}
#ifdef HAVE_SPX
		if (this->spxStream != NULL)
		{
			delete this->spxStream;
			this->spxStream = NULL;
		}
#endif
	}
	
/******* METHODS *******************************************************/

	void SimpleSound::update(float k)
	{
		SoundBuffer::update(k);
		if (this->category->isDynamicDecode() && this->isSpx() && this->decoded && !this->isPlaying())
		{
			this->clearBuffer();
			this->decoded = false;
		}
	}

	bool SimpleSound::_loadOgg()
	{
#if HAVE_OGG
		xal::log("loading ogg sound " + this->fileName);
		vorbis_info *info;
		OggVorbis_File oggStream;
		if (ov_fopen((char*)this->virtualFileName.c_str(), &oggStream) != 0)
		{
			xal::log("ogg: error opening file!");
			return false;
		}
		alGenBuffers(1, &this->buffer);
		info = ov_info(&oggStream, -1);
		unsigned long length = (unsigned long)ov_pcm_total(&oggStream, -1) * info->channels * 2; // always 16 bit data
		unsigned char *data = new unsigned char[length];
		bool result = false;
		if (data != NULL)
		{
			int section;
			unsigned long size = length;
			unsigned char *buffer = data;
			int read;
			while (size > 0)
			{
				read = ov_read(&oggStream, (char*)buffer, size, 0, 2, 1, &section);
				if (read == 0)
				{
					length -= size;
					break;
				}
				size -= read;
				buffer += read;
			}
#ifdef __BIG_ENDIAN__
			for (uint16_t* p = (uint16_t*)data; (unsigned char*)p < buffer; p++)
			{
				XAL_NORMALIZE_ENDIAN(*p);
			}
#endif	
			alBufferData(this->buffer, (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data, length, info->rate);
			this->duration = ((float)length) / (info->rate * info->channels * 2);
			delete [] data;
			result = true;
		}
		else
		{
			xal::log("ogg: could not allocate ogg buffer.");
		}
		ov_clear(&oggStream);
		return result;
#else
#warning HAVE_OGG is not defined to 1. No Ogg support.
		xal::log("no ogg support built in, cannot load " + this->fileName);
		return false;
#endif
	}

	void SimpleSound::clearBuffer()
	{
		if (this->buffer != 0)
		{
			alDeleteBuffers(1, &this->buffer);
			alGenBuffers(1, &this->buffer);
		}
	}

	bool SimpleSound::_loadSpx()
	{
#ifdef HAVE_SPX
		xal::log("loading spx sound " + this->fileName);
		if (!hfile::exists(this->fileName))
		{
			xal::log("spx: file does not exist!");
			return false;
		}
		alGenBuffers(1, &this->buffer);
		hfile file(this->fileName);
		this->spxStreamLength = file.size();
		this->spxStream = (unsigned char*)malloc(this->spxStreamLength * sizeof(unsigned char));
		file.read_raw(this->spxStream, this->spxStreamLength);
		file.close();
		return true;
#else
		return false;
#endif
	}

	bool SimpleSound::_decodeSpx()
	{
#ifdef HAVE_SPX
		/*Holds the audio that will be written to file (16 bits per sample)*/

		/*Speex handle samples as float, so we need an array of floats*/
		float output[FRAME_SIZE];
		char cbits[200];
		unsigned short nbBytes;
		/*Holds the state of the decoder*/
		void* state;
		/*Holds bits so they can be read and written to by the Speex routines*/
		SpeexBits bits;
		int i, tmp;
		/*Create a new decoder state in narrowband mode*/
		state = speex_decoder_init(&speex_uwb_mode);

		/*Set the perceptual enhancement on*/
		tmp = 1;
		speex_decoder_ctl(state, SPEEX_SET_ENH, &tmp);

		int size;
		unsigned char* stream = this->spxStream;

		//fin = fopen(this->fileName.c_str(), "rb");
		int offset = 0;
		memcpy(&size, &this->spxStream[offset], sizeof(int));
		offset += sizeof(int);
		short* buffer = (short*)malloc((FRAME_SIZE + size) * 2 * sizeof(short));
		short* bufferPtr = buffer;

	   /*Initialization of the structure that holds the bits*/
		speex_bits_init(&bits);
		int nFrames = 0;
		while (true)
		{
			/*Read the size encoded by sampleenc, this part will likely be 
			different in your application*/
			memcpy(&nbBytes, &this->spxStream[offset], sizeof(unsigned short));
			offset += sizeof(unsigned short);
			//fread(&nbBytes, sizeof(unsigned short), 1, fin);
			//  fprintf (stderr, "nbBytes: %d\n", nbBytes);
			if (offset >= this->spxStreamLength)
			{
				break;
			}

			memcpy(cbits, &this->spxStream[offset], nbBytes);
			offset += nbBytes;
			/*Copy the data into the bit-stream struct*/
			speex_bits_read_from(&bits, cbits, nbBytes);

			/*Decode the data*/
			speex_decode(state, &bits, output);

			/*Copy from float to short (16 bits) for output*/
			for (i = 0; i < FRAME_SIZE; i++, bufferPtr++)
			{
				*bufferPtr = output[i];
			}
			nFrames++;
		}

		//printf("read %d speex frames\n",nFrames);
		/*Destroy the decoder state*/
		speex_decoder_destroy(state);
		/*Destroy the bit-stream truct*/
		speex_bits_destroy(&bits);
		alBufferData(this->buffer, AL_FORMAT_MONO16, buffer, size * 2, 44100);
		this->duration = (float)size / 44100;
		free(buffer);
		// REFACTOR TO HERE
		return true;
#else
		return false;
#endif
	}

}
