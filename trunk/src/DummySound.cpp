/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include "DummySound.h"
#include "SoundManager.h"

namespace xal
{

	DummySound::DummySound(std::string filename) : Sound(filename)
	{
		SoundManager::getSingleton().logMessage("creating dummy sound: "+filename);
	}

	DummySound::~DummySound()
	{
	}
}
