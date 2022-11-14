/*!**********************************************************************
* @copyright Copyright (C) 2022 thuong.nv -email: mark.ngo@kohyoung.com.\n
*            All rights reserved.
*************************************************************************
* @file     kyhttp.h
* @date     Jul 11, 2022
* @brief    HTTP client libcurl implementation file.
*
** HTTP client implementation file using LibCurl.
*************************************************************************/
#pragma once

#include <iostream>
#include <Windows.h>

enum tracker_state
{
	eLogger_None	 = 0x0000,
	eLogger_Error	 = 0x0001,
	eLogger_Warning = 0x0002,
	eLogger_Assert  = 0x0004,
	eLogger_Info    = 0x0008,
	eLogger_Trace   = 0x0009,
};


#define KY_HTTP_MAX_LENGTH_MSG_LOG 2048

/******************************************************************************
*! @brief  : tracker function support
*! @return : void
*! @author : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/
std::string logger_to_utf8(const wchar_t* mb, const int& nsize)
{
	std::string utf8;
	utf8.resize(nsize + sizeof(wchar_t), 0);
	int nbytes = WideCharToMultiByte(CP_UTF8, 0, mb, (int)nsize / sizeof(wchar_t),
		(LPSTR)utf8.c_str(), (int)utf8.size(), NULL, NULL);
	utf8.resize(nbytes);
	return utf8;
}

/******************************************************************************
*! @brief  : tracker get date time now in system local
*! @return : void
*! @author : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/
void logger_get_datetime(wchar_t* buff)
{
	SYSTEMTIME SystemTime;
	GetLocalTime(&SystemTime);

	const size_t buffsize = 200;
	wchar_t bufftime[buffsize];

	memset(bufftime, 0, buffsize);
	swprintf_s(bufftime, L"%04d-%02d-%02d %02d:%02d:%02d:%03d",
		SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
		SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond,
		SystemTime.wMilliseconds);

	memcpy_s(buff, wcslen(bufftime) * sizeof(wchar_t), bufftime, wcslen(bufftime) * sizeof(wchar_t));
}

/******************************************************************************
*! @brief  : Write msg to file tracker.log
*! @return : void
*! @author : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/
void logger_save(const wchar_t* text)
{
	static std::wstring filepath = L"";
	if (filepath.empty())
	{
		wchar_t path[MAX_PATH];
		if (0 >= ::GetModuleFileName(NULL, path, sizeof(path)))
			return;

		std::wstring module_path = path;
		std::wstring file_name = L"HttpRequestLog\\";

		module_path = module_path.substr(0, module_path.rfind('\\') + 1);
		filepath = module_path + file_name;

		if (CreateDirectory(filepath.c_str(), NULL) || // create ok
			ERROR_ALREADY_EXISTS == GetLastError())
		{
			filepath.append(L"request.log");
		}
		else
		{
			filepath = L"";
			return;
		}
	}

	if (filepath.empty())
		return;

	// write tracker to file
	FILE* file = _wfsopen(filepath.c_str(), L"a+", SH_DENYNO);
	if (!file)
		return;

	std::string temp = logger_to_utf8(text, wcslen(text)*sizeof(wchar_t));
	fputs(temp.c_str(), file);
	fputs("\n", file);

	fclose(file);
}

/******************************************************************************
*! @brief  : Write msg to file logfile
*! @return : void
*! @author : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/
void logger_printf(int state, const char* filename, int linenum, BOOL savetime, const wchar_t* format, va_list args)
{
	wchar_t fmt[KY_HTTP_MAX_LENGTH_MSG_LOG];
	memset(fmt, 0, KY_HTTP_MAX_LENGTH_MSG_LOG);

	wchar_t fstate[50];
	memset(fstate, 0, 50);

	if (state == eLogger_Warning)
	{
		memcpy(fstate, L"[warn] ", sizeof(L"[warn] "));
	}
	else if (state == eLogger_Error)
	{
		memcpy(fstate, L"[error]", sizeof(L"[error]"));
	}
	else if (state == eLogger_Info)
	{
		memcpy(fstate, L"[info] ", sizeof(L"[info] "));
	}

	if (savetime)
	{
		const size_t ndatetime = 200;
		wchar_t datetime[ndatetime];  memset(datetime, 0, ndatetime);
		logger_get_datetime(datetime);

		if(wcscmp(L"", fstate) ==0)
			swprintf_s(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, L"%s %s", datetime, format);
		else 
			swprintf_s(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, L"%s %s %s", datetime, fstate, format);
	}
	else
	{
		if (wcscmp(L"", fstate) == 0)
			swprintf_s(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, L"%s", format);
		else
			swprintf_s(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, L"%s %s", fstate, format);
	}

	if (filename != NULL)
	{
		const size_t nmaxfilename = 200;

		wchar_t fname[nmaxfilename];
		memset(fname, 0, nmaxfilename);

		size_t outSize;
		mbstowcs_s(&outSize, &fname[0], nmaxfilename, filename, strlen(filename));

		swprintf_s(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, L"%s [%s %d]", fmt, fname, linenum);
		
	}

	wchar_t msg[KY_HTTP_MAX_LENGTH_MSG_LOG];
	memset(msg, 0, KY_HTTP_MAX_LENGTH_MSG_LOG);
	vswprintf_s(msg, KY_HTTP_MAX_LENGTH_MSG_LOG, fmt, args);

	logger_save(msg);
}

void logger_printf(int state, const char* filename, int linenum, BOOL savetime, const wchar_t* format, ...)
{
	va_list args;
	va_start(args, format);
	logger_printf(state, filename, linenum, savetime, format, args);
	va_end(args);
}

/******************************************************************************
*! @brief  : Write msg to file logfile - function
*! @return : void
*! @author : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/
void logger_printf_func(int begin, const char* filename, int linenum, BOOL savetime, const wchar_t* format, va_list args)
{
	wchar_t fmt[KY_HTTP_MAX_LENGTH_MSG_LOG];
	memset(fmt, 0, KY_HTTP_MAX_LENGTH_MSG_LOG);

	const size_t nmaxfilename = 200;
	wchar_t fname[nmaxfilename];
	memset(fname, 0, nmaxfilename);

	size_t outSize;
	mbstowcs_s(&outSize, &fname[0], nmaxfilename, filename, strlen(filename));

	if (savetime)
	{
		const size_t ndatetime = 200;
		wchar_t datetime[ndatetime];  memset(datetime, 0, ndatetime);
		logger_get_datetime(datetime);
		swprintf_s(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, L"%s", datetime);
	}

	if (begin)
		swprintf_s(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, L"%s [func-%s:%d][begin] %s", fmt, fname, linenum, format);
	else
		swprintf_s(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, L"%s [func-%s:%d][end] %s", fmt, fname, linenum, format);


	wchar_t msg[KY_HTTP_MAX_LENGTH_MSG_LOG];
	memset(msg, 0, KY_HTTP_MAX_LENGTH_MSG_LOG);
	vswprintf_s(msg, KY_HTTP_MAX_LENGTH_MSG_LOG, fmt, args);

	logger_save(msg);
}

void logger_printf_func(int begin, const char* filename, int linenum, BOOL savetime, const wchar_t* format, ...)
{
	va_list args;
	va_start(args, format);
	logger_printf_func(begin, filename, linenum, savetime, format, args);
	va_end(args);
}


#define KY_HTTP_TRACE(fmt,...)			logger_printf(eLogger_Trace, __FUNCTION__,__LINE__, TRUE, fmt, ##__VA_ARGS__)
#define KY_HTTP_LOG(fmt,...)			logger_printf(NULL			 , NULL, -1, TRUE, fmt,##__VA_ARGS__)
#define KY_HTTP_LOG_ERROR(fmt,...)		logger_printf(eLogger_Error  , NULL, -1, TRUE, fmt,##__VA_ARGS__)
#define KY_HTTP_LOG_WARNING(fmt,...)	logger_printf(eLogger_Warning, NULL, -1, TRUE, fmt,##__VA_ARGS__)
#define KY_HTTP_LOG_ASSERT(fmt,...)		logger_printf(eLogger_Assert , NULL, -1, TRUE, fmt,##__VA_ARGS__)
#define KY_HTTP_LOG_INFO(fmt,...)		logger_printf(eLogger_Info   , NULL, -1, TRUE, fmt,##__VA_ARGS__)

#define KY_HTTP_ENTRY(fmt, ...)			logger_printf_func(TRUE, __FUNCTION__,__LINE__, TRUE, fmt, ##__VA_ARGS__)
#define KY_HTTP_LEAVE(fmt, ...)			logger_printf_func(FALSE, __FUNCTION__,__LINE__, TRUE, fmt, ##__VA_ARGS__)