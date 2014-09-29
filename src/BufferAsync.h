/// @file
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines utilities for asynchronous buffer loading.

#ifndef XAL_BUFFER_ASYNC_H
#define XAL_BUFFER_ASYNC_H

#include <hltypes/harray.h>
#include <hltypes/hlist.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>
#include <hltypes/hthread.h>
#include <hltypes/hstring.h>

#include "xalExport.h"

namespace xal
{
	class Buffer;

	class BufferAsync
	{
	public:
		static void update();
		static bool queueLoad(Buffer* buffer);
		static bool prioritizeLoad(Buffer* buffer);
		static bool isRunning();

	protected:
		static harray<Buffer*> buffers;
		static int loaded;
		static hmutex queueMutex;

		static hthread readerThread;
		static bool readerRunning;

		static harray<hthread*> decoderThreads;

		static void _read(hthread* thread);
		static void _decode(hthread* thread);

	private: // prevents inheritance and instantiation
		BufferAsync() { }
		~BufferAsync() { }

	};
	
}

#endif
