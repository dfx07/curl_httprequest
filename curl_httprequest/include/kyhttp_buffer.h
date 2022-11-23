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

#include <memory>

class HttpBuffer
{
	char*		 m_data;
	unsigned int m_size;
	unsigned int m_capacity;

public:
	HttpBuffer() : m_data(nullptr),
		m_size(0), m_capacity(0)
	{

	}

	~HttpBuffer()
	{
		this->_delete();
	}

	void*  buffer() const { return m_data; }
	size_t length() const { return m_size; }

private:
	/******************************************************************************
	*! @brief     : delete memory 
	*! @parameter : void
	*! @author    : thuong.nv - [Date] : 11/11/2022
	******************************************************************************/
	void _delete()
	{
		delete[] m_data;
		m_size = 0;
		m_capacity = 0;
	}

	void free()
	{
		if (m_data)
		{
			memset(m_data, 0, m_size);
		}
		m_size = 0;
	}

	/******************************************************************************
	*! @brief     : allocate memory
	*! @parameter : nsize : memory size
	*! @parameter : cp    : copy old memory
	*! @return : 1 : ok  / 0 : false
	*! @author    : thuong.nv - [Date] : 11/11/2022
	******************************************************************************/
	int alloc(int nsize, bool cp = false)
	{
		if (nsize <= m_capacity)
			return 1;

		const char* m_olddata = m_data;

		m_data = new char[nsize];
		memset(m_data, 0, nsize);
		m_capacity = nsize;
		delete[] m_olddata;

		if (cp && m_olddata &&  m_data != m_olddata)
		{
			memcpy(m_data, m_olddata, m_size);
		}
		else
		{
			memset(m_data, 0, m_size);
		}

		return 1;
	}

	/******************************************************************************
	*! @brief  : allocate memory - append to old memory
	*! @parameter : nsize : memory size append
	*! @return : 1 : ok  / 0 : false
	*! @author : thuong.nv - [Date] : 11/11/2022
	******************************************************************************/
	int alloc_append(int nsize)
	{
		int newsize = m_size + nsize;

		if (newsize <= m_capacity)
			return 1;

		char* m_olddata = m_data;
		m_capacity = newsize;

		m_data = new char[m_capacity];
		memset(m_data, 0, m_capacity);

#if defined(WIN32) || defined(WIN64)
		memcpy_s(m_data, m_capacity, m_olddata, m_size);
#else
		strncpy(destination, source, count);
#endif
		delete[] m_olddata;

		return 1;
	}

public:
	std::wstring to_wstring()
	{
		std::wstring utf16; utf16.resize(m_size, 0);
		int nWide = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, m_data, (int)m_size,
			(LPWSTR)utf16.c_str(), (int)utf16.size());
		utf16.resize(nWide);
		return utf16;
	}

	int save_file(IN const wchar_t* path, IN const void* data, IN const int& nsize)
	{
		FILE* file = _wfsopen(path, L"wb", SH_DENYNO);
		if (!file)
		{
			std::cout << "[err] - HttpBuffer save file failed !" << std::endl;
			return FALSE;
		}

		fwrite(data, sizeof(char), nsize, file);

		fclose(file);
		return TRUE;
	}

	void* operator[](const size_t& i) const
	{
		return &m_data[i];
	}

	int append(const void* data, unsigned int nsize)
	{
		if (nsize <= 0)
			return 1;

		int alloc_ok = alloc_append(nsize + 2);
		if (alloc_ok)
		{
			memcpy(m_data + m_size, data, nsize);
			m_size += nsize;
			return nsize;
		}
		return 0;
	}

	int set(const void* data, unsigned int nsize)
	{
		if (nsize <= 0)
			return 1;

		int alloc_ok = alloc(nsize + 2, false);
		if (alloc_ok)
		{
			memcpy(m_data, data, nsize);

			m_size = nsize;
			return nsize;
		}
		return 0;
	}

	void reserve(unsigned int nsize)
	{
		if (nsize <= 0)
			return ;

		alloc(nsize, true);
	}

	bool empty()
	{
		return m_size <= 0 ? true : false;
	}

	void clear()
	{
		this->free();
	}
};
