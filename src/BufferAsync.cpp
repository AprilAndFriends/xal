/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_ANDROID) || defined(__APPLE__)
#include <unistd.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "xal.h"
#include "Buffer.h"
#include "BufferAsync.h"

namespace xal
{
	harray<Buffer*> BufferAsync::buffers;
	int BufferAsync::loaded = 0;
	hmutex BufferAsync::queueMutex;

	hthread BufferAsync::readerThread(&BufferAsync::_read, "XAL async loader");
	bool BufferAsync::readerRunning = false;

	harray<hthread*> BufferAsync::decoderThreads;

	static int cpus = 0; // needed, because certain calls are made when fetching SystemInfo that are not allowed to be made in secondary threads on some platforms

	void BufferAsync::update()
	{
		hmutex::ScopeLock lock(&BufferAsync::queueMutex);
		if (BufferAsync::readerRunning && !BufferAsync::readerThread.isRunning())
		{
			BufferAsync::readerThread.join();
			BufferAsync::readerRunning = false;
		}
		if (!BufferAsync::readerRunning && BufferAsync::buffers.size() > 0) // new textures got queued in the meantime
		{
			BufferAsync::readerRunning = true;
			BufferAsync::readerThread.start();
		}
	}

	bool BufferAsync::queueLoad(Buffer* buffer)
	{
		if (cpus == 0)
		{
#ifdef _WIN32
			SYSTEM_INFO w32info;
			GetNativeSystemInfo(&w32info);
			cpus = w32info.dwNumberOfProcessors;
#elif defined(__APPLE__) // not sure why, but for Apple hardware the ONLN one is used (possibly some hardware may have locked available cores)
			cpus = (int)sysconf(_SC_NPROCESSORS_ONLN);
#else // on Android and other Unix it's better to use CONF since they are more flexible than Apple's hardware and might switch cores on/off dynamically
			cpus = (int)sysconf(_SC_NPROCESSORS_CONF);
#endif
		}
		hmutex::ScopeLock lock(&BufferAsync::queueMutex);
		if (BufferAsync::buffers.contains(buffer))
		{
			return false;
		}
		BufferAsync::buffers += buffer;
		if (!BufferAsync::readerRunning)
		{
			BufferAsync::readerRunning = true;
			BufferAsync::readerThread.start();
		}
		return true;
	}

	bool BufferAsync::prioritizeLoad(Buffer* buffer)
	{
		hmutex::ScopeLock lock(&BufferAsync::queueMutex);
		if (!BufferAsync::buffers.contains(buffer))
		{
			return false;
		}
		int index = BufferAsync::buffers.indexOf(buffer);
		if (index >= BufferAsync::loaded) // if not loaded from disk yet
		{
			if (index > BufferAsync::loaded) // if not already at the front
			{
				BufferAsync::buffers.removeAt(index);
				BufferAsync::buffers.insertAt(BufferAsync::loaded, buffer);
			}
		}
		else if (index > 0) // if data was already loaded in RAM, but not decoded and not already at the front
		{
			BufferAsync::buffers.removeAt(index);
			BufferAsync::buffers.addFirst(buffer);
		}
		return true;
	}

	bool BufferAsync::isRunning()
	{
		hmutex::ScopeLock lock(&BufferAsync::queueMutex);
		return BufferAsync::readerRunning;
	}

	void BufferAsync::_read(hthread* thread)
	{
		Buffer* buffer = NULL;
		bool streamLoaded = true;
		hthread* decoderThread = NULL;
		int index = 0;
		int size = 0;
		bool running = true;
		hmutex::ScopeLock lock;
		while (running)
		{
			running = false;
			// check for new queued textures
			lock.acquire(&BufferAsync::queueMutex);
			if (BufferAsync::buffers.size() > BufferAsync::loaded)
			{
				running = true;
				buffer = BufferAsync::buffers[BufferAsync::loaded];
				lock.release();
				streamLoaded = buffer->_prepareAsyncStream();
				lock.acquire(&BufferAsync::queueMutex);
				index = BufferAsync::buffers.indexOf(buffer); // it's possible that the queue was rearranged in the meantime
				if (streamLoaded)
				{
					if (index >= BufferAsync::loaded)
					{
						if (index > BufferAsync::loaded) // if texture was moved towards the back of the queue
						{
							// put it back to the current decoder position
							BufferAsync::buffers.removeAt(index);
							BufferAsync::buffers.insertAt(BufferAsync::loaded, buffer);
						}
					}
					++BufferAsync::loaded;
				}
				else // it was canceled
				{
					BufferAsync::buffers.removeAt(index);
				}
			}
			size = BufferAsync::loaded;
			lock.release();
			// create new worker threads if needed
			if (size > 0)
			{
				running = true;
				size = hmin(size, cpus) - BufferAsync::decoderThreads.size();
				for_iter (i, 0, size)
				{
					decoderThread = new hthread(&BufferAsync::_decode, "XAL async decoder");
					BufferAsync::decoderThreads += decoderThread;
					decoderThread->start();
				}
			}
			// check current worker threads' status
			if (BufferAsync::decoderThreads.size() > 0)
			{
				running = true;
				for_iter (i, 0, BufferAsync::decoderThreads.size())
				{
					if (!BufferAsync::decoderThreads[i]->isRunning())
					{
						decoderThread = BufferAsync::decoderThreads.removeAt(i);
						decoderThread->join();
						delete decoderThread;
						--i;
					}
				}
			}
		}
	}

	void BufferAsync::_decode(hthread* thread)
	{
		Buffer* buffer = NULL;
		hmutex::ScopeLock lock(&BufferAsync::queueMutex);
		while (BufferAsync::loaded > 0)
		{
			buffer = BufferAsync::buffers.removeFirst();
			--BufferAsync::loaded;
			lock.release();
			buffer->_decodeFromAsyncStream();
			lock.acquire(&BufferAsync::queueMutex);
		}
	}

}
