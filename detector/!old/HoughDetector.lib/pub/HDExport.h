#ifndef HDExport_H
#define HDExport_H

#if defined _WIN32 || defined __CYGWIN__
  #define HD_DLL_IMPORT __declspec(dllimport)
  #define HD_DLL_EXPORT __declspec(dllexport)
  #define HD_DLL_LOCAL
#else
  #if __GNUC__ >= 4
	#define HD_DLL_IMPORT __attribute__ ((visibility ("default")))
	#define HD_DLL_EXPORT __attribute__ ((visibility ("default")))
	#define HD_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
	#define HD_DLL_IMPORT
	#define HD_DLL_EXPORT
	#define HD_DLL_LOCAL
  #endif
#endif

#ifdef HD_DLL // dynamic lib
  #ifdef HD_DLL_EXPORTS
	#define HD_API HD_DLL_EXPORT
  #else
	#define HD_API HD_DLL_IMPORT
  #endif
  #define HD_LOCAL HD_DLL_LOCAL
#else // static lib
  #define HD_API
  #define HD_LOCAL
#endif

#endif // HDExport_H
