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

#ifndef XAL_EXPORT_H
#define XAL_EXPORT_H

	/// @def xalExport
	/// @brief Macro for DLL exports/imports.
	/// @def xalFnExport
	/// @brief Macro for function DLL exports/imports.
	#ifdef _LIB
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
			#define xalFnExport __attribute__ ((visibility("default")))
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

