/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_SIMPLESOUND_H
#define XAL_SIMPLESOUND_H

#include <hltypes/hstring.h>
#include "xalExport.h"
#include "SoundBuffer.h"

namespace xal
{
	class xalExport SimpleSound : public SoundBuffer
	{
	public:
		SimpleSound(chstr name, chstr category, chstr prefix = "");
		~SimpleSound();
		
		unsigned int getBuffer() const { return this->buffer; }
		
	protected:
		unsigned int buffer;
		
		bool _loadOgg();
		
	};

}

#endif
