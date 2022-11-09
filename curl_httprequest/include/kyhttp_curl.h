#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "kyhttp.h"

__BEGIN_NAMESPACE__


#define CURL_STATICLIB
#pragma comment (lib, "libcurl_debug.lib")

#pragma comment (lib, "Normaliz.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "advapi32.lib")

/*==================================================================================
	HttpHeader:
		+ CurlHttpHeader
	HttpContent:
		+) Multipart
		+) 
===================================================================================*/
class CurlHttpHeader : public HttpHeader
{
private:
	HttpHeaderData	   m_header_data;
private:
	struct curl_slist* m_curl_slist;
	std::string		   m_buffer;
private:
	void header_format_replace(const char* format, ...)
	{
		const int nbufsize = 1024;
		char buf[nbufsize];
		memset(buf, 0, nbufsize);

		// list paramater write file follow format
		va_list args;
		va_start(args, format);
		vsnprintf(buf, nbufsize, format, args);
		va_end(args);

		// append buff to output
		m_curl_slist = curl_slist_append(m_curl_slist, buf);
	}


	void Free()
	{
		curl_slist_free_all(m_curl_slist);
	}

public:
	CurlHttpHeader() : m_curl_slist(nullptr)
	{

	}

	~CurlHttpHeader()
	{
		this->Free();
	}

protected:
	virtual int	Create(IN void* base) override
	{
		CURL* curl = static_cast<CURL*>(base);
		if (!curl) return 0;

		if (m_header_data.m_content_type != HttpDetailContentType::Auto)
			header_format_replace("Content-Type: %s", get_string_content_type(m_header_data.m_content_type).c_str());
		if (!m_header_data.m_host.empty())
			header_format_replace("Host: %s", m_header_data.m_host.c_str());
		if (!m_header_data.m_accept.empty())
			header_format_replace("Accept: %s", m_header_data.m_accept.c_str());
		if (!m_header_data.m_accept_encoding.empty())
			header_format_replace("Accept-encoding: %s", m_header_data.m_accept_encoding.c_str());

		header_format_replace("Connection: Keep-Alive");
		header_format_replace("User-Agent: Brinicle");
		header_format_replace("Pragma: no-cache");
		header_format_replace("Cache-Control: no-cache");

		if (!m_curl_slist)
			return 0;

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, (curl_slist*)m_curl_slist);

		return 1;
	}

public:

	virtual void SetRequestParam(IN const char* req_param)
	{
		m_header_data.m_request_param = req_param;
	}

	virtual void SetContentType(IN HttpDetailContentType content_type)
	{
		m_header_data.m_content_type = content_type;
	}

	virtual void SetAccept(IN const char* accept)
	{
		m_header_data.m_accept = accept;
	}

	virtual void SetAcceptEncoding(IN const char* accept_encoding)
	{
		m_header_data.m_accept_encoding = accept_encoding;
	}

	virtual void SetHost(IN const char* host)
	{
		m_header_data.m_host = host;
	}
};


/*==================================================================================
* Class HttpMultipartInfo
* Base on curl_mime (curl)
===================================================================================*/

struct HttpContentPart
{
	//HttpContentDispositionType	m_type;
	std::string					m_name;
	std::string					m_value;

	std::string					m_filename;

	// data field
	struct
	{
		const void*				m_data;
		size_t					m_size;
	};
};

class CurlHttpRawContent : public HttpContent
{

};


class CurlHttpUrlEncodedContent : public HttpContent
{
	struct KeyValueParam
	{
		std::string key;
		std::string value;
	};

private:
	// It is cache for later use -> please don't delete
	std::string		m_str_curl_content_cache;
	std::vector<KeyValueParam> m_keyvalue;

private:
	virtual HttpContentType GetType() const
	{
		return HttpContentType::application;
	}

	virtual int Create(IN void* base)
	{
		CURL* curl = static_cast<CURL*>(base);
		if (!curl) return 0;

		m_str_curl_content_cache.clear();

		if (!m_keyvalue.empty())
			m_str_curl_content_cache.append(m_keyvalue[0].key + '=' + m_keyvalue[0].value);

		for (int i = 1; i < m_keyvalue.size(); i++)
		{
			m_str_curl_content_cache.append('&' + m_keyvalue[0].key + '=' + m_keyvalue[0].value);
		}

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, m_str_curl_content_cache.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, m_str_curl_content_cache.length());

		return 1;
	}

public:
	void AddKeyValue(const char* key, const char* value)
	{
		m_keyvalue.push_back({ key, value });
	}

	void AddKeyValue(const char* key, const std::string& value)
	{
		m_keyvalue.push_back({ key, value });
	}

	void AddKeyValue(const char* key, const bool value)
	{
		std::string vl;
		vl = value ? "true" : "false";
		m_keyvalue.push_back({ key, vl});
	}

	template<typename T, typename std::enable_if<std::is_arithmetic<T>::value, T>::type* = nullptr>
	void AddKeyValue(const char* key, const T& value)
	{
		std::string t = std::to_string(value);
		m_keyvalue.push_back({ key, t });
	}
};

class CurlHttpMultipartContent : public HttpContent
{
private:
	struct curl_mime*			 m_curl_mine;
	std::vector<HttpContentPart> m_part_list;

private:
	void Free()
	{
		curl_mime_free(m_curl_mine);
		m_curl_mine = NULL;
	}

	bool AppendPart(const HttpContentPart& part)
	{
		if (!m_curl_mine) return false;

		/* Fill in the file upload field */
		auto field = curl_mime_addpart(m_curl_mine);

		// [TODO] custom disposition_type define auto curl
		//if (part.m_type != HttpContentDispositionType::_Auto)
		//{
		//	curl_mime_type(field, get_string_content_disposition_type(part.m_type).c_str());
		//}

		if (part.m_data)
		{
			curl_mime_name(field, part.m_name.c_str());
			curl_mime_filename(field, part.m_filename.c_str());
			curl_mime_data(field, (const char*)part.m_data, part.m_size);
		}
		else
		{
			curl_mime_name(field, part.m_name.c_str());
			curl_mime_data(field, part.m_value.c_str(), CURL_ZERO_TERMINATED);
		}

		return true;
	}

public:
	CurlHttpMultipartContent(): m_curl_mine(NULL)
	{

	}

	void AddPartKeyValue(const char* name, const char* value)
	{
		HttpContentPart part;

		//part.m_type  = HttpContentDispositionType::_Auto;
		part.m_name  = name;
		part.m_value = value;
		part.m_data  = NULL;
		part.m_size  = 0;

		m_part_list.push_back(part);
	}

	void AddPartFile(const char* name, const char* filename,const void* data = NULL, size_t data_size = 0)
	{
		HttpContentPart part;

		//part.m_type   = HttpContentDispositionType::_Auto;
		part.m_name     = name;
		part.m_filename = filename;
		part.m_data     = data;
		part.m_size     = data_size;

		m_part_list.push_back(part);
	}

protected:
	virtual HttpContentType GetType() const
	{
		return HttpContentType::multipart;
	}

	virtual int Create(IN void* base)
	{
		CURL* curl = static_cast<CURL*>(base);
		if (!curl) return 0;

		this->Free(); // clear old data

		m_curl_mine = curl_mime_init(curl);

		int i = 0;
		for (i = 0; i < m_part_list.size(); i++)
		{
			if (!this->AppendPart(m_part_list[i]))
				break;
		}

		// create part failed
		if (i < m_part_list.size())
		{
			this->Free();
			return 0;
		}

		curl_easy_setopt(curl, CURLOPT_MIMEPOST, m_curl_mine);

		return 1;
	}

	virtual void* GetData()
	{
		return m_curl_mine;
	}
};

class CurlHttpRequest : public HttpRequest
{
protected:
	HttpHeaderPtr		m_header;
	HttpContent*		m_content;

protected:
	RequestUri			m_uri;
	HttpMethod			m_method;

private:

	virtual int Create(void* base) override
	{
		CURL* curl = static_cast<CURL*>(base);

		if(!curl)
			return 0;

		m_header = std::make_shared<CurlHttpHeader>();

		if (m_header && !m_header->Create(curl))
			return 0;

		if (m_content && !m_content->Create(curl))
			return 0;

		return 1;
	}

public:

	virtual void SetContent(IN HttpContent* content)
	{
		m_content = content;
	}

	virtual HttpMethod GetMethod() const
	{
		return m_method;
	}

	virtual RequestUri GetUri() const
	{
		return m_uri;
	}
};


class CurlHttpReponse : public HttpResponse
{
private:
	HTTPStatusCode	m_status;
	std::string		m_header;
	std::string		m_content;

protected:
	void SetStatus(HTTPStatusCode& status)
	{
		m_status = status;
	}
public:
	CurlHttpReponse() : m_status(HTTPStatusCode::OK)
	{
		m_header.reserve(1000);
		m_content.reserve(1000);
	}

public:
	virtual void Clear() override
	{
		m_status = HTTPStatusCode::OK;
		m_header.clear();
		m_content.clear();
	}

	virtual HTTPStatusCode Status() const override
	{
		return m_status;
	}

	virtual std::string	   Header() const override
	{
		return m_header;
	}

	virtual std::string	   Content() const override
	{
		return m_content;
	}

	friend class CurlHttpClient;
};


class CurlHttpClient : public HttpClient
{
private:
	HttpRequestPtr		m_request;
	HttpResponsePtr		m_response;
	HttpClientOption	m_option;

	HttpClientProgress	m_progress;
	CURL*				m_curl;


	float				m_time_send;
	float				m_speed_send;
	float				m_speed_recv;
public:
	CurlHttpClient(): m_curl(nullptr),
		m_request(nullptr),
		m_response(nullptr)
	{

	}
	~CurlHttpClient()
	{
		this->destroy_curl();
	}

private:
	static int HttpProgressFunc(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
	{
		if (ptr == NULL)
			return 0;

		HttpClientProgress* progress = static_cast<HttpClientProgress*>(ptr);

		// stop download or upload data network
		if (progress && progress->m_force_stop)
		{
			return 1;
		}

		if (TotalToUpload > 0 && progress)
		{
			progress->m_action = 1;
			progress->m_cur_upload = NowUploaded;
			progress->m_total_upload = TotalToUpload;
		}
		else if (TotalToDownload > 0 && progress)
		{
			progress->m_action = 2;
			progress->m_cur_download = NowDownloaded;
			progress->m_total_download = TotalToDownload;
		}
		return 0;
	}
	static int HttpReceiveResponseFunc(void* contents, size_t size, size_t nmemb, void* user_data)
	{
		if (user_data)
		{
			((std::string*)user_data)->append((char*)contents, size * nmemb);
		}
		return size * nmemb;
	}

private:
	/******************************************************************************
	*! @brief  : setup body data package
	*! @author : thuong.nv - [Date] : 03/10/2022
	*! @parameter:	header : header info struct
	*! @return : HTTPRequestCode : OK , FAILED
	******************************************************************************/
	bool init_curl()
	{
		if (m_curl)
			this->destroy_curl();

		m_curl	   = curl_easy_init();
		m_response = std::make_shared<CurlHttpReponse>();
		return true;
	}

	void destroy_curl()
	{
		curl_easy_cleanup(m_curl);
		m_curl = nullptr;
	}

	/******************************************************************************
	*! @brief  : setup body data package
	*! @author : thuong.nv - [Date] : 03/10/2022
	*! @parameter:	header : header info struct
	*! @return : HTTPRequestCode : OK , FAILED
	******************************************************************************/
	HTTPStatusCode Create(IN HttpMethod method)
	{
		if (!init_curl())
		{
			std::cout << "[err] : init_curl failed !" << std::endl;
			return HTTPStatusCode::CREATE_FAILED;
		}

		// Default get
		if (method == HttpMethod::POST)
		{
			curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
		}

		//if (m_option.m_show_request)
			curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

		if (m_option.m_connect_timout > 0)
			curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT_MS, m_option.m_connect_timout);

		// Internal CURL progressmeter must be disabled if we provide our own callback
		curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, &m_progress);
		// Install the callback function
		curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, &CurlHttpClient::HttpProgressFunc);


		if (m_option.m_max_upload_speed > 0)
		{
			// limit upload kb.s
			curl_off_t max_speed = 1000 * m_option.m_max_upload_speed; // 25kB/s
			curl_easy_setopt(m_curl, CURLOPT_MAX_SEND_SPEED_LARGE, max_speed);
		}
		if (m_option.m_max_download_speed > 0)
		{
			// limit download kb.s
			curl_off_t max_speed = 1000 * m_option.m_max_download_speed; // 25kB/s
			curl_easy_setopt(m_curl, CURLOPT_MAX_RECV_SPEED_LARGE, m_option.m_max_download_speed);
		}

		m_response->Clear();

		auto pCurlResponse = std::dynamic_pointer_cast<CurlHttpReponse>(m_response);
		if (m_response)
		{
			curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, &CurlHttpClient::HttpReceiveResponseFunc);
			curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &pCurlResponse->m_header);
			curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &CurlHttpClient::HttpReceiveResponseFunc);
			curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &pCurlResponse->m_content);
		}
		return HTTPStatusCode::OK;
	}

	HTTPStatusCode CreateRequest(IN HttpRequest* request)
	{
		if (!request)
			return HTTPStatusCode::FAILED;

		if (!request->Create(m_curl))
		{
			return HTTPStatusCode::FAILED;
		}

		return HTTPStatusCode::OK;
	}

	HTTPStatusCode CreateRequest(IN HttpMethod method, IN HttpHeader* header = nullptr, IN HttpContent* content = nullptr)
	{
		
		if (header && !header->Create(m_curl))
		{
			return HTTPStatusCode::FAILED;
		}

		if (method == HttpMethod::POST)
		{
			if (content && !content->Create(m_curl))
			{
				return HTTPStatusCode::FAILED;
			}
		}

		return HTTPStatusCode::OK;
	}

private:
	HTTPStatusCode SendRequest(IN const RequestUri& uri)
	{
		if (!m_curl)
			return HTTPStatusCode::FAILED;

		curl_easy_setopt(m_curl, CURLOPT_URL, uri.uri.c_str());

		CURLcode res = curl_easy_perform(m_curl);

		curl_easy_getinfo(m_curl, CURLINFO_SPEED_UPLOAD, &m_speed_send);
		curl_easy_getinfo(m_curl, CURLINFO_TOTAL_TIME, &m_time_send);
		curl_easy_getinfo(m_curl, CURLINFO_SPEED_DOWNLOAD, &m_speed_recv);

		return HTTPStatusCode::OK;
	}
public:
	virtual HTTPStatusCode Send(IN HttpRequest* request)
	{
		auto method = request->GetMethod();

		if (this->Create(method) != HTTPStatusCode::OK)
		{
			std::cout << "[err] POST - Create engine failed !" << std::endl;
			return HTTPStatusCode::CREATE_FAILED;
		}

		if (this->CreateRequest(request) != HTTPStatusCode::OK)
		{
			std::cout << "[err] POST - Create request failed !" << std::endl;
			return HTTPStatusCode::CREATE_FAILED;
		}

		return SendRequest(request->GetUri());
	}

	virtual HTTPStatusCode Post(IN const RequestUri uri, IN HttpHeader* header, IN HttpContent* content)
	{
		if (this->Create(HttpMethod::POST) != HTTPStatusCode::OK)
		{
			std::cout << "[err] POST - Create engine failed !" << std::endl;
			return HTTPStatusCode::CREATE_FAILED;
		}

		if (this->CreateRequest(HttpMethod::POST, header, content) != HTTPStatusCode::OK)
		{
			std::cout << "[err] POST - Create request failed !" << std::endl;
			return HTTPStatusCode::CREATE_FAILED;
		}

		return SendRequest(uri);
	}


	virtual HTTPStatusCode Get(IN const RequestUri uri, IN HttpHeader* header)
	{
		if (this->Create(HttpMethod::GET) != HTTPStatusCode::OK)
		{
			return HTTPStatusCode::CREATE_FAILED;
		}

		if (this->CreateRequest(HttpMethod::GET, header) != HTTPStatusCode::OK)
		{
			std::cout << "[err] GET - Create request failed !" << std::endl;
			return HTTPStatusCode::CREATE_FAILED;
		}

		return SendRequest(uri);
	}
};


__END___NAMESPACE__