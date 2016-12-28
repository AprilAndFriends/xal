/// @file
/// @version 3.3
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
#elif defined(__APPLE__)
#define RESOURCE_PATH "media/"
#else
#define RESOURCE_PATH "./"
#endif

#include <stdio.h>
#if defined(_WIN32) && !defined(_WINRT)
#include <windows.h>
#endif

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include <xal/AudioManager.h>
#include <xal/Player.h>
#include <xal/Sound.h>
#include <xal/xal.h>

#define _THREADED_UPDATE

#define CATEGORY_NORMAL "normal"
#define CATEGORY_STREAMED "streamed"
#define SOUND_NAME_NORMAL "bark"
#define SOUND_NAME_STREAMED "wind"
#define PREFIX "streamed/audio_"
#define SOUND_NORMAL SOUND_NAME_NORMAL
#define SOUND_STREAMED PREFIX SOUND_NAME_STREAMED

#ifndef _THREADED_UPDATE
#define __XAL_THREADED false
#else
#define __XAL_THREADED true
#endif

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
	// initialize XAL with platform default audio system, threaded update 100 times per second (every 0.01 seconds)
	xal::init(xal::AudioSystemType::Default, hwnd, __XAL_THREADED, 0.01f);

	// create a category for fully loaded and decoded sounds
	xal::manager->createCategory(CATEGORY_NORMAL, xal::BufferMode::OnDemand, xal::SourceMode::Disk);
	// create a category for streamed sounds
	xal::manager->createCategory(CATEGORY_STREAMED, xal::BufferMode::Streamed, xal::SourceMode::Disk);
	// create sound using a prefix (forces usage of the ogg files)
	xal::manager->createSound(RESOURCE_PATH SOUND_NAME_NORMAL ".ogg", CATEGORY_NORMAL);
	// create streamed sound using no prefix (forces usage of the ogg files)
	xal::manager->createSound(RESOURCE_PATH "streamable/" SOUND_NAME_STREAMED ".ogg", CATEGORY_STREAMED, PREFIX);
	xal::Player* p = NULL;
	
	// create a sound player for manual control
	p = xal::manager->createPlayer(SOUND_NORMAL);
	// play the sound
	hlog::write("", "- starting " SOUND_NORMAL);
	p->play();
	p->setPitch(0.5f);
	while (p->isPlaying())
	{
		hlog::writef("", "    - " SOUND_NORMAL " - samples: %d - time: %f", p->getSamplePosition(), p->getTimePosition());
		hthread::sleep(100);
		xal::manager->update(0.1f);
	}
	hlog::write("", "- finished " SOUND_NORMAL);
	// destroy the player
	xal::manager->destroyPlayer(p);

	// create a new sound player
	p = xal::manager->createPlayer(SOUND_STREAMED);
	// play the sound
	hlog::write("", "- starting " SOUND_STREAMED);
	p->play();
	while (p->isPlaying())
	{
		hlog::writef("", "    - " SOUND_STREAMED " - samples: %d - time: %f", p->getSamplePosition(), p->getTimePosition());
		hthread::sleep(100);
		xal::manager->update(0.1f);
	}
	hlog::write("", "- finished " SOUND_STREAMED);
	// destroy the player
	xal::manager->destroyPlayer(p);

	// fire & forget, no control over the sound
	xal::manager->play(SOUND_NORMAL);
	hlog::write("", "- starting " SOUND_NORMAL);
	while (xal::manager->isAnyPlaying(SOUND_NORMAL))
	{
		hthread::sleep(100);
		xal::manager->update(0.1f);
	}
	hlog::write("", "- finished " SOUND_NORMAL);

	// destroying the sounds manually
	xal::manager->destroySound(xal::manager->getSound(SOUND_NORMAL));
	xal::manager->destroySound(xal::manager->getSound(SOUND_STREAMED));
	// destroying XAL itself
	xal::destroy();
#ifndef _WINRT
	system("pause");
#endif
	return 0;
}
