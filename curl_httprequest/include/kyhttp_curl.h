#pragma once
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

#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "kyhttp.h"
#include "kyhttp_log.h"
#include "kyhttp_buffer.h"

__BEGIN_NAMESPACE__


#define CURL_STATICLIB
#pragma comment (lib, "libcurl_debug.lib")

#pragma comment (lib, "Normaliz.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "advapi32.lib")



#define SSL_CERTIFICATE_TYPE_PEM   "PEM"


/*==================================================================================
	HttpHeader:
		+ CurlHttpHeader
	HttpContent:
		+) Multipart
		+) 
===================================================================================*/

/*==================================================================================
* Class HttpMultipartInfo
* Base on curl_mime (curl)
===================================================================================*/


class HttpRawContent : public HttpContent
{

};


class HttpUrlEncodedContent : public HttpContent
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

	virtual int UpdateContent(IN void* base)
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

class HttpMultipartContent : public HttpContent
{
	struct HttpContentPart
	{
		std::string					m_name;
		std::string					m_value;
		std::string					m_filename;

		struct // data field
		{
			const void* m_data;
			size_t					m_size;
		};
	};

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
	HttpMultipartContent(): m_curl_mine(NULL)
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

	virtual int UpdateContent(IN void* base)
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

class HttpRequest
{
protected: // property header data
	HttpHeaderData		m_header_data;
	HttpContent*		m_content;
	HttpMethod			m_method;

private: // curl header data 
	struct curl_slist*  m_curl_slist;
	std::string		    m_buffer;

public:
	//void curl_header_append(const char* format, ...)
	//{
	//	const int nbufsize = 1024;
	//	char buf[nbufsize];
	//	memset(buf, 0, nbufsize);

	//	// list paramater write file follow format
	//	va_list args;
	//	va_start(args, format);
	//	vsnprintf(buf, nbufsize, format, args);
	//	va_end(args);

	//	// append buff to output
	//	m_curl_slist = curl_slist_append(m_curl_slist, buf);
	//}
	~HttpRequest()
	{
		this->curl_header_free();
	}

private:
	void curl_header_free()
	{
		curl_slist_free_all(m_curl_slist);
	}

	int update_curl_header(CURL* curl, const HttpHeaderData& header_data)
	{
		if (!curl) return 0;

		this->curl_header_free();

		auto append_curl_header = [](curl_slist** curl_slist, const char* format, ...)
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
			*curl_slist = curl_slist_append(*curl_slist, buf);
		};

		if (header_data.m_content_type != HttpDetailContentType::Auto)
			append_curl_header(&m_curl_slist, "Content-Type: %s", get_string_content_type(header_data.m_content_type).c_str());
		if (!header_data.m_host.empty())
			append_curl_header(&m_curl_slist, "Host: %s", header_data.m_host.c_str());
		if (!header_data.m_accept.empty())
			append_curl_header(&m_curl_slist, "Accept: %s", header_data.m_accept.c_str());
		if (!header_data.m_accept_encoding.empty())
			append_curl_header(&m_curl_slist, "Accept-encoding: %s", header_data.m_accept_encoding.c_str());

		append_curl_header(&m_curl_slist, "Connection: Keep-Alive");
		append_curl_header(&m_curl_slist, "User-Agent: Brinicle");
		append_curl_header(&m_curl_slist, "Pragma: no-cache");
		append_curl_header(&m_curl_slist, "Cache-Control: no-cache");

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, (curl_slist*)m_curl_slist);
	}

	int update_curl_content(CURL* curl, HttpContent* content)
	{
		if (!content) return 0;

		return content->UpdateContent(curl);
	}

	friend class HttpClient;

private:

	int update_curl_request(HttpMethod method, CURL* curl)
	{
		if (!update_curl_header(curl, m_header_data))
		{
			return 0;
		}

		if (HttpMethod::GET != method)
		{
			if (!update_curl_content(curl, m_content))
			{
				return 0;
			}
		}

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

	virtual void SetContent(IN HttpContent* content)
	{
		m_content = content;
	}
};


class HttpResponse
{
private:
	LONG			m_status;

	std::string		m_header;
	std::string		m_content;

protected:

public:
	HttpResponse() : m_status(HTTPCode::NODEFINE)
	{
		m_header.reserve(1000);
		m_content.reserve(1000);
	}

protected:

	virtual void Clear()
	{
		m_status = HTTPCode::NODEFINE;
		m_header.clear();
		m_content.clear();
	}

public:

	virtual HTTPCode GetStatusCode() const
	{
		return static_cast<HTTPCode>(m_status);
	}

	virtual std::string	   Header() const
	{
		return m_header;
	}

	virtual std::string	   Content() const
	{
		return m_content;
	}

	friend class HttpClient;
};


class HttpClient : public IHttpClient
{
private:
	CURL*				m_curl;
private:
	HttpRequestPtr		m_request;
	HttpResponsePtr		m_response;

	HttpClientOption	m_option;
	HttpCookie			m_cookie;
	SSLSetting			m_SSLSetting;
	WebProxy			m_proxy;

	HttpClientProgress	m_progress;

	float				m_time_send;
	float				m_speed_send;
	float				m_speed_recv;
public:
	HttpClient(): m_curl(nullptr),
		m_request(nullptr),
		m_response(nullptr)
	{

	}
	~HttpClient()
	{
		this->Destroy();
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
	bool Init()
	{
		if (m_curl) this->Destroy();

		m_curl	   = curl_easy_init();
		
		return true;
	}

	void Destroy()
	{
		curl_easy_cleanup(m_curl);
		m_curl	   = nullptr;
		m_response = nullptr;
	}

	static KYHttpErrorCode ConvertCURLCodeToHTTPCode(CURLcode curl_code)
	{
	    //DEBUG_ENTRY("CURLcode curl_code = <%d>", curl_code)
	
	    KYHttpErrorCode kyhttp_error_code;
	
	    switch (curl_code)
	    {
	        case CURLE_OK :
				kyhttp_error_code = KY_HTTP_OK;
	
	            break;
	        case CURLE_COULDNT_RESOLVE_PROXY :
				kyhttp_error_code = KY_HTTP_COULDNT_RESOLVE_PROXY;
	
	            break;
	        case CURLE_COULDNT_RESOLVE_HOST :
				kyhttp_error_code = KY_HTTP_COULDNT_RESOLVE_HOST;
	
	            break;
	        case CURLE_COULDNT_CONNECT :
				kyhttp_error_code = KY_HTTP_COULDNT_CONNECT;
	
	            break;
	        case CURLE_OUT_OF_MEMORY :
				kyhttp_error_code = KY_HTTP_OUT_OF_MEMORY;
	
	            break;
	        case CURLE_SSL_CONNECT_ERROR :
				kyhttp_error_code = KY_HTTP_SSL_HANDSHAKE_FAIL;
	
	            break;
	        case CURLE_PEER_FAILED_VERIFICATION :
				kyhttp_error_code = KY_HTTP_SERVER_FAILED_VERIFICATION;
	
	            break;
	        case CURLE_SEND_ERROR :
				kyhttp_error_code = KY_HTTP_SEND_ERROR;
	
	            break;
	        case CURLE_RECV_ERROR :
				kyhttp_error_code = KY_HTTP_RECV_ERROR;
	
	            break;
	        case CURLE_SSL_CERTPROBLEM :
				kyhttp_error_code = KY_HTTP_SSL_CERTPROBLEM;
	
	            break;
	        case CURLE_OPERATION_TIMEDOUT  :
				kyhttp_error_code = KY_HTTP_REQUEST_TIMEOUT;
	
	            break;
	        default :
				kyhttp_error_code = KY_HTTP_FAILED;
	
	            break;
	    }
	
	    //DEBUG_LEAVE("retVal = <%d>", KY_HTTP_code);
	    return kyhttp_error_code;
	}

	void CreateConfig(const HttpClientOption& option)
	{
		if (m_option.m_show_request)
		curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

		if (option.m_connect_timout > 0)
			curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT_MS, option.m_connect_timout);

		// Internal CURL progressmeter must be disabled if we provide our own callback
		curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, &m_progress);
		// Install the callback function
		curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, &HttpClient::HttpProgressFunc);

		if (option.m_max_upload_speed > 0)
		{
			// limit upload kb.s
			curl_off_t max_speed = 1000 * m_option.m_max_upload_speed; // 25kB/s
			curl_easy_setopt(m_curl, CURLOPT_MAX_SEND_SPEED_LARGE, max_speed);
		}
		if (option.m_max_download_speed > 0)
		{
			// limit download kb.s
			curl_off_t max_speed = 1000 * m_option.m_max_download_speed; // 25kB/s
			curl_easy_setopt(m_curl, CURLOPT_MAX_RECV_SPEED_LARGE, m_option.m_max_download_speed);
		}
	}

	void CreateResponse()
	{
		if (m_response)
		{
			m_response->Clear();

			curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, &HttpClient::HttpReceiveResponseFunc);
			curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &m_response->m_header);
			curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &HttpClient::HttpReceiveResponseFunc);
			curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_response->m_content);
		}
		else
		{
			m_response = std::make_shared<HttpResponse>();
		}
	}

	void CreateSSLOption(const SSLSetting* ssl_setting)
	{
		//// Set server certificate.
		//curl_easy_setopt(m_curl, CURLOPT_SSLCERTTYPE, SSL_CERTIFICATE_TYPE_PEM);
		//curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_DATA, configuration->mindsphere_certificate);
		//curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_FUNCTION, *_ssl_context_callback);
		//curl_easy_setopt(m_curl, CURLOPT_SSL_CIPHER_LIST, SUPPORTED_CIPHERS_LIST);
		//curl_easy_setopt(m_curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
		//
		// Verify the server's SSL certificate.
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 2);
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1);
	}

	void CreateProxyOption(const WebProxy* proxy)
	{
		// Set proxy options if proxy is used.
		if (!proxy->m_hostname.empty())
		{
		    curl_easy_setopt(m_curl, CURLOPT_PROXY, proxy->m_hostname);
		    curl_easy_setopt(m_curl, CURLOPT_PROXYPORT, proxy->m_port);
		    curl_easy_setopt(m_curl, CURLOPT_PROXYTYPE, proxy->m_proxy_type);
		
		    if (!proxy->m_username.empty())
		    {
	            curl_easy_setopt(m_curl, CURLOPT_PROXYUSERNAME, proxy->m_username.c_str());
		        curl_easy_setopt(m_curl, CURLOPT_PROXYPASSWORD, proxy->m_password.c_str());
		    }
		}
	}

	/******************************************************************************
	*! @brief  : setup body data package
	*! @author : thuong.nv - [Date] : 03/10/2022
	*! @parameter:	header : header info struct
	*! @return : HTTPRequestCode : OK , FAILED
	******************************************************************************/
	HTTPStatusCode CreateHttpRequest(IN HttpMethod method)
	{
		if (!Init())
		{
			std::cout << "[err] : init_curl failed !" << std::endl;
			return HTTPStatusCode::CREATE_FAILED;
		}

		if (method == HttpMethod::POST)
			curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
		else 
			curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1L); // Default get

		this->CreateConfig(m_option);
		this->CreateSSLOption(&m_SSLSetting);
		this->CreateProxyOption(&m_proxy);
		this->CreateResponse();

		return HTTPStatusCode::OK;
	}

	HTTPStatusCode UpdateRequest(IN HttpMethod method, IN HttpRequest* request)
	{
		if (!request)
			return HTTPStatusCode::FAILED;

		if (!request->update_curl_request(method, m_curl))
		{
			return HTTPStatusCode::FAILED;
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

		long http_code = 0;
		curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &http_code);
		m_response->m_status = http_code;

		return HTTPStatusCode::OK;
	}
public:
	virtual void SetConfigunation(IN HttpClientOption option)
	{
		m_option = option;
	}

	virtual void SettingProxy(IN WebProxy& proxy_info)
	{
		m_proxy = proxy_info;
	}

	virtual void SettingCookie(IN HttpCookie& cookie)
	{
		m_cookie = cookie;
	}

public:
	virtual HTTPStatusCode Request(IN HttpMethod method, IN const RequestUri uri, IN HttpRequest* request)
	{
		HTTPStatusCode request_code = HTTPStatusCode::FAILED;

		switch (method)
		{
		case kyhttp::POST:
			request_code = this->Post(uri, request);
			break;
		case kyhttp::GET:
			request_code = this->Get(uri, request);
			break;
		default:
			break;
		}

		return request_code;
	}

	virtual HTTPStatusCode Post(IN const RequestUri uri, IN HttpRequest* request)
	{
		if (nullptr == request)
		{
			std::cout << "[err-post] - Post request nullptr" << std::endl;
			return HTTPStatusCode::FAILED;
		}

		if (this->CreateHttpRequest(HttpMethod::POST) != HTTPStatusCode::OK)
		{
			std::cout << "[err] POST - Create engine failed !" << std::endl;
			return HTTPStatusCode::CREATE_FAILED;
		}

		if (this->UpdateRequest(HttpMethod::POST, request) != HTTPStatusCode::OK)
		{
			std::cout << "[err] POST - Create request failed !" << std::endl;
			return HTTPStatusCode::CREATE_FAILED;
		}

		return SendRequest(uri);
	}


	virtual HTTPStatusCode Get(IN const RequestUri uri, IN HttpRequest* request)
	{
		if (this->CreateHttpRequest(HttpMethod::GET) != HTTPStatusCode::OK)
		{
			return HTTPStatusCode::CREATE_FAILED;
		}

		if (request && this->UpdateRequest(HttpMethod::GET, request) != HTTPStatusCode::OK)
		{
			std::cout << "[err] GET - Create request failed !" << std::endl;
			return HTTPStatusCode::CREATE_FAILED;
		}

		return SendRequest(uri);
	}

	virtual HttpResponsePtr Response() const
	{
		return m_response;
	}


	friend class HttpReponse;
};


__END___NAMESPACE__