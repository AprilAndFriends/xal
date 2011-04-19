/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Provides bacic functionality for XAL.

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
