/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XALUTIL_EXPORT_H
#define XALUTIL_EXPORT_H

	#ifdef _STATICLIB
		#define xalUtilExport
		#define xalUtilFnExport
	#else
		#ifdef _WIN32
			#ifdef XALUTIL_EXPORTS
				#define xalUtilExport __declspec(dllexport)
				#define xalUtilFnExport __declspec(dllexport)
			#else
				#define xalUtilExport __declspec(dllimport)
				#define xalUtilFnExport __declspec(dllimport)
			#endif
		#else
			#define xalUtilExport __attribute__ ((visibility("default")))
			#define xalUtilFnExport
		#endif
	#endif

#endif

