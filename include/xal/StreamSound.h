/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_STREAMSOUND_H
#define XAL_STREAMSOUND_H

#include <vorbis/vorbisfile.h>

#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include "xalExport.h"
#include "SoundBuffer.h"

#define STREAM_BUFFER_COUNT 8
#define STREAM_BUFFER_SIZE 8192

namespace xal
{
	class xalExport StreamSound : public SoundBuffer
	{
	public:
		StreamSound(chstr name, chstr category, chstr prefix = "");
		~StreamSound();
		
		bool load();
		void update(unsigned int sourceId);
		
		void queueBuffers(unsigned int sourceId, int index = 0, int count = STREAM_BUFFER_COUNT);
		void unqueueBuffers(unsigned int sourceId, int index = 0, int count = STREAM_BUFFER_COUNT);
		
		unsigned int getBuffer();
		
	protected:
		unsigned int buffers[STREAM_BUFFER_COUNT];
		int bufferIndex;
		OggVorbis_File oggStream;
		vorbis_info* vorbisInfo;
		
		int _readStream(char* buffer, int size);
		void _resetStream();
		int _fillBuffer(unsigned int buffer);
		bool _loadOgg();

	};

}

#endif
