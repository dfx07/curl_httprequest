#pragma once


#ifdef KYHTTP_EXPORTS
	#define KYHTTP_API __declspec(dllexport)
#else
#ifdef KYHTTP_USE_DLL
	#define KYHTTP_API __declspec(dllimport)
#else
	#define KYHTTP_API
#endif
#endif


#include "kyhttp_curl.h"


KYHTTP_API HANDLE __cdecl KyHttp_Create()
{
	kyhttp::HttpClient*client = new kyhttp::HttpClient();
	return client;
}

KYHTTP_API void __cdecl KyHttp_Destroy(PHANDLE phHandle)
{
	if (phHandle && *phHandle)
	{
		delete (kyhttp::HttpClient*)*phHandle;
		*phHandle = NULL;
	}
}

KYHTTP_API kyhttp::HttpErrorCode __cdecl KyHttp_RequestMessage(kyhttp::Uri uri)
{

}

