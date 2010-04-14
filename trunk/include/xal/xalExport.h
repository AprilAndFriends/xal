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

