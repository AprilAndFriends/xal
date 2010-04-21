/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_DUMMY_SOUND_H
#define XAL_DUMMY_SOUND_H

#include "Sound.h"
#include "xalExport.h"

namespace xal
{

	class xalExport DummySound : public Sound
	{
	public:
		DummySound(std::string filename);
		~DummySound();
	};

}
#endif
