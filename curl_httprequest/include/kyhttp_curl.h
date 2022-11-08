#pragma once

#include "kyhttp.h"
#include <curl/curl.h>

__BEGIN_NAMESPACE__


/*==================================================================================
* Class HttpMultipartInfo
* Base on curl_mime (curl)
===================================================================================*/
class HttpMultipartInfo : public HttpContentInfo
{
private:
	struct curl_mime* m_curl_mine;
private:
	void Free()
	{

	}

public:
	HttpMultipartInfo() : m_curl_mine(NULL)
	{

	}
protected:
	virtual HttpContentType GetType() const
	{
		return HttpContentType::MULTIPART;
	}

	virtual int Create(IN HttpClient* request = NULL)
	{
		CURL* curl = static_cast<CURL*>(request->base());
		if (!request || !curl)
			return -1;

		// clear old data
		this->Free();

		/* Create the form */
		m_curl_mine = curl_mime_init(curl);

		/* Fill in the file upload field */
		auto field = curl_mime_addpart(m_curl_mine);
		curl_mime_type(field, get_string_content_type(m_content_type).c_str());
		curl_mime_name(field, m_strname.c_str());
		curl_mime_filedata(field, m_strvalue.c_str());
		curl_mime_data(field, (const char*)m_data, m_nsize);
		return 1;
	}

	virtual void* GetData()
	{
		return m_curl_mine;
	}

public:
	virtual std::string	GetRawContent() = 0;
};



class CurlHttpRequest : public HttpRequest
{
public:

};



class CurlHttpClient : public HttpClient
{
public:
	virtual HTTPStatusCode Post(IN const RequestUri uri, IN HttpRequest* request)
	{

		return HTTPStatusCode::OK;
	}


	virtual HTTPStatusCode Get(IN const RequestUri uri, IN HttpRequest* request)
	{

		return HTTPStatusCode::OK;
	}
};


__END___NAMESPACE__