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
/// Defines macros for DLL exports/imports.

#ifndef XALUTIL_EXPORT_H
#define XALUTIL_EXPORT_H

	/// @def xalUtilExport
	/// @brief Macro for DLL exports/imports.
	/// @def xalUtilFnExport
	/// @brief Macro for function DLL exports/imports.
	#ifdef _LIB
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
			#define xalUtilFnExport __attribute__ ((visibility("default")))
		#endif
	#endif
	#ifndef DEPRECATED_ATTRIBUTE
		#ifdef _MSC_VER
			#define DEPRECATED_ATTRIBUTE(message) __declspec(deprecated(message))
		#elif !defined(__APPLE__) || __has_extension(attribute_deprecated_with_message) || (defined(__GNUC) && ((GNUC >= 5) || ((GNUC == 4) && (GNUC_MINOR__ >= 5))))
			#define DEPRECATED_ATTRIBUTE(message) __attribute__((deprecated(message)))
		#else
			#define DEPRECATED_ATTRIBUTE(message) __attribute__((deprecated))
		#endif
	#endif

#endif

