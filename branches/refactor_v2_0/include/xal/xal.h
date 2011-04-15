/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic, Ivan Vucica                           *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_H
#define XAL_H

#include <hltypes/hstring.h>

#include "xalExport.h"

namespace xal
{
	xalFnExport void init(chstr deviceName = "", bool threaded = true, float updateTime = 0.01f);
	xalFnExport void destroy();
	xalFnExport void setLogFunction(void (*function)(chstr));
	xalFnExport void log(chstr message, chstr prefix = "[xal] ");

}

#endif
