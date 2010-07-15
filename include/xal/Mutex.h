/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_MUTEX_H
#define XAL_MUTEX_H

#ifndef _WIN32
//#include <pthread.h>
#endif

namespace xal
{
	class Mutex
	{
	public:
		Mutex();
		~Mutex();
		//! Lock the mutex. If another thread has lock, the caller thread will wait until the previous thread unlocks it
		void lock();
		//! Unlock the mutex. Use this when you're done with thread-safe sections of your code
		void unlock();
		
	protected:
#ifdef _WIN32
		void* handle;
#else
		pthread_mutex_t handle;
#endif
		
	};
}

#endif
