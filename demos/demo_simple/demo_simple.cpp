/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

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
#define _update(time) xal::mgr->update(time)

int main(int argc, char **argv)
{
	unsigned long hwnd = 0;
#ifdef _WIN32
	hwnd = (unsigned long)GetConsoleWindow();
#endif
	// initialize XAL with platform default audio system, optional threaded update and 100 times per second
	xal::init(XAL_AS_DEFAULT, hwnd, __XAL_THREADED, 0.01f);

	// create a category for fully loaded and decoded sounds
	xal::mgr->createCategory(CATEGORY_NORMAL, xal::FULL, xal::FULL);
	// create a category for streamed sounds
	xal::mgr->createCategory(CATEGORY_STREAMED, xal::STREAMED, xal::STREAMED);
	// create sound using a prefix
	xal::mgr->createSound("../media/" SOUND_NAME_NORMAL ".ogg", CATEGORY_NORMAL);
	// create streamed sound using no prefix
	xal::mgr->createSound("../media/streamable/" SOUND_NAME_STREAMED ".ogg", CATEGORY_STREAMED, PREFIX);
	xal::Player* p;

	// create a sound player for manual control
	p = xal::mgr->createPlayer(SOUND_NORMAL);
	// play the sound
	printf(("- starting " SOUND_NORMAL "\n"));
	p->play();
	p->setPitch(0.5f);
	while (p->isPlaying())
	{
		hthread::sleep(10);
		_update(0.01f);
	}
	printf(("- finished " SOUND_NORMAL "\n"));
	// destroy the player
	xal::mgr->destroyPlayer(p);

	// create a new sound player
	p = xal::mgr->createPlayer(SOUND_STREAMED);
	// play the sound
	printf(("- starting " SOUND_STREAMED "\n"));
	p->play();
	while (p->isPlaying())
	{
		hthread::sleep(10);
		_update(0.01f);
	}
	printf(("- finished " SOUND_STREAMED "\n"));
	// destroy the player
	xal::mgr->destroyPlayer(p);

	// fire & forget, no control over the sound
	xal::mgr->play(SOUND_NORMAL);
	printf(("- starting " SOUND_NORMAL "\n"));
	while (xal::mgr->isAnyPlaying(SOUND_NORMAL))
	{
		hthread::sleep(10);
		_update(0.01f);
	}
	printf(("- finished " SOUND_NORMAL "\n"));

	// destroying the sounds manually
	xal::mgr->destroySound(xal::mgr->getSound(SOUND_NORMAL));
	xal::mgr->destroySound(xal::mgr->getSound(SOUND_STREAMED));
	// destroying XAL itself
	xal::destroy();
	system("pause");
	return 0;
}
