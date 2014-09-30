/// @file
/// @version 3.05
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifndef _ANDROID
#ifndef _WINRT
#define RESOURCE_PATH "../../demos/media/"
#else
#define RESOURCE_PATH "media/"
#endif
#else
#define RESOURCE_PATH "./"
#endif

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include <xal/AudioManager.h>
#include <xal/Player.h>
#include <xal/xal.h>
#include <xalutil/ParallelSoundManager.h>
#include <xalutil/Playlist.h>

#define _USE_STREAM
//#define _USE_LINKS
//#define _USE_THREADING
//#define _USE_MEMORY_MANAGEMENT
//#define _USE_RAM_SOURCES

#define _TEST_BASIC
//#define _TEST_SOUND
//#define _TEST_MULTIPLAY
//#define _TEST_HANDLE_STREAM
//#define _TEST_FADE_IN
//#define _TEST_FADE_OUT
//#define _TEST_FADE_IN_OUT
//#define _TEST_COMPLEX_HANDLER
//#define _TEST_MEMORY_MANAGEMENT

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
#ifdef _USE_RAM_SOURCES
#define SOURCE_MODE xal::RAM
#else
#define SOURCE_MODE xal::DISK
#endif

#define OPENAL_MAX_SOURCES 16 // needed when using OpenAL

void _test_basic(xal::Player* player)
{
	hlog::write("", "  - start test basic...");
	player->play();
	while (player->isPlaying())
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
	}
}

void _test_sound(xal::Player* player)
{
	hlog::write("", "  - start test sound...");
	player->play();
	for_iter (i, 0, 20)
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
		player->pause();

		hlog::writef("", "  - PAUSE: %02d - %05d %7.3f", i, player->getSamplePosition(), player->getTimePosition());

		hthread::sleep(100);
		xal::mgr->update(0.1f);
		player->play();

		hlog::writef("", "  - PLAY:  %02d - %05d %7.3f", i, player->getSamplePosition(), player->getTimePosition());
	}
	xal::mgr->update(1.0f);
}

void _test_multiplay(xal::Player* player)
{
	hlog::write("", "  - start test multiple play...");
	xal::mgr->play(S_BARK);
	hthread::sleep(100);
	xal::mgr->play(S_BARK);
	while (xal::mgr->isAnyPlaying(S_BARK))
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
	}
	hthread::sleep(500);
	hlog::write("", "starting stop test");
	xal::mgr->play(S_WIND);
	hthread::sleep(200);
	xal::mgr->play(S_WIND);
	int count = 0;
	while (xal::mgr->isAnyPlaying(S_WIND))
	{
		hlog::writef("", "- wind stop iteration: %d", count);
		for_iter (i, 0, 5)
		{
			hthread::sleep(100);
			xal::mgr->update(0.1f);
		}
		xal::mgr->stop(S_WIND);
		count++;
	}
	hthread::sleep(500);
	hlog::write("", "starting stopFirst test");
	xal::mgr->play(S_WIND);
	hthread::sleep(200);
	xal::mgr->play(S_WIND);
	count = 0;
	while (xal::mgr->isAnyPlaying(S_WIND))
	{
		hlog::writef("", "- wind stopFirst iteration: %d", count);
		for_iter (i, 0, 5)
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
	hlog::write("", "  - start test handle stream...");
	player->play();
	hthread::sleep(200);
	xal::mgr->update(0.2f);
	for_iter (i, 0, 5)
	{
		hlog::writef("", "  - play %d", i);
		player->play();
		hthread::sleep(1000);
		xal::mgr->update(1.0f);
		if (i == 1)
		{
			hlog::writef("", "  - fade %d", i);
			player->pause(1.2f);
			for_iter (j, 0, 10)
			{
				hthread::sleep(100);
				xal::mgr->update(0.1f);
			}
		}
		else if (i == 3)
		{
			hlog::writef("", "  - pause %d", i);
			player->pause();
			for_iter (j, 0, 10)
			{
				hthread::sleep(100);
				xal::mgr->update(0.1f);
			}
		}
	}
	hlog::write("", "  - stop");
	player->stop();
	xal::mgr->update(0.1f);
}

void _test_fadein(xal::Player* player)
{
	hlog::write("", "  - start test fade in...");
	xal::Player* p1 = xal::mgr->createPlayer(S_WIND);
	p1->play(1.0f);
	for_iter (i, 0, 20)
	{
		hthread::sleep(100);
		hlog::writef("", "T:%d P:%s FI:%s FO:%s", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	p1->stop();
	xal::mgr->update(1.0f);
}

void _test_fadeout(xal::Player* player)
{
	hlog::write("", "  - start test fade out...");
	xal::Player* p1 = xal::mgr->createPlayer(S_WIND);
	p1->play();
	p1->stop(1.0f);
	for_iter (i, 0, 20)
	{
		hthread::sleep(100);
		hlog::writef("", "T:%d P:%s FI:%s FO:%s", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	p1->stop();
	xal::mgr->update(0.1f);
}

void _test_fadeinout(xal::Player* player)
{
	hlog::write("", "  - start test fade in and out...");
	xal::Player* p1 = xal::mgr->createPlayer(S_WIND);
	p1->play(1.0f);
	for_iter (i, 0, 8)
	{
		hthread::sleep(100);
		hlog::writef("", "T:%d P:%s FI:%s FO:%s", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	p1->pause(1.0f);
	for_iter (i, 0, 6)
	{
		hthread::sleep(100);
		hlog::writef("", "T:%d P:%s FI:%s FO:%s", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	p1->play(1.0f);
	for_iter (i, 0, 3)
	{
		hthread::sleep(100);
		hlog::writef("", "T:%d P:%s FI:%s FO:%s", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	hlog::write("", "- 10 more updates");
	for_iter (i, 0, 10)
	{
		hthread::sleep(100);
		hlog::writef("", "T:%d P:%s FI:%s FO:%s", i, p1->isPlaying() ? "1" : "_", p1->isFadingIn() ? "1" : "_", p1->isFadingOut() ? "1" : "_");
		xal::mgr->update(0.1f);
	}
	p1->stop();
	xal::mgr->update(1.0f);
}

void _test_complex_handler(xal::Player* player)
{
	hlog::write("", "  - start test complex handler...");
	xal::Player* temp;
	xal::Player* p1 = xal::mgr->createPlayer(S_WIND);
	xal::Player* p2 = xal::mgr->createPlayer(S_WIND_2);
	p1->play();
	p2->play();
	p2->pause();
	for_iter (i, 0, 50)
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
	hlog::write("", "  - start test memory management...");
	xal::mgr->setIdlePlayerUnloadTime(0.5f);
	player->play();
	hthread::sleep(200);
	xal::mgr->update(0.2f);
	player->pause();
	hthread::sleep(1000);
	xal::mgr->update(1.0f);
	player->stop();
	player->play();
	hthread::sleep(200);
	xal::mgr->update(0.2f);
	player->stop();
	hlog::write("", "expecting automatic memory clearing now...");
	hthread::sleep(1000);
	xal::mgr->update(1.0f);
	xal::Player* p1 = xal::mgr->createPlayer(S_WIND);
	p1->play();
	player->play();
	hthread::sleep(200);
	xal::mgr->update(0.2f);
	p1->pause();
	player->pause();
	hlog::write("", "trying to clear memory on paused sounds...");
	xal::mgr->clearMemory();
	hthread::sleep(1000);
	xal::mgr->update(0.5f);
	p1->stop();
	player->stop();
	hlog::write("", "trying to clear memory on stopped sounds...");
	xal::mgr->clearMemory();
	xal::mgr->destroyPlayer(p1);
}

void _test_sources(xal::Player* player)
{
	hlog::write("", "  - start test sources...");
	for_iter (i, 0, OPENAL_MAX_SOURCES + 1)
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
	for_iter (i, 0, 20)
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
	}
	p1->stop();
	xal::mgr->update(0.1f);
}

void _test_util_playlist(xal::Player* player)
{
	hlog::write("", "  - start test util playlist...");
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
	hlog::write("", "  - start test util parallel sounds...");
	harray<hstr> names;
	names += S_BARK;
	names += S_WIND;
	xal::ParallelSoundManager pmgr(0.25f);
	pmgr.updateList(names);
	for_iter (i, 0, 1000)
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
	}
	names.clear();
	pmgr.updateList(names);
	for_iter (i, 0, 10)
	{
		hthread::sleep(100);
		xal::mgr->update(0.1f);
	}
}

#ifndef _WINRT
int main(int argc, char **argv)
#else
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ args)
#endif
{
	void* hwnd = 0;
#if defined(_WIN32) && !defined(_WINRT)
	hwnd = GetConsoleWindow();
#endif
#ifndef _USE_THREADING
	xal::init(xal::AS_DEFAULT, hwnd, false);
#else
	xal::init(xal::AS_DEFAULT, hwnd, true, 0.01f);
#endif
#ifdef _USE_MEMORY_MANAGEMENT
	xal::mgr->createCategory("sound", xal::MANAGED, SOURCE_MODE);
#endif
	harray<hstr> files = xal::mgr->createSoundsFromPath(RESOURCE_PATH, "sound", "");
#ifndef _USE_LINKS
#ifndef _USE_STREAM
	xal::mgr->createCategory("streamable", xal::MANAGED, SOURCE_MODE);
#else
	xal::mgr->createCategory("streamable", xal::STREAMED, SOURCE_MODE);
#endif
	files += xal::mgr->createSoundsFromPath(RESOURCE_PATH "streamable", "streamable", "");
#else
#ifdef _USE_MEMORY_MANAGEMENT
	xal::mgr->createCategory("cat", xal::MANAGED, SOURCE_MODE);
#else
	xal::mgr->createCategory("cat", xal::FULL, SOURCE_MODE);
#endif
	xal::mgr->createSound(RESOURCE_PATH "linked/linked_sound.xln", "cat");
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
	hlog::write("", "  - done");
	xal::destroy();
#ifndef _WINRT
	system("pause");
#endif
	return 0;
}
