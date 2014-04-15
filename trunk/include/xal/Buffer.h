/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Provides a buffer for audio data.

#ifndef XAL_BUFFER_H
#define XAL_BUFFER_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Player;
	class Sound;
	class Source;

	class xalExport Buffer
	{
	public:
		friend class AudioManager;

		Buffer(Sound* sound);
		~Buffer();

		HL_DEFINE_GET(hstr, filename, Filename);
		HL_DEFINE_GET(int, fileSize, FileSize);
		HL_DEFINE_GET(unsigned char*, stream, Stream);
		HL_DEFINE_GET(Source*, source, Source);

		int getSize();
		int getChannels();
		int getSamplingRate();
		int getBitsPerSample();
		float getDuration();
		Format getFormat();
		bool isStreamed();
		bool isMemoryManaged();
		bool setOffset(int value);

		void prepare();
		int load(bool looping, int size = STREAM_BUFFER_SIZE);
		void bind(Player* player, bool playerPaused);
		void unbind(Player* player, bool playerPaused);
		void keepLoaded();
		void rewind();

		int convertToOutputSize(int size);
		int convertToInputSize(int size);
		int readPcmData(unsigned char** output);

	protected:
		hstr filename;
		int fileSize;
		BufferMode mode;
		bool loaded;
		bool decoded;
		unsigned char* stream;
		int streamSize;
		int dataSize;
		Source* source;
		bool loadedData;
		int size;
		int channels;
		int samplingRate;
		int bitPerSample;
		float duration;
		harray<Player*> boundPlayers;
		float idleTime;

		void _update(float k);
		void _tryLoadData();
		bool _tryClearMemory();

	};

}

#endif
