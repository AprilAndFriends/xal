/// @file
/// @version 4.0
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
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "Utility.h"
#include "xalExport.h"

namespace xal
{
	class BufferAsync;
	class Player;
	class Sound;
	class Source;

	class Buffer
	{
	public:
		friend class AudioManager;
		friend class BufferAsync;

		Buffer(Sound* sound);
		Buffer(Category* category, unsigned char* data, int size, int channels, int samplingRate, int bitsPerSample);
		~Buffer();

		HL_DEFINE_GET(hstr, filename, Filename);
		HL_DEFINE_GET(int, fileSize, FileSize);
		inline hstream& getStream() { return this->stream; }
		HL_DEFINE_GET(Source*, source, Source);
		HL_DEFINE_GET(float, idleTime, IdleTime);

		int getSize();
		int getChannels();
		int getSamplingRate();
		int getBitsPerSample();
		float getDuration();
		Format getFormat() const;
		bool isLoaded();
		bool isAsyncLoadQueued();
		bool isStreamed() const;
		bool isMemoryManaged() const;
		// TODO
		//bool setOffset(int value);

		void prepare();
		bool prepareAsync();
		int load(bool looping, int size = STREAM_BUFFER_SIZE);
		void bind(Player* player, bool playerPaused);
		void unbind(Player* player, bool playerPaused);
		void keepLoaded();
		void rewind();

		int calcOutputSize(int size);
		int calcInputSize(int size);
		void readPcmData(hstream& output);

	protected:
		hstr filename;
		int fileSize;
		BufferMode mode;
		bool loaded;
		hstream stream;
		bool asyncLoadQueued;
		bool asyncLoadDiscarded;
		hmutex asyncLoadMutex;
		Source* source;
		bool loadedMetaData;
		int size;
		int channels;
		int samplingRate;
		int bitsPerSample;
		float duration;
		harray<Player*> boundPlayers;
		float idleTime;
		
		void _update(float timeDelta);
		void _tryLoadMetaData();
		bool _tryClearMemory();

		bool _prepareAsyncStream();
		void _decodeFromAsyncStream();
		void _waitForAsyncLoad(float timeout = 0.0f);

	};

}

#endif
