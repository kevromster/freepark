#ifndef TFExport_H
#define TFExport_H

#if defined _WIN32 || defined __CYGWIN__
  #define TF_DLL_IMPORT __declspec(dllimport)
  #define TF_DLL_EXPORT __declspec(dllexport)
  #define TF_DLL_LOCAL
#else
  #if __GNUC__ >= 4
	#define TF_DLL_IMPORT __attribute__ ((visibility ("default")))
	#define TF_DLL_EXPORT __attribute__ ((visibility ("default")))
	#define TF_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
	#define TF_DLL_IMPORT
	#define TF_DLL_EXPORT
	#define TF_DLL_LOCAL
  #endif
#endif

#ifdef TF_DLL // dynamic lib
  #ifdef TF_DLL_EXPORTS
	#define TF_API TF_DLL_EXPORT
  #else
	#define TF_API TF_DLL_IMPORT
  #endif
  #define TF_LOCAL TF_DLL_LOCAL
#else // static lib
  #define TF_API
  #define TF_LOCAL
#endif

#endif // TFExport_H
