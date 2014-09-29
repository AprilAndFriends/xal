/// @file
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

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

	hthread BufferAsync::readerThread(&BufferAsync::_read);
	bool BufferAsync::readerRunning = false;

	harray<hthread*> BufferAsync::decoderThreads;

	static int cpus = 0; // needed, because certain calls are made when fetching SystemInfo that are not allowed to be made in secondary threads on some platforms

	void BufferAsync::update()
	{
		BufferAsync::queueMutex.lock();
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
		BufferAsync::queueMutex.unlock();
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
			cpus = sysconf(_SC_NPROCESSORS_ONLN);
#else // on Android and other Unix it's better to use CONF since they are more flexible than Apple's hardware and might switch cores on/off dynamically
			cpus = sysconf(_SC_NPROCESSORS_CONF);
#endif
		}
		bool result = false;
		BufferAsync::queueMutex.lock();
		if (!BufferAsync::buffers.contains(buffer))
		{
			BufferAsync::buffers += buffer;
			if (!BufferAsync::readerRunning)
			{
				BufferAsync::readerRunning = true;
				BufferAsync::readerThread.start();
			}
			result = true;
		}
		BufferAsync::queueMutex.unlock();
		return result;
	}

	bool BufferAsync::prioritizeLoad(Buffer* buffer)
	{
		bool result = false;
		BufferAsync::queueMutex.lock();
		if (BufferAsync::buffers.contains(buffer))
		{
			int index = BufferAsync::buffers.index_of(buffer);
			if (index >= BufferAsync::loaded) // if not loaded from disk yet
			{
				if (index > BufferAsync::loaded) // if not already at the front
				{
					BufferAsync::buffers.remove_at(index);
					BufferAsync::buffers.insert_at(loaded, buffer);
				}
			}
			else if (index > 0) // if data was already loaded in RAM, but not decoded and not already at the front
			{
				BufferAsync::buffers.remove_at(index);
				BufferAsync::buffers.push_first(buffer);
			}
			result = true;
		}
		BufferAsync::queueMutex.unlock();
		return result;
	}

	bool BufferAsync::isRunning()
	{
		BufferAsync::queueMutex.lock();
		bool result = BufferAsync::readerRunning;
		BufferAsync::queueMutex.unlock();
		return result;
	}

	void BufferAsync::_read(hthread* thread)
	{
		Buffer* buffer = NULL;
		bool streamLoaded = true;
		hthread* decoderThread = NULL;
		int size = 0;
		bool running = true;
		while (running)
		{
			running = false;
			// check for new queued textures
			BufferAsync::queueMutex.lock();
			if (BufferAsync::buffers.size() > loaded)
			{
				running = true;
				buffer = BufferAsync::buffers[loaded];
				BufferAsync::queueMutex.unlock();
				streamLoaded = buffer->_prepareAsyncStream();
				BufferAsync::queueMutex.lock();
				int index = BufferAsync::buffers.index_of(buffer); // it's possible that the queue was rearranged in the meantime
				if (streamLoaded)
				{
					if (index >= BufferAsync::loaded)
					{
						if (index > BufferAsync::loaded) // if texture was moved towards the back of the queue
						{
							// put it back to the current decoder position
							BufferAsync::buffers.remove_at(index);
							BufferAsync::buffers.insert_at(BufferAsync::loaded, buffer);
						}
					}
					++loaded;
				}
				else // it was canceled
				{
					BufferAsync::buffers.remove_at(index);
				}
			}
			size = BufferAsync::loaded;
			BufferAsync::queueMutex.unlock();
			// create new worker threads if needed
			if (size > 0)
			{
				running = true;
				size = hmin(size, cpus) - BufferAsync::decoderThreads.size();
				for_iter (i, 0, size)
				{
					decoderThread = new hthread(&BufferAsync::_decode);
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
						decoderThread = BufferAsync::decoderThreads.remove_at(i);
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
		while (true)
		{
			BufferAsync::queueMutex.lock();
			if (BufferAsync::loaded == 0)
			{
				BufferAsync::queueMutex.unlock();
				break;
			}
			Buffer* buffer = BufferAsync::buffers.remove_first();
			--BufferAsync::loaded;
			BufferAsync::queueMutex.unlock();
			buffer->_decodeFromAsyncStream();
		}
	}

}
