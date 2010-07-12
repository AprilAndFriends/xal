/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <stdio.h>
#include <xal/AudioManager.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

int main(int argc, char **argv)
{
	xal::init();
	
	harray<hstr> files = audiomgr->loadPathCategory("../media", "cat");
	xal::Sound* s = audiomgr->getSound("bark");
	
	//harray<hstr> files = audiomgr->loadPathCategory("../media", "cat", "test.");
	//xal::Sound* s = audiomgr->getSound("test.bark");
	
	//harray<hstr> files = audiomgr->loadPath("..");
	//xal::Sound* s = audiomgr->getSound("bark");
	
	//harray<hstr> files = audiomgr->loadPath("..", "test");
	//xal::Sound* s = audiomgr->getSound("testbark");
	
	s->play();
	while (s->isPlaying()) { }
	audiomgr->setCategoryGain("cat", 0.5f);
	s->play();
	while (s->isPlaying()) { }
	audiomgr->setCategoryGain("cat", 1.0f);
	s->play();
	while (s->isPlaying()) { }
	
	xal::destroy();
	return 0;
}
