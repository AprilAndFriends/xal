/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Internal utility header.

#ifndef XAL_UTILITY_H
#define XAL_UTILITY_H

#ifndef _ANDROID
	#define STREAM_BUFFER_COUNT 8 // greater or equal to 2
	#define STREAM_BUFFER_SIZE 32768 // equal to any power of 2
#else // Android has to be more restrictive about audio streaming due to various memory problems
	#define STREAM_BUFFER_COUNT 4 // greater or equal to 2
	#define STREAM_BUFFER_SIZE 4096 // equal to any power of 2
#endif
#define STREAM_BUFFER (STREAM_BUFFER_COUNT * STREAM_BUFFER_SIZE)

#endif
