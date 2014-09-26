#pragma once

#ifndef UTILITY_DLL
	#define UTILITY_API			__declspec( dllimport )
	#define UTILITY_DATA		__declspec( dllimport )
	#define UTILITY_CLASS		AFX_CLASS_IMPORT
#else
	#define UTILITY_API			__declspec( dllexport )
	#define UTILITY_DATA		__declspec( dllexport )
	#define UTILITY_CLASS		AFX_CLASS_EXPORT
#endif