/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_STREAMSOUND_H
#define XAL_STREAMSOUND_H

#include <hltypes/hstring.h>
#include "xalExport.h"
#include "Sound.h"

namespace xal
{
	class xalExport StreamSound : public Sound
	{
	public:
		StreamSound(chstr name, chstr category, chstr prefix = "");
		~StreamSound();

	};

}

#endif
