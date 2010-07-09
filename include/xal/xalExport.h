/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_EXPORT_H
#define XAL_EXPORT_H

	#ifdef _STATICLIB
		#define xalExport
		#define xalFnExport
	#else
		#ifdef _WIN32
			#ifdef XAL_EXPORTS
				#define xalExport __declspec(dllexport)
				#define xalFnExport __declspec(dllexport)
			#else
				#define xalExport __declspec(dllimport)
				#define xalFnExport __declspec(dllimport)
			#endif
		#else
			#define xalExport __attribute__ ((visibility("default")))
			#define xalFnExport
		#endif
	#endif

#endif

