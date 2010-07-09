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
	//xal::Sound* s = audiomgr->loadSound("../media/bark.ogg", "music");
	
	//audiomgr->loadCategoryPath("../media", "test.");
	//xal::Sound* s = audiomgr->getSound("test.bark", "media");
	
	//audiomgr->loadCategoryPath("../media");
	//xal::Sound* s = audiomgr->getSound("bark", "media");
	
	printf("\n");
	harray<hstr> files = audiomgr->loadPath("..");
	for (hstr* it = files.iterate(); it; it = files.next())
	{
		hstr file = "../" + (*it);
		printf("%s\n", (*it).c_str());
		//printf("%s\n", (*it).c_str());
	}
	printf("\n");
	
	xal::Sound* s = audiomgr->loadSound("../media/bark.ogg", "media");
	//xal::Sound* s = audiomgr->getSound("bark", "media");
	
	s->play();
	while (s->isPlaying()) { }
	s->play();
	while (s->isPlaying()) { }
	s->play();
	while (s->isPlaying()) { }
	
	xal::destroy();
	return 0;
}
