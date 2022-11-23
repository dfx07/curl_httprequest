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

#include "kyhttpdef.h"
#include "kyhttp_utils.h"


__BEGIN_NAMESPACE__

enum tracker_state
{
	eLogger_None	= 0x0000,
	eLogger_Error	= 0x0001,
	eLogger_Warning = 0x0002,
	eLogger_Assert  = 0x0004,
	eLogger_Info    = 0x0008,
	eLogger_Trace   = 0x0009,
};

#define KY_HTTP_MAX_LENGTH_MSG_LOG 2048

/******************************************************************************
*! @brief  : get date time now in system local
*! @return : void
*! @author : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/
static void logger_get_datetime(char* buff)
{
	SYSTEMTIME SystemTime;
	GetLocalTime(&SystemTime);

	const size_t buffsize = 200;
	char bufftime[buffsize];

	memset(bufftime, 0, buffsize);
	snprintf(bufftime, buffsize, "%04d-%02d-%02d %02d:%02d:%02d:%03d",
		SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
		SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond,
		SystemTime.wMilliseconds);

	memcpy_s(buff, buffsize, bufftime, buffsize);
}

/******************************************************************************
*! @brief  : save the text to file (log)
*! @return : void
*! @author : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/
static void logger_save(const char* text)
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

	//OutputDebugString(text);
	//OutputDebugString(L"\n");

	if (filepath.empty())
		return;

	// write tracker to file
	FILE* file = _wfsopen(filepath.c_str(), L"a+", SH_DENYNO);
	if (!file)
		return;

	std::string temp(text);
	fputs(temp.c_str(), file);
	fputs("\n", file);

	fclose(file);
}

/******************************************************************************
*! @brief  : Write msg to file logfile
*! @return : void
*! @author : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/

static void logger_printf(int state, const char* filename, int linenum, BOOL savetime, const char* format, va_list args)
{
	char fmt[KY_HTTP_MAX_LENGTH_MSG_LOG];
	memset(fmt, 0, KY_HTTP_MAX_LENGTH_MSG_LOG);

	char fstate[50];
	memset(fstate, 0, 50);

	if (state == eLogger_Warning)
	{
		memcpy(fstate, "[Warn] ", sizeof("[Warn] "));
	}
	else if (state == eLogger_Error)
	{
		memcpy(fstate, "[Error]", sizeof("[Error]"));
	}
	else if (state == eLogger_Info)
	{
		memcpy(fstate, "[Info] ", sizeof("[Info] "));
	}

	if (savetime)
	{
		const size_t ndatetime = 200;
		char datetime[ndatetime];  memset(datetime, 0, ndatetime);
		logger_get_datetime(datetime);

		if (strcmp("", fstate) == 0)
			snprintf(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, "<%s> %s", datetime, format);
		else
			snprintf(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, "<%s> %s%s", datetime, fstate, format);
	}
	else
	{
		if (strcmp("", fstate) == 0)
			snprintf(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, "%s", format);
		else
			snprintf(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, "%s %s", fstate, format);
	}

	if (filename != NULL)
	{
		const size_t nmaxfilename = 200;
		snprintf(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, "%s ( %s %d )", fmt, filename, linenum);
	}

	char msg[KY_HTTP_MAX_LENGTH_MSG_LOG];
	memset(msg, 0, KY_HTTP_MAX_LENGTH_MSG_LOG);
	vsnprintf_s(msg, KY_HTTP_MAX_LENGTH_MSG_LOG, fmt, args);

	logger_save(msg);
}

static void logger_printf(int state, const char* filename, int linenum, BOOL savetime, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	logger_printf(state, filename, linenum, savetime, format, args);
	va_end(args);
}

static void logger_printf(int state, const char* filename, int linenum, BOOL savetime, const wchar_t* format, ...)
{
	wchar_t msg[KY_HTTP_MAX_LENGTH_MSG_LOG];
	memset(msg, 0, KY_HTTP_MAX_LENGTH_MSG_LOG);

	va_list args; va_start(args, format);
	vswprintf_s(msg, KY_HTTP_MAX_LENGTH_MSG_LOG, format, args);
	va_end(args);

	std::string temp = kyhttp::convert_wc_to_string(msg);

	logger_printf(state, filename, linenum, savetime, "%s", temp.c_str());
}

/******************************************************************************
*! @brief  : Write msg to file logfile - function
*! @return : void
*! @author : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/
static void logger_printf_func(int begin, const char* filename, int linenum, BOOL savetime, const char* format, va_list args)
{
	char fmt[KY_HTTP_MAX_LENGTH_MSG_LOG];
	memset(fmt, 0, KY_HTTP_MAX_LENGTH_MSG_LOG);

	if (savetime)
	{
		const size_t ndatetime = 200;
		char datetime[ndatetime];  memset(datetime, 0, ndatetime);
		logger_get_datetime(datetime);
		snprintf(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, "%s", datetime);
	}

	if (begin)
		snprintf(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, "%s [func-%s:%d][begin] %s", fmt, filename, linenum, format);
	else
		snprintf(fmt, KY_HTTP_MAX_LENGTH_MSG_LOG, "%s [func-%s:%d][end] %s", fmt, filename, linenum, format);


	char msg[KY_HTTP_MAX_LENGTH_MSG_LOG];
	memset(msg, 0, KY_HTTP_MAX_LENGTH_MSG_LOG);
	vsnprintf_s(msg, KY_HTTP_MAX_LENGTH_MSG_LOG, fmt, args);

	logger_save(msg);
}

static void logger_printf_func(int begin, const char* filename, int linenum, BOOL savetime, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	logger_printf_func(begin, filename, linenum, savetime, format, args);
	va_end(args);
}

static void logger_printf_func(int begin, const char* filename, int linenum, BOOL savetime, const wchar_t* format, ...)
{
	wchar_t msg[KY_HTTP_MAX_LENGTH_MSG_LOG];
	memset(msg, 0, KY_HTTP_MAX_LENGTH_MSG_LOG);

	va_list args;
	va_start(args, format);
	vswprintf_s(msg, KY_HTTP_MAX_LENGTH_MSG_LOG, format, args);
	va_end(args);

	std::string temp = kyhttp::convert_wc_to_string(msg, KY_HTTP_MAX_LENGTH_MSG_LOG);
	logger_printf_func(begin, filename, linenum, savetime, "%s", temp.c_str());
}


#define KY_HTTP_TRACE(fmt,...)			logger_printf(eLogger_Trace, __FUNCTION__,__LINE__, TRUE, fmt, ##__VA_ARGS__)
#define KY_HTTP_LOG(fmt,...)			logger_printf(NULL			 , NULL, -1, TRUE , fmt,##__VA_ARGS__)
#define KY_HTTP_WRITE(fmt,...)			logger_printf(NULL			 , NULL, -1, FALSE, fmt,##__VA_ARGS__)
#define KY_HTTP_LOG_ERROR(fmt,...)		logger_printf(eLogger_Error  , NULL, -1, TRUE , fmt,##__VA_ARGS__)
#define KY_HTTP_LOG_WARN(fmt,...)		logger_printf(eLogger_Warning, NULL, -1, TRUE , fmt,##__VA_ARGS__)
#define KY_HTTP_LOG_ASSERT(fmt,...)		logger_printf(eLogger_Assert , NULL, -1, TRUE , fmt,##__VA_ARGS__)
#define KY_HTTP_LOG_INFO(fmt,...)		logger_printf(eLogger_Info   , NULL, -1, TRUE , fmt,##__VA_ARGS__)

#define KY_HTTP_LOGA(fmt,...)			logger_printf(NULL			 , NULL, -1, TRUE , fmt,##__VA_ARGS__)

#define KY_HTTP_ENTRY(fmt, ...)			logger_printf_func(TRUE, __FUNCTION__,__LINE__, TRUE, fmt, ##__VA_ARGS__)
#define KY_HTTP_LEAVE(fmt, ...)			logger_printf_func(FALSE, __FUNCTION__,__LINE__, TRUE, fmt, ##__VA_ARGS__)


__END___NAMESPACE__