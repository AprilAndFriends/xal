/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_STREAMSOUND_H
#define XAL_STREAMSOUND_H

#if HAVE_OGG
#include <vorbis/vorbisfile.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include "xalExport.h"
#include "SoundBuffer.h"

#define STREAM_BUFFER_COUNT 2
#define STREAM_BUFFER_SIZE 262144

namespace xal
{
	class xalExport StreamSound : public SoundBuffer
	{
	public:
		StreamSound(chstr name, chstr category, chstr prefix = "");
		~StreamSound();
		
		void update(float k);
		
		void queueBuffers(int index, int count);
		void queueBuffers();
		void unqueueBuffers(int index, int count);
		void unqueueBuffers();
		void rewindStream();
		
		Sound* play(float fadeTime = 0.0f, bool looping = false);
		
		unsigned int getBuffer() const;
		
	protected:
		unsigned int buffers[STREAM_BUFFER_COUNT];
		int bufferIndex;
#if HAVE_OGG
		OggVorbis_File oggStream;
		vorbis_info* vorbisInfo;
#endif
		
		int _readStream(char* buffer, int size);
		void _resetStream();
		int _fillBuffer(unsigned int buffer);
		void _fillStartBuffers();
		bool _loadOgg();

	};

}

#endif
