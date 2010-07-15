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
	
		void start();
		virtual void execute();
		void join();
		void sleep(int milliseconds);

	protected:
#ifdef _WIN32
		void* handle;
#else
		pthread_t handle;
#endif
		volatile bool running;
		
	};
}

#endif
