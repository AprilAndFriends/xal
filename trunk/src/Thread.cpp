/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <stdio.h>
#include "Thread.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace xal
{
#ifdef _WIN32
	unsigned long WINAPI async_Call(void* param)
	{
#else
	void *async_Call(void* param)
	{
#endif
		Thread* t = (Thread*)param;
		t->executeThread();
#ifndef _WIN32
		pthread_exit(NULL);
#endif
		return 0;
	}

	Thread::Thread()
	{
		this->running = false;
		this->handle = 0;
	}

	Thread::~Thread()
	{
#ifdef _WIN32
		if (this->handle)
		{
			CloseHandle(this->handle);	
		}
#endif
	}

	void Thread::startThread()
	{
		this->running = true;
#ifdef _WIN32
		this->handle = CreateThread(0, 0, &async_Call, this, 0, 0);
#else
		int ret = pthread_create(&this->handle, NULL, &async_Call, this);
		if (ret != 0)
		{
			printf("ERROR: Unable to create thread!\n");
		}
#endif
	}

	void Thread::waitforThread()
	{
		this->running = false;
#ifdef _WIN32
		WaitForSingleObject(this->handle, INFINITE);
		if (this->handle)
		{
			CloseHandle(this->handle);
			this->handle = 0;
		}
#else
		pthread_join(this->handle, 0);
#endif
	}
	
}
