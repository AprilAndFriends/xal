/// @file
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _FORMAT_SPX
#include <speex/speex.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "SPX_Source.h"
#include "xal.h"

#define FRAME_SIZE 640

namespace xal
{
	SPX_Source::SPX_Source(chstr filename, Category* category) : Source(filename, category)
	{
	}

	SPX_Source::~SPX_Source()
	{
		this->close();
	}

	bool SPX_Source::load(unsigned char* output)
	{
		if (!Source::load(output, size))
		{
			return false;
		}
		/*
		float output2[FRAME_SIZE];
		char cbits[200];
		unsigned short nbBytes;
		void* state;
		SpeexBits bits;
		int i, tmp;
		state = speex_decoder_init(&speex_uwb_mode);
		tmp = 1;
		speex_decoder_ctl(state, SPEEX_SET_ENH, &tmp);

		int size;
		unsigned char* stream = output;

		//fin = fopen(this->fileName.c_str(), "rb");
		int offset = 0;
		memcpy(&size, &output[offset], sizeof(int));
		offset += sizeof(int);
		short* buffer = (short*)malloc((FRAME_SIZE + size) * 2 * sizeof(short));
		short* bufferPtr = buffer;

		speex_bits_init(&bits);
		int nFrames = 0;
		while (true)
		{
			memcpy(&nbBytes, &output[offset], sizeof(unsigned short));
			offset += sizeof(unsigned short);
			//fread(&nbBytes, sizeof(unsigned short), 1, fin);
			//  fprintf (stderr, "nbBytes: %d\n", nbBytes);
			if (offset >= this->size)
			{
				break;
			}

			memcpy(cbits, &output[offset], nbBytes);
			offset += nbBytes;
			speex_bits_read_from(&bits, cbits, nbBytes);

			speex_decode(state, &bits, output2);

			for_iterx (i, 0, FRAME_SIZE)
			{
				bufferPtr[i] = (short)output2[i];
			}
			++nFrames;
		}

		//printf("read %d speex frames\n",nFrames);
		speex_decoder_destroy(state);
		speex_bits_destroy(&bits);
		//alBufferData(this->buffer, AL_FORMAT_MONO16, buffer, size * 2, 44100);
		this->duration = (float)size / 44100;
		free(buffer);
		// REFACTOR TO HERE
		//*/
		return true;
	}

}
#endif
