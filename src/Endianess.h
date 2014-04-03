/// @file
/// @author  Kresimir Spes
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Fixes endianess.

#ifndef XAL_NORMALIZE_ENDIAN

#include <hltypes/exception.h>

#ifdef __BIG_ENDIAN__
	// ppc & friends need convert from littleendian to their bigendian
#define XAL_NORMALIZE_ENDIAN(variable) \
	variable = (sizeof(variable) == 1 ? \
		(variable) : \
		sizeof(variable) == 2 ? \
			(((variable) & 0xFF) << 8) | \
			(((variable) & 0xFF00) >> 8) : \
		sizeof(variable) == 4 ? \
			(((variable) & 0xFF) << 24) | \
			(((variable) & 0xFF00) << 8) | \
			(((variable) & 0xFF0000) >> 8) | \
			(((variable) & 0xFF000000) >> 24) : \
		throw hl_exception("Unsupported sizeof(" # variable ")\n") \
	);
#define XAL_NORMALIZE_FLOAT_ENDIAN(variable) \
	{ \
		uint32_t _var = *(uint32_t*)&variable; \
		XAL_NORMALIZE_ENDIAN(_var); \
		variable = *(float*)&_var; \
	}	   
				
#else
	// i386 & friends do a noop
	#define XAL_NORMALIZE_ENDIAN(variable)	  
	#define XAL_NORMALIZE_FLOAT_ENDIAN(variable)
#endif

#endif
