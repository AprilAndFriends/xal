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

int main(int argc, char **argv)
{
	xal::init();
	xal::Sound* s = audioMgr->loadSound("../media/bark.ogg", "music");
	s->play();
	while (s->isPlaying()) { }
	s->play();
	while (s->isPlaying()) { }
	s->play();
	while (s->isPlaying()) { }
	
	xal::destroy();
	return 0;
}
