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
#include <xal/AudioManager.h>
#include <xal/Sound.h>
#include <xal/Sound.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#define _TEST_SOUND
//#define _TEST_SOURCE_HANDLING
//#define _TEST_MULTIPLE_PLAY
//#define _TEST_STREAM
//#define _TEST_FADE_IN
//#define _TEST_FADE_OUT
//#define _TEST_THREADED
//#define _TEST_COMPLEX_HANDLER

#include <windows.h>

int main(int argc, char **argv)
{
#ifndef _TEST_THREADED
	xal::init("", false);
#else
	xal::init("", true, 0.2f);
#endif
	xal::Sound* s;
#ifdef _TEST_STREAM
	xal::mgr->createCategory("cat", true);
#endif
	harray<hstr> files = xal::mgr->createSoundsFromPath("../media", "cat", "");
#ifdef _TEST_SOUND
#ifdef _TEST_STREAM
	s = xal::mgr->getSound("wind");
#else
	s = xal::mgr->getSound("bark");
#endif
#ifndef _TEST_COMPLEX_HANDLER
	s->play();
	while (s->isPlaying())
	{
		Sleep(100.0f);
#ifndef _TEST_THREADED
		xal::mgr->update(0.1f);
#endif
	}
#ifndef _TEST_THREADED
	xal::mgr->update(0.01f);
#endif
#endif
	
#ifdef _TEST_MULTIPLE_PLAY
	s->play();
	Sleep(100.0f);
	s->play();
	while (s->isPlaying())
	{
		Sleep(100.0f);
#ifndef _TEST_THREADED
		xal::mgr->update(0.1f);
#endif
	}
#elif defined _TEST_SOURCE_HANDLING
	for (int i = 0; i < XAL_MAX_SOURCES + 1; i++)
	{
		Sleep(20.0f);
		s->play();
	}
	while (s->isPlaying())
	{
		Sleep(100.0f);
#ifndef _TEST_THREADED
		xal::mgr->update(0.1f);
#endif
	}
	xal::mgr->update(0.01f);
	s = xal::mgr->getSound("wind");

	s->play();
	for (int i = 0; i < 20; i++)
	{
		Sleep(100.0f);
#ifndef _TEST_THREADED
		xal::mgr->update(0.1f);
#endif
	}
#ifndef _TEST_THREADED
	xal::mgr->update(0.1f);
#endif
	s->stop();
	
#endif
#endif

#ifdef _TEST_FADE_IN
	s = xal::mgr->getSound("wind");
	s->play(1.0f);
	for (int i = 0; i < 20; i++)
	{
		Sleep(100.0f);
		printf("T:%d P:%s F:%s\n", i, hstr(s->isPlaying()).c_str(), hstr(s->isFading()).c_str());
#ifndef _TEST_THREADED
		xal::mgr->update(0.1f);
#endif
	}
	s->stop();
#ifndef _TEST_THREADED
	xal::mgr->update(0.01f);
#endif
#endif

#ifdef _TEST_FADE_OUT
	s = xal::mgr->getSound("wind");
	s->play();
	s->stop(1.0f);
	for (int i = 0; i < 20; i++)
	{
		Sleep(100.0f);
		printf("T:%d P:%s F:%s\n", i, hstr(s->isPlaying()).c_str(), hstr(s->isFading()).c_str());
#ifndef _TEST_THREADED
		xal::mgr->update(0.1f);
#endif
	}
#endif

#ifdef _TEST_COMPLEX_HANDLER
	xal::Sound* temp;
	xal::Sound* s1 = xal::mgr->getSound("wind")->play();
	xal::Sound* s2 = xal::mgr->getSound("wind_copy")->play();
	xal::Sound* t1 = s1;
	xal::Sound* t2 = s2;
	s2->pause();
	for (int i = 0; i < 50; i++)
	{
		Sleep(100.0f);
#ifndef _TEST_THREADED
		xal::mgr->update(0.1f);
#endif
		s2->play();
		s1->pause();
		Sleep(100.0f);
#ifndef _TEST_THREADED
		xal::mgr->update(0.1f);
#endif
		s2->pause();
		s1->play();
		if (i % 3 == 0)
		{
			temp = s1;
			s1 = s2;
			s2 = temp;
		}
	}
#endif
	
	xal::destroy();
	system("pause");
	return 0;
}
