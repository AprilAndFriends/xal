/// @file
/// @author  Boris Mikic
/// @version 2.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

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

//#define _USE_STREAM
//#define _USE_LINKS
//#define _USE_THREADING
#define _USE_MEMORY_MANAGEMENT

//#define _TEST_BASIC
//#define _TEST_SOUND
//#define _TEST_MULTIPLAY
//#define _TEST_HANDLE_STREAM
//#define _TEST_FADE_IN
//#define _TEST_FADE_OUT
//#define _TEST_FADE_IN_OUT
//#define _TEST_COMPLEX_HANDLER
#define _TEST_MEMORY_MANAGEMENT

//#define _TEST_SOURCE_HANDLING // usually OpenAL only

//#define _TEST_UTIL_PLAYLIST
//#define _TEST_UTIL_PARALLEL_SOUNDS

#ifndef _USE_LINKS
#define S_BARK "bark"
#define S_WIND "wind"
#define S_WIND_2 "wind_2"
#ifndef _USE_STREAM
#define USED_SOUND S_BARK
#else
#define USED_SOUND S_WIND
#endif
#else
#define USED_SOUND "linked_sound"
#define S_BARK "bark"
#define S_WIND "linked_sound"
#define S_WIND_2 "wind_2"
#endif
#ifdef _USE_MEMORY_MANAGEMENT
#define MEMORY_MANGEMENT_ENABLED true
#else
#define MEMORY_MANGEMENT_ENABLED false
#endif

#define OPENAL_MAX_SOURCES 16 // needed when using OpenAL

void _test_basic(xal::Player* player)
{
	printf("  - start test basic...\n");
	player->play();
	while (player->isPlaying())
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
	}
}

void _test_sound(xal::Player* player)
{
	printf("  - start test sound...\n");
	player->play();
	for (int i = 0; i < 20; i++)
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
		player->pause();
		hthread::sleep(100);
		xal::mgr->update(0.1f);
		player->play();
	}
	xal::mgr->update(1.0f);
}

void _test_multiplay(xal::Player* player)
{
	printf("  - start test multiple play...\n");
	xal::mgr->play(S_BARK);
	hthread::sleep(100);
	xal::mgr->play(S_BARK);
	while (xal::mgr->isAnyPlaying(S_BARK))
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
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
			xal::mgr->update(0.1f);
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
			xal::mgr->update(0.1f);
		}
		xal::mgr->stopFirst(S_WIND);
		count++;
	}
	xal::mgr->update(0.1f);
}

void _test_handle_stream(xal::Player* player)
{
	printf("  - start test handle stream...\n");
	player->play();
	hthread::sleep(200);
	xal::mgr->update(0.2f);
	for (int i = 0; i < 5; i++)
	{
		printf("  - play %d\n", i);
		player->play();
		hthread::sleep(1000);
		xal::mgr->update(1.0f);
		if (i == 1)
		{
			printf("  - fade %d\n", i);
			player->pause(1.2f);
			for (int j = 0; j < 10; j++)
			{
				hthread::sleep(100);
				xal::mgr->update(0.1f);
			}
		}
		else if (i == 3)
		{
			printf("  - pause %d\n", i);
			player->pause();
			for (int j = 0; j < 10; j++)
			{
				hthread::sleep(100);
				xal::mgr->update(0.1f);
			}
		}
	}
	printf("  - stop\n");
	player->stop();
	xal::mgr->update(0.1f);
}

void _test_fadein(xal::Player* player)
{
	printf("  - start test fade in...\n");
	xal::Player* p1 = xal::mgr->createPlayer(S_WIND);
	p1->play(1.0f);
	for (int i = 0; i < 20; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	p1->stop();
	xal::mgr->update(1.0f);
}

void _test_fadeout(xal::Player* player)
{
	printf("  - start test fade out...\n");
	xal::Player* p1 = xal::mgr->createPlayer(S_WIND);
	p1->play();
	p1->stop(1.0f);
	for (int i = 0; i < 20; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	p1->stop();
	xal::mgr->update(0.1f);
}

void _test_fadeinout(xal::Player* player)
{
	printf("  - start test fade in and out...\n");
	xal::Player* p1 = xal::mgr->createPlayer(S_WIND);
	p1->play(1.0f);
	for (int i = 0; i < 8; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	p1->pause(1.0f);
	for (int i = 0; i < 6; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	p1->play(1.0f);
	for (int i = 0; i < 3; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	printf("- 10 more updates\n");
	for (int i = 0; i < 10; i++)
	{
		hthread::sleep(100);
		printf("T:%d P:%s FI:%s FO:%s\n", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	p1->stop();
	xal::mgr->update(1.0f);
}

void _test_complex_handler(xal::Player* player)
{
	printf("  - start test complex handler...\n");
	xal::Player* temp;
	xal::Player* p1 = xal::mgr->createPlayer(S_WIND);
	xal::Player* p2 = xal::mgr->createPlayer(S_WIND_2);
	p1->play();
	p2->play();
	p2->pause();
	for (int i = 0; i < 50; i++)
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
		p2->play();
		p1->pause();
		hthread::sleep(100);
		xal::mgr->update(0.1f);
		p2->pause();
		p1->play();
		if (i % 3 == 0)
		{
			temp = p1;
			p1 = p2;
			p2 = temp;
		}
	}
	p1->stop();
	p2->stop();
	xal::mgr->destroyPlayer(p1);
	xal::mgr->destroyPlayer(p2);
	xal::mgr->update(0.1f);
}

void _test_memory_management(xal::Player* player)
{
	printf("  - start test memory management...\n");
	xal::mgr->setIdlePlayerUnloadTime(0.5f);
	player->play();
	hthread::sleep(200);
	xal::mgr->update(0.2f);
	player->pause();
	hthread::sleep(1000);
	xal::mgr->update(1.0f);
	player->stop();
	printf("expecting sound reload because on ON_DEMAND...\n");
	player->play();
	hthread::sleep(200);
	xal::mgr->update(0.2f);
	player->stop();
	printf("expecting automatic memory clearing now...\n");
	hthread::sleep(1000);
	xal::mgr->update(1.0f);
	xal::Player* p1 = xal::mgr->createPlayer(S_WIND);
	p1->play();
	player->play();
	hthread::sleep(200);
	xal::mgr->update(0.2f);
	p1->pause();
	player->pause();
	printf("trying to clear memory on paused sounds...\n");
	xal::mgr->clearMemory();
	hthread::sleep(1000);
	xal::mgr->update(0.5f);
	p1->stop();
	player->stop();
	printf("trying to clear memory on stopped sounds...\n");
	xal::mgr->clearMemory();
	xal::mgr->destroyPlayer(p1);
}

void _test_sources(xal::Player* player)
{
	printf("  - start test sources...\n");
	for (int i = 0; i < OPENAL_MAX_SOURCES + 1; i++)
	{
		xal::mgr->play(S_BARK);
		hthread::sleep(20);
	}
	while (xal::mgr->isAnyPlaying(S_BARK))
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
	}
	xal::mgr->update(0.01f);
	xal::Player* p1 = xal::mgr->createPlayer(S_WIND);
	p1->play();
	for (int i = 0; i < 20; i++)
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
	}
	p1->stop();
	xal::mgr->update(0.1f);
}

void _test_util_playlist(xal::Player* player)
{
	printf("  - start test util playlist...\n");
	xal::Playlist list(false);
	list.queueSound(S_BARK);
	list.queueSound(S_BARK);
	list.queueSound(S_WIND);
	list.queueSound(S_WIND_2);
	list.queueSound(S_BARK);
	list.play();
	while (list.isPlaying())
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
		list.update();
	}
}

void _test_util_parallel_sounds(xal::Player* player)
{
	printf("  - start test util parallel sounds...\n");
	harray<hstr> names;
	names += S_BARK;
	names += S_WIND;
	xal::ParallelSoundManager pmgr(0.25f);
	pmgr.updateList(names);
	for (int i = 0; i < 100000; i++)
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
	}
	names.clear();
	pmgr.updateList(names);
	for (int i = 0; i < 10; i++)
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
	}
}

int main(int argc, char **argv)
{
	void* hwnd = 0;
#ifdef _WIN32
	hwnd = GetConsoleWindow();
#endif
#ifndef _USE_THREADING
	xal::init(XAL_AS_DEFAULT, hwnd, false);
#else
	xal::init(XAL_AS_DEFAULT, hwnd, true, 0.01f);
#endif
#ifdef _USE_MEMORY_MANAGEMENT
	xal::mgr->createCategory("sound", xal::ON_DEMAND, xal::ON_DEMAND, MEMORY_MANGEMENT_ENABLED);
#endif
	harray<hstr> files = xal::mgr->createSoundsFromPath("../media", "sound", "");
#ifndef _USE_LINKS
#ifndef _USE_STREAM
	xal::mgr->createCategory("streamable", xal::LAZY, xal::LAZY, MEMORY_MANGEMENT_ENABLED);
#else
	xal::mgr->createCategory("streamable", xal::STREAMED, xal::STREAMED, MEMORY_MANGEMENT_ENABLED);
#endif
	files += xal::mgr->createSoundsFromPath("../media/streamable", "streamable", "");
#else
	xal::mgr->createCategory("cat", xal::FULL, xal::FULL, MEMORY_MANGEMENT_ENABLED);
	xal::mgr->createSound("../media/linked/linked_sound.xln", "cat");
#endif
	xal::Player* player = xal::mgr->createPlayer(USED_SOUND);

#ifdef _TEST_BASIC
	_test_basic(player);
#endif
#ifdef _TEST_SOUND
	_test_sound(player);
#endif
#ifdef _TEST_HANDLE_STREAM
	_test_handle_stream(player);
#endif
#ifdef _TEST_MULTIPLAY
	_test_multiplay(player);
#endif
#ifdef _TEST_FADE_IN
	_test_fadein(player);
#endif
#ifdef _TEST_FADE_OUT
	_test_fadeout(player);
#endif
#ifdef _TEST_FADE_IN_OUT
	_test_fadeinout(player);
#endif
#ifdef _TEST_COMPLEX_HANDLER
	_test_complex_handler(player);
#endif
#ifdef _TEST_MEMORY_MANAGEMENT
	_test_memory_management(player);
#endif
#ifdef _TEST_SOURCE_HANDLING
	_test_sources(player);
#endif
#ifdef _TEST_UTIL_PLAYLIST
	_test_util_playlist(player);
#endif
#ifdef _TEST_UTIL_PARALLEL_SOUNDS
	_test_util_parallel_sounds(player);
#endif

	xal::mgr->destroyPlayer(player);
	printf("  - done\n");
	xal::destroy();
	system("pause");
	return 0;
}
