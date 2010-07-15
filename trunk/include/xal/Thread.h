/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_THREAD_H
#define XAL_THREAD_H

#ifndef _WIN32
#include <pthread.h>
#endif

namespace xal
{
	class Thread
	{
	public:
		Thread();
		virtual ~Thread();
	
		//! Creates the thread object and runs it
		void startThread();
		//! The main thread loop function
		virtual void executeThread() = 0;
		//! sets running to false and waits for the thread to complete the last cycle
		void waitforThread();

	protected:
#ifdef _WIN32
		void* handle;
#else
		pthread_t handle;
#endif
		//! Indicates whether the thread is running. As long as this is true, the thread runs in a loop
		volatile bool running;
		
	};
}

#endif
