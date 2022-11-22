/*!**********************************************************************
* @copyright Copyright (C) 2022 thuong.nv -email: mark.ngo@kohyoung.com.\n
*            All rights reserved.
*************************************************************************
* @file     kyhttputil.h
* @date     Jul 11, 2022
* @brief    HTTP client libcurl implementation file.
*
** Including auxiliary functions
*************************************************************************/
#pragma once
#include <iostream>
#include <Windows.h>

#include "kyhttpdef.h"

__BEGIN_NAMESPACE__

/******************************************************************************
*! @brief  : Read bytes data file
*! @return : int : nsize / data : buff
*! @author : thuong.nv - [CreateDate] : 11/11/2022
*! @note   : delete data when finished using
******************************************************************************/
static int read_data_file(IN const wchar_t* path, OUT void** data)
{
    if (data) *data = NULL;
    int nbytes = 0;
    // open and read file share data
    FILE* file = _wfsopen(path, L"rb", _SH_DENYRD);
    if (!file) return nbytes;

    // read number of bytes file size 
    fseek(file, 0L, SEEK_END);
    nbytes = static_cast<int>(ftell(file));
    fseek(file, 0L, SEEK_SET);

    // read data form file to memory
    auto tbuff = new unsigned char[nbytes + 2];
    memset(tbuff, 0, (nbytes + 2));
    nbytes = (int)fread_s(tbuff, nbytes, sizeof(char), nbytes, file);
    fclose(file);

    // Read content bytes file + nbyte read
    if (data) *data = tbuff;
    else delete[] tbuff;
    return nbytes;
}

/******************************************************************************
*! @brief  : write data to file (create if not exist)
*! @return : int : nsize / data : buff
*! @author : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/
static int write_data_file(IN const wchar_t* path, IN const void* data, IN const int& nsize)
{
	FILE* file = _wfsopen(path, L"wb", SH_DENYNO);
	if (!file) return 0;

	if (nsize <= 0 || !data)
	{
		fwrite(data, sizeof(char), nsize, file);
	}
	fclose(file);
	return 1;
}

static int write_data_file_append(IN const wchar_t* path, IN const void* data, IN const int& nsize)
{
	FILE* file = _wfsopen(path, L"ab+", SH_DENYNO);
	if (!file) return 0;

	if (nsize <= 0 || !data)
	{
		fclose(file);
		return 1;
	}

	fwrite(data, sizeof(char), nsize, file);

	fclose(file);
	return 1;
}

/******************************************************************************
*! @brief  : convert number bytes to text (including unit of measure)
*! @return : wstring 
*! @author : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/
static std::string convert_bytes_to_text(IN double bytes)
{
	const int nsizebuff = 50;
	char buff[nsizebuff];
	memset(buff, 0, nsizebuff);

	if (bytes >= 1073741824.0)   // GB
	{
		snprintf(buff, nsizebuff, "%.2f GB", bytes / 1073741824.0);
	}
	else if (bytes >= 1048576.0) // MB
	{
		snprintf(buff, nsizebuff, "%.2f MB", bytes / 1048576);
	}
	else if (bytes >= 1024)   // KB
	{
		snprintf(buff, nsizebuff, "%.2f KB", bytes / 1024);
	}
	else if (bytes > 1)       // bytes
	{
		snprintf(buff, nsizebuff, "%.2f bytes", bytes);
	}
	else if (bytes == 1)
	{
		snprintf(buff, nsizebuff, "1 byte");
	}
	else
	{
		snprintf(buff, nsizebuff, "0 byte");
	}
	return buff;
}

/******************************************************************************
*! @brief  : Create folder follow path
*! @author : thuong.nv - [Date] : 03/10/2022
*! @return : true : exist / false: not exist
******************************************************************************/
static bool create_directory_recursive(IN const std::wstring& path)
{
	BOOL bret = ::CreateDirectory(path.c_str(), NULL);

	if (bret)  return true;
	else
	{
		DWORD dwErr = GetLastError();
		if (dwErr == ERROR_ALREADY_EXISTS)
			return true;
		if ((dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND))
		{
			std::wstring subpath = path.substr(0, path.find_last_of('\\'));

			if (create_directory_recursive(subpath))
			{
				return ::CreateDirectory(path.c_str(), NULL) ? true : false;
			}
		}
	}
	return false;
}

/******************************************************************************
*! @brief    : convert wide char to string
*! @parameter: nsize : number bytes
*! @return   : void
*! @author   : thuong.nv - [CreateDate] : 11/11/2022
******************************************************************************/
static std::string convert_wc_to_string(IN const wchar_t* wc, IN const int& nsize = -1)
{
	std::string utf8;
	int length = nsize;

	if (length < 0)   // -1 default
		length = wcslen(wc);

	utf8.resize(length + 1, 0);

	int nbytes = ::WideCharToMultiByte(CP_UTF8, 0, wc, length,
		(LPSTR)utf8.c_str(), (int)utf8.size(), NULL, NULL);
	utf8.resize(nbytes);
	return utf8;
}

/******************************************************************************
*! @brief  : convert multiple byte to wstring
*! @return : void
*! @author : thuong.nv - [CreateDate] : 12/11/2022
******************************************************************************/
static std::wstring convert_mb_to_wstring(IN const char* mb, IN const int& nsize)
{
	std::wstring utf16;
	utf16.resize(nsize + 4, 0);
	int nWide = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, mb,
		nsize, (LPWSTR)utf16.c_str(), (int)utf16.size());
	utf16.resize(nWide);
	return utf16;
}


__END___NAMESPACE__