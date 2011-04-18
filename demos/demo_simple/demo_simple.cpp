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
#include <hltypes/hthread.h>

#include <xal/AudioManager.h>
#include <xal/Player.h>
#include <xal/xal.h>
#include <xalutil/ParallelSoundManager.h>
#include <xalutil/Playlist.h>

//#define _TEST_STREAM
//#define _TEST_LINKS
//#define _TEST_THREADED

//#define _TEST_SOUND
//#define _TEST_SOURCE_HANDLING
//#define _TEST_MULTIPLAY
//#define _TEST_MULTIPLE_STREAM
#define _TEST_FADE_IN
#define _TEST_FADE_OUT
#define _TEST_FADE_IN_OUT
//#define _TEST_COMPLEX_HANDLER

//#define _TEST_UTIL_PLAYLIST
//#define _TEST_UTIL_PARALLEL_SOUNDS

#ifndef _TEST_THREADED
#define _update(time) xal::mgr->update(time)
#else
#define _update(time)
#endif

#ifndef _TEST_LINKS
#define S_BARK "bark"
#define S_WIND "wind"
#define S_WIND_COPY "wind_copy"
#ifdef _TEST_STREAM
#define USED_SOUND S_BARK
#else
#define USED_SOUND S_WIND
#endif
#else
#define USED_SOUND "linked_sound"
#define S_BARK "bark"
#define S_WIND "linked_sound"
#define S_WIND_COPY "wind_copy"
#endif

#define XAL_MAX_SOURCES 16 // needed when using OpenAL

xal::Player* s;

void _test_sound()
{
	printf("  - start test sound...\n");
	s->play();
	for (int i = 0; i < 20; i++)
	{
		hthread::sleep(100);
		_update(0.1f);
		s->pause();
		hthread::sleep(100);
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
		hthread::sleep(1000);
		_update(1.0f);
		if (i == 1)
		{
			s->pause(1.5f);
			for (int j = 0; j < 10; j++)
			{
				hthread::sleep(100);
				_update(0.1f);
			}
		}
		else if (i == 3)
		{
			s->pause();
			for (int j = 0; j < 10; j++)
			{
				hthread::sleep(100);
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
	xal::mgr->play(S_BARK);
	hthread::sleep(100);
	xal::mgr->play(S_BARK);
	while (xal::mgr->isAnyPlaying(S_BARK))
	{
		hthread::sleep(100);
		_update(0.1f);
	}
	hthread::sleep(500);
	xal::log("starting stop test");
	xal::mgr->play(S_WIND);
	hthread::sleep(200);
	xal::mgr->play(S_WIND);
	int count = 0;
	while (xal::mgr->isAnyPlaying(S_WIND))
	{
		xal::log(hsprintf("- wind stop iteration: %d", count));
		for (int i = 0; i < 5; i++)
		{
			hthread::sleep(100);
			_update(0.1f);
		}
		xal::mgr->stop(S_WIND);
		count++;
	}
	hthread::sleep(500);
	xal::log("starting stopFirst test");
	xal::mgr->play(S_WIND);
	hthread::sleep(200);
	xal::mgr->play(S_WIND);
	count = 0;
	while (xal::mgr->isAnyPlaying(S_WIND))
	{
		xal::log(hsprintf("- wind stopFirst iteration: %d", count));
		for (int i = 0; i < 5; i++)
		{
			hthread::sleep(100);
			_update(0.1f);
		}
		xal::mgr->stopFirst(S_WIND);
		count++;
	}
	_update(0.1f);
}

void _test_sources()
{
	printf("  - start test sources...\n");
	s->play();
	for (int i = 0; i < XAL_MAX_SOURCES + 1; i++)
	{
		hthread::sleep(20);
		s->play();
	}
	while (s->isPlaying())
	{
		hthread::sleep(100);
		_update(0.1f);
	}
	xal::mgr->update(0.01f);
	s = xal::mgr->createPlayer(S_WIND);
	s->play();
	for (int i = 0; i < 20; i++)
	{
		hthread::sleep(100);
		_update(0.1f);
	}
	s->stop();
	_update(0.1f);
}

void _test_fadein()
{
	printf("  - start test fade in...\n");
	s = xal::mgr->createPlayer(S_WIND);
	s->play(1.0f);
	for (int i = 0; i < 20; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, s->isPlaying() ? "1" : "_", s->isFadingIn() ? "1" : "_", s->isFadingOut() ? "1" : "_");
		_update(0.1f);
	}
	s->stop();
	_update(1.0f);
}

void _test_fadeout()
{
	printf("  - start test fade out...\n");
	s = xal::mgr->createPlayer(S_WIND);
	s->play();
	s->stop(1.0f);
	for (int i = 0; i < 20; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, s->isPlaying() ? "1" : "_", s->isFadingIn() ? "1" : "_", s->isFadingOut() ? "1" : "_");
		_update(0.1f);
	}
	s->stop();
	_update(0.1f);
}

void _test_fadeinout()
{
	printf("  - start test fade in and out...\n");
	s = xal::mgr->createPlayer(S_WIND);
	s->play(1.0f);
	for (int i = 0; i < 8; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, s->isPlaying() ? "1" : "_", s->isFadingIn() ? "1" : "_", s->isFadingOut() ? "1" : "_");
		_update(0.1f);
	}
	s->pause(1.0f);
	for (int i = 0; i < 6; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, s->isPlaying() ? "1" : "_", s->isFadingIn() ? "1" : "_", s->isFadingOut() ? "1" : "_");
		_update(0.1f);
	}
	s->play(1.0f);
	for (int i = 0; i < 3; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, s->isPlaying() ? "1" : "_", s->isFadingIn() ? "1" : "_", s->isFadingOut() ? "1" : "_");
		_update(0.1f);
	}
	printf("- 10 more updates\n");
	for (int i = 0; i < 10; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, s->isPlaying() ? "1" : "_", s->isFadingIn() ? "1" : "_", s->isFadingOut() ? "1" : "_");
		_update(0.1f);
	}
	s->stop();
	_update(1.0f);
}

void _test_complex_handler()
{
	printf("  - start test complex handler...\n");
	xal::Player* temp;
	xal::Player* s1 = xal::mgr->createPlayer(S_WIND);
	xal::Player* s2 = xal::mgr->createPlayer(S_WIND_COPY);
	s1->play();
	s2->play();
	s2->pause();
	for (int i = 0; i < 50; i++)
	{
		hthread::sleep(100);
		_update(0.1f);
		s2->play();
		s1->pause();
		hthread::sleep(100);
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
	list.queueSound(S_BARK);
	list.queueSound(S_BARK);
	list.queueSound(S_WIND);
	list.queueSound(S_WIND_COPY);
	list.queueSound(S_BARK);
	list.play();
	while (list.isPlaying())
	{
		hthread::sleep(100);
		_update(0.1f);
		list.update();
	}
}

void _test_util_parallel_sounds()
{
	printf("  - start test util parallel sounds...\n");
	harray<hstr> names;
	names += S_BARK;
	xal::ParallelSoundManager pmgr;
	pmgr.updateList(names);
	hthread::sleep(1000);
	_update(1.0f);
	names.clear();
	pmgr.updateList(names);
	hthread::sleep(1000);
	_update(1.0f);
}

int main(int argc, char **argv)
{
#ifndef _TEST_THREADED
	xal::init("", false);
#else
	xal::init("", true, 0.01f);
#endif
	harray<hstr> files = xal::mgr->createSoundsFromPath("../media", "sound", "");
#ifndef _TEST_LINKS
#ifdef _TEST_STREAM
	xal::mgr->createCategory("streamable", true);
#endif
	files += xal::mgr->createSoundsFromPath("../media/streamable", "streamable", "");
#else
	xal::mgr->createCategory("cat", true);
	xal::mgr->createSound("../media/linked/linked_sound.xln", "cat");
#endif
	s = xal::mgr->createPlayer(USED_SOUND);

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
