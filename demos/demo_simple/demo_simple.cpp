/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include <xal/AudioManager.h>
#include <xal/Sound.h>
#include <xalutil/ParallelSoundManager.h>
#include <xalutil/Playlist.h>

//#define _TEST_STREAM
//#define _TEST_LINKS
//#define _TEST_THREADED

#define _TEST_SOUND
//#define _TEST_SOURCE_HANDLING
//#define _TEST_MULTIPLAY
//#define _TEST_MULTIPLE_STREAM
//#define _TEST_FADE_IN
//#define _TEST_FADE_OUT
//#define _TEST_FADE_IN_OUT
//#define _TEST_COMPLEX_HANDLER

//#define _TEST_UTIL_PLAYLIST
//#define _TEST_UTIL_PARALLEL_SOUNDS


#ifndef _TEST_THREADED
#define _update(time) xal::mgr->update(time)
#else
#define _update(time)
#endif
xal::Sound* s;

void _test_sound()
{
	printf("  - start test sound...\n");
	s->play();
	for (int i = 0; i < 20; i++)
	{
		Sleep(100.0f);
		_update(0.1f);
		s->pause();
		Sleep(100.0f);
		_update(0.1f);
		s->play();
	}
	_update(1.0f);
	s->stop();
}

void _test_multistream()
{
	printf("  - start test multiple stream...\n");
	for (int i = 0; i < 5; i++)
	{
		s->play();
		Sleep(1000.0f);
		_update(1.0f);
		if (i == 2)
		{
			s->pause(1.5f);
			for (int j = 0; j < 10; j++)
			{
				Sleep(100.0f);
				_update(0.1f);
			}
		}
	}
	s->stop();
	_update(0.1f);
}

void _test_multiplay()
{
	printf("  - start test multiple play...\n");
	s->play();
	Sleep(100.0f);
	s->play();
	while (s->isPlaying())
	{
		Sleep(100.0f);
		_update(0.1f);
	}
	s->stop();
	_update(0.1f);
}

void _test_sources()
{
	printf("  - start test sources...\n");
	s->play();
	for (int i = 0; i < XAL_MAX_SOURCES + 1; i++)
	{
		Sleep(20.0f);
		s->play();
	}
	while (s->isPlaying())
	{
		Sleep(100.0f);
		_update(0.1f);
	}
	xal::mgr->update(0.01f);
	s = xal::mgr->getSound("wind");
	s->play();
	for (int i = 0; i < 20; i++)
	{
		Sleep(100.0f);
		_update(0.1f);
	}
	s->stop();
	_update(0.1f);
}

void _test_fadein()
{
	printf("  - start test fade in...\n");
	s = xal::mgr->getSound("wind");
	s->play(1.0f);
	for (int i = 0; i < 20; i++)
	{
		Sleep(100.0f);
		printf("T:%d P:%s F:%s\n", i, hstr(s->isPlaying()).c_str(), hstr(s->isFading()).c_str());
		_update(0.1f);
	}
	s->stop();
	_update(1.0f);
}

void _test_fadeout()
{
	printf("  - start test fade out...\n");
	s = xal::mgr->getSound("wind");
	s->play();
	s->stop(1.0f);
	for (int i = 0; i < 20; i++)
	{
		Sleep(100.0f);
		printf("T:%d P:%s F:%s\n", i, hstr(s->isPlaying()).c_str(), hstr(s->isFading()).c_str());
		_update(0.1f);
	}
	s->stop();
	_update(0.1f);
}

void _test_fadeinout()
{
	printf("  - start test fade in and out...\n");
	s = xal::mgr->getSound("wind");
	s->play(1.0f);
	for (int i = 0; i < 8; i++)
	{
		Sleep(100.0f);
		printf("T:%d P:%s F:%s\n", i, hstr(s->isPlaying()).c_str(), hstr(s->isFading()).c_str());
		_update(0.1f);
	}
	s->pause(1.0f);
	for (int i = 0; i < 6; i++)
	{
		Sleep(100.0f);
		printf("T:%d P:%s F:%s\n", i, hstr(s->isPlaying()).c_str(), hstr(s->isFading()).c_str());
		_update(0.1f);
	}
	s->play(1.0f);
	for (int i = 0; i < 3; i++)
	{
		Sleep(100.0f);
		printf("T:%d P:%s F:%s\n", i, hstr(s->isPlaying()).c_str(), hstr(s->isFading()).c_str());
		_update(0.1f);
	}
	printf("- 10 more updates\n");
	for (int i = 0; i < 10; i++)
	{
		Sleep(100.0f);
		printf("T:%d P:%s F:%s\n", i, hstr(s->isPlaying()).c_str(), hstr(s->isFading()).c_str());
		_update(0.1f);
	}
	s->stop();
	_update(1.0f);
}

void _test_complex_handler()
{
	printf("  - start test complex handler...\n");
	xal::Sound* temp;
	xal::Sound* s1 = xal::mgr->getSound("wind");
	xal::Sound* s2 = xal::mgr->getSound("wind_copy");
	xal::Sound* t1 = s1->play();
	xal::Sound* t2 = s2->play();
	s2->pause();
	for (int i = 0; i < 50; i++)
	{
		Sleep(100.0f);
		_update(0.1f);
		s2->play();
		s1->pause();
		Sleep(100.0f);
		_update(0.1f);
		s2->pause();
		s1->play();
		if (i % 3 == 0)
		{
			temp = s1;
			s1 = s2;
			s2 = temp;
		}
	}
	s1->stop();
	s2->stop();
	_update(0.1f);
}

void _test_util_playlist()
{
	printf("  - start test util playlist...\n");
	xal::Playlist list(false);
	list.queueSound("bark");
	list.queueSound("bark");
	list.queueSound("wind");
	list.queueSound("bark");
	list.play();
	while (list.isPlaying()) { list.update(); }
}

void _test_util_parallel_sounds()
{
	printf("  - start test util parallel sounds...\n");
	harray<hstr> names;
	names += "bark";
	xal::ParallelSoundManager pmgr;
	pmgr.updateList(names);
	Sleep(1.0f);
	_update(1.0f);
	names.clear();
	pmgr.updateList(names);
	Sleep(1.0f);
	_update(1.0f);
}

int main(int argc, char **argv)
{
#ifndef _TEST_THREADED
	xal::init("", false);
#else
	xal::init("", true, 0.2f);
#endif
#ifndef _TEST_LINKS
#ifdef _TEST_STREAM
	xal::mgr->createCategory("cat", true);
#endif
	harray<hstr> files = xal::mgr->createSoundsFromPath("../media", "cat", "");
#else
	xal::mgr->createCategory("cat", true);
	xal::mgr->createSound("../media/linked/linked_sound.xln", "cat");
#endif
#ifndef _TEST_LINKS
#ifdef _TEST_STREAM
	s = xal::mgr->getSound("wind");
#else
	s = xal::mgr->getSound("bark");
#endif
#else
	s = xal::mgr->getSound("linked_sound");
#endif

#ifdef _TEST_SOUND
	_test_sound();
#endif
#ifdef _TEST_MULTIPLE_STREAM
	_test_multistream();
#endif
#ifdef _TEST_MULTIPLAY
	_test_multiplay();
#endif
#ifdef _TEST_SOURCE_HANDLING
	_test_sources();
#endif
#ifdef _TEST_FADE_IN
	_test_fadein();
#endif
#ifdef _TEST_FADE_OUT
	_test_fadeout();
#endif
#ifdef _TEST_FADE_IN_OUT
	_test_fadeinout();
#endif
#ifdef _TEST_COMPLEX_HANDLER
	_test_complex_handler();
#endif
#ifdef _TEST_UTIL_PLAYLIST
	_test_util_playlist();
#endif
#ifdef _TEST_UTIL_PARALLEL_SOUNDS
	_test_util_parallel_sounds();
#endif
	printf("  - done\n");
	xal::destroy();
	system("pause");
	return 0;
}
