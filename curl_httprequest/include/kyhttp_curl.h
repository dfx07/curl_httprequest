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
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>

#include "kyhttp_types.h"
#include "kyhttp_log.h"
#include "kyhttp_buffer.h"

__BEGIN_NAMESPACE__


#define CURL_STATICLIB

#ifdef _DEBUG
#pragma comment (lib, "libcurl_debug.lib")
#else
#pragma comment (lib, "libcurl.lib")
#endif // _DEBUG

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
class IKeyValue
{
	struct KeyValueParam
	{
		std::string key;
		std::string value;
	};

protected:
	std::vector<KeyValueParam> m_keyvalue;

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
		m_keyvalue.push_back({ key, vl });
	}

	template<typename T, typename std::enable_if<std::is_arithmetic<T>::value, T>::type* = nullptr>
	void AddKeyValue(const char* key, const T& value, const int& precision = 3)
	{
		std::string t = "";
		if (std::is_floating_point<T>::value)
		{
			std::stringstream stream;
			stream << std::fixed << std::setprecision(precision) << value;
			t = stream.str();
		}
		else
		{
			t = std::to_string(value);
		}

		// remove trailing zero
		while (!t.empty() && t.back() == '0')
		{
			t.pop_back();
		}
		m_keyvalue.push_back({ key, t });
	}
};

class Uri : private IKeyValue
{
private:
	std::string location;

public:
	template<typename T>
	void add_query_param(const char* key, T value)
	{
		this->AddKeyValue(key, value);
	}

	void set_location(IN const char* loc)
	{
		location = loc;
	}

	std::string get_query_param() const
	{
		std::string query_param;

		if (!m_keyvalue.empty())
		{
			query_param.append(m_keyvalue[0].key + "=" + m_keyvalue[0].value);
		}
		for (int i = 1; i < m_keyvalue.size(); i++)
		{
			const auto& param = m_keyvalue[i];
			query_param.append("&" + param.key + "=" + param.value);
		}
		return query_param;
	}

	friend class HttpClient;
};

class HttpRawContent : public HttpContent
{

};

class HttpUrlEncodedContent : public HttpContent, public IKeyValue
{
private:
	// It is cache for later use -> please don't delete
	std::string		m_str_curl_content_cache;

private:
	virtual HttpContentType GetType() const
	{
		return HttpContentType::urlencoded;
	}

	virtual void* InitContent(IN void* base)
	{
		CURL* curl = static_cast<CURL*>(base);
		if (!curl) return NULL;

		m_str_curl_content_cache.clear();

		if (!m_keyvalue.empty())
			m_str_curl_content_cache.append(m_keyvalue[0].key + '=' + m_keyvalue[0].value);

		for (int i = 1; i < m_keyvalue.size(); i++)
		{
			m_str_curl_content_cache.append('&' + m_keyvalue[0].key + '=' + m_keyvalue[0].value);
		}

		return &m_str_curl_content_cache;
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
			size_t		m_size;
		};
	};

private:
	struct curl_mime*			 m_curl_mine;
	std::vector<HttpContentPart> m_part_list;

private:
	void FreeCurlMine()
	{
		curl_mime_free(m_curl_mine);
		m_curl_mine = NULL;
	}

	void InitCurlMine(IN CURL* curl)
	{
		if (m_curl_mine)
			this->FreeCurlMine();
		m_curl_mine = curl_mime_init(curl);
	}

	INT AddPart(const HttpContentPart& part)
	{
		if (!m_curl_mine) return FALSE;

		/* Fill in the file upload field */
		auto field = curl_mime_addpart(m_curl_mine);

		if (!field) return FALSE;

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

		return TRUE;
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

	virtual void* InitContent(IN void* base)
	{
		CURL* curl = static_cast<CURL*>(base);
		if (!curl) return NULL;

		this->InitCurlMine(curl);

		for (int i = 0; i < m_part_list.size(); i++)
		{
			if (FALSE == this->AddPart(m_part_list[i]))
			{
				std::cout << "[erro] : add part failed" << std::endl;
			}
		}

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
	~HttpRequest()
	{
		this->HeaderFree();
	}

private:
	void HeaderFree()
	{
		curl_slist_free_all(m_curl_slist);
	}

private:
	void* CreateHeaderData()
	{
		this->HeaderFree();

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

		if (m_header_data.m_content_type != HttpDetailContentType::Auto)
			append_curl_header(&m_curl_slist, "Content-Type: %s", get_string_content_type(m_header_data.m_content_type).c_str());
		if (!m_header_data.m_host.empty())
			append_curl_header(&m_curl_slist, "Host: %s", m_header_data.m_host.c_str());
		if (!m_header_data.m_accept.empty())
			append_curl_header(&m_curl_slist, "Accept: %s", m_header_data.m_accept.c_str());
		if (!m_header_data.m_accept_encoding.empty())
			append_curl_header(&m_curl_slist, "Accept-encoding: %s", m_header_data.m_accept_encoding.c_str());

		for (int i = 0; i < m_header_data.m_extension.size(); i++)
		{
			append_curl_header(&m_curl_slist, m_header_data.m_extension[i].c_str());
		}

		append_curl_header(&m_curl_slist, "Connection: Keep-Alive");
		append_curl_header(&m_curl_slist, "User-Agent: Brinicle");
		append_curl_header(&m_curl_slist, "Pragma: no-cache");
		append_curl_header(&m_curl_slist, "Cache-Control: no-cache");

		return m_curl_slist;
	}

	void* CreateContentData(IN void* base, IN HttpContentType& type)
	{
		CURL* curl = static_cast<CURL*>(base);
		if (!m_content || !curl)
			return NULL;

		void* content = m_content->InitContent(curl);

		type = m_content->GetType();
	}

	friend class HttpClient;

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

	HttpBuffer		m_header;
	HttpBuffer		m_content;

	std::string		m_redirect_url;
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
		m_redirect_url.clear();
	}

public:

	virtual HTTPCode GetStatusCode() const
	{
		return static_cast<HTTPCode>(m_status);
	}

	virtual const HttpBuffer* Header() const
	{
		return &m_header;
	}

	virtual const HttpBuffer* Content() const
	{
		return &m_content;
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

	double				m_request_time;
	float				m_sent_size;

	int					m_use_openssl = false; // curl build = schannel = false | openssl = true
	int					m_use_custom_ssl = false;

public:
	HttpClient(): m_curl(nullptr),
		m_request(nullptr), m_response(nullptr),
		m_use_openssl(false),
		m_use_custom_ssl(false)
	{

	}
	~HttpClient()
	{
		this->DestroyCurl();
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
			((HttpBuffer*)user_data)->append((char*)contents, size * nmemb);
		}
		return size * nmemb;
	}

private:
	/******************************************************************************
	*! @brief  : initialize , destroy curl
	*! @author : thuong.nv - [Date] : 03/10/2022
	*! @parameter:	header : header info struct
	*! @return : bool : TRUE / FALSE
	******************************************************************************/
	bool InitCurl()
	{
		if (m_curl) this->DestroyCurl();

		m_curl = curl_easy_init();
		return true;
	}

	void DestroyCurl()
	{
		curl_easy_cleanup(m_curl);
		m_curl = nullptr;
	}

	static HttpErrorCode ConvertCURLCodeToHTTPCode(CURLcode curl_code)
	{
	    //DEBUG_ENTRY("CURLcode curl_code = <%d>", curl_code)
	
		HttpErrorCode kyhttp_error_code = KY_HTTP_FAILED;
	
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

	HttpErrorCode CreateConfig(const HttpClientOption& option)
	{
		CURLcode curlcode = CURLcode::CURLE_OK;

		if (m_option.m_show_request)
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L));

		if (option.m_connect_timout > 0)
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT_MS, option.m_connect_timout));

		// Internal CURL progressmeter must be disabled if we provide our own callback
		PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, FALSE));
		PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, &m_progress));

		// Install the callback function
		PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, &HttpClient::HttpProgressFunc));

		if (option.m_max_upload_speed > 0)
		{
			// limit upload kb.s
			curl_off_t max_speed = 1000 * m_option.m_max_upload_speed; // 25kB/s
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_MAX_SEND_SPEED_LARGE, max_speed));
		}
		if (option.m_max_download_speed > 0)
		{
			// limit download kb.s
			curl_off_t max_speed = 1000 * m_option.m_max_download_speed; // 25kB/s
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_MAX_RECV_SPEED_LARGE, m_option.m_max_download_speed));
		}

		// auto redirected
		if (TRUE == option.m_auto_redirect)
		{
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 0L));
		}

		HttpErrorCode retcode = ConvertCURLCodeToHTTPCode(curlcode);
		return retcode;
	}

	HttpErrorCode InitClearResponse(int bForceCreate = false)
	{
		if (bForceCreate || !m_response)
		{
			m_response = std::make_shared<HttpResponse>();
		}
		else // clear old data
		{
			m_response->Clear();
		}

		curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, &HttpClient::HttpReceiveResponseFunc);
		curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &m_response->m_header);
		curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &HttpClient::HttpReceiveResponseFunc);
		curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_response->m_content);

		return HttpErrorCode::KY_HTTP_OK;
	}

	HttpErrorCode CreateSSLOption(const SSLSetting* ssl_setting)
	{
		// Verify manual confirmation using file *.pem
		if (m_use_openssl)
		{
			curl_easy_setopt(m_curl, CURLOPT_PROXY_SSLCERTTYPE, SSL_CERTIFICATE_TYPE_PEM);
			curl_easy_setopt(m_curl, CURLOPT_PROXY_SSL_VERIFYPEER, 1);
			curl_easy_setopt(m_curl, CURLOPT_PROXY_SSL_VERIFYHOST, 1);
			curl_easy_setopt(m_curl, CURLOPT_PROXY_CAINFO, "./cacert.pem");
		}
		// libcurl schanel build : Verify window certificate
		else
		{
			// Verify the server's SSL certificate.
			curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 1);
			curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1);
		}

		return HttpErrorCode::KY_HTTP_OK;
	}

	HttpErrorCode CreateProxyOption(const WebProxy* proxy)
	{
		// Set proxy options if proxy is used.
		if (!m_proxy.m_hostname.empty())
		{
			curl_easy_setopt(m_curl, CURLOPT_PROXY, m_proxy.m_hostname.c_str());
			curl_easy_setopt(m_curl, CURLOPT_PROXYPORT, m_proxy.m_port);
			
			// use libcurl build openssl
			if (m_use_openssl)
			{
				curl_easy_setopt(m_curl, CURLOPT_PROXYTYPE, m_proxy.m_proxy_type);
				curl_easy_setopt(m_curl, CURLOPT_PROXY_SSLCERTTYPE, SSL_CERTIFICATE_TYPE_PEM);
				curl_easy_setopt(m_curl, CURLOPT_PROXY_CAINFO, "./cacert.pem");
			}
			// use libcurl build schannel
			else
			{
				// Schannel not support HTTPs-proxy
				auto proxytype = m_proxy.m_proxy_type == kyhttp::KY_PROXY_HTTPS ?
					kyhttp::KY_PROXY_HTTP : m_proxy.m_proxy_type;
				curl_easy_setopt(m_curl, CURLOPT_PROXYTYPE, proxytype);
			}

		    if (!m_proxy.m_username.empty())
		    {
	            curl_easy_setopt(m_curl, CURLOPT_PROXYUSERNAME, m_proxy.m_username.c_str());
		        curl_easy_setopt(m_curl, CURLOPT_PROXYPASSWORD, m_proxy.m_password.c_str());
		    }
		}
		return HttpErrorCode::KY_HTTP_OK;
	}

	/******************************************************************************
	*! @brief  : curl initialization and related settings
	*! @author : thuong.nv - [Date] : 11/11/2022
	*! @parameter:	method : GET / POST
	*! @return : HttpErrorCode 
	******************************************************************************/
	HttpErrorCode InitHttpRequest(IN HttpMethod method)
	{
		HttpErrorCode err_code = HttpErrorCode::KY_HTTP_OK;

		if (!InitCurl())
		{
			KY_HTTP_LOG_ERROR(L"[err] : init_curl failed !");
			return HttpErrorCode::KY_HTTP_FAILED;
		}

		if (method == HttpMethod::POST)
			curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
		else 
			curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1L); // Default get

		PASS_ERROR_CODE(err_code, this->CreateConfig(m_option));
		PASS_ERROR_CODE(err_code, this->CreateSSLOption(&m_SSLSetting));
		PASS_ERROR_CODE(err_code, this->CreateProxyOption(&m_proxy));
		PASS_ERROR_CODE(err_code, this->InitClearResponse());

		KY_HTTP_LOG_INFO(L"Init Http request. Result code : <%u>", err_code);
		return err_code;
	}

	/******************************************************************************
	*! @brief  : setup header and content data for httprequest
	*! @author : thuong.nv - [Date] : 11/11/2022
	*! @parameter:	method : GET / POST
	*! @parameter:	HttpRequest : Contains header and content information
	*! @return : HttpErrorCode
	******************************************************************************/
	HttpErrorCode CreateRequestData(IN HttpMethod method, IN HttpRequest* request)
	{
		if (!request)
			return HttpErrorCode::KY_HTTP_FAILED;

		CURLcode curlcode = CURLE_OK;

		if (HttpMethod::GET == method && NULL == request)
		{
			return HttpErrorCode::KY_HTTP_OK;
		}

		// create header request data
		void* header_request = request->CreateHeaderData();
		curl_slist* header = static_cast<curl_slist*>(header_request);

		PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, header));

		// create content request data
		HttpContentType type = HttpContentType::none;
		void* content_request = request->CreateContentData(m_curl, type);

		if (HttpContentType::multipart == type)
		{
			curl_mime* data_post = static_cast<curl_mime*>(content_request);
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_MIMEPOST, data_post));
		}
		else if (HttpContentType::urlencoded == type)
		{
			std::string* data = static_cast<std::string*>(content_request);
			if (data)
			{
				PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, data->c_str()));
				PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, data->length()) );
			}
		}

		HttpErrorCode retcode = ConvertCURLCodeToHTTPCode(curlcode);
		KY_HTTP_LOG_INFO(L"Create request data done. Result code = <%u>", retcode);

		return retcode;
	}

private:
	HttpErrorCode SendRequest(IN const Uri& uri, BOOL redict = FALSE)
	{
		if (!m_curl)
			return HttpErrorCode::KY_HTTP_FAILED;
		std::string link = uri.location;

		std::string query_param = uri.get_query_param();
		if (!query_param.empty())
			link.append("?" + query_param);

		curl_easy_setopt(m_curl, CURLOPT_URL, link.c_str());

		CURLcode res = curl_easy_perform(m_curl);
		HttpErrorCode retcode = ConvertCURLCodeToHTTPCode(res);

		if (retcode != HttpErrorCode::KY_HTTP_OK)
		{
			KY_HTTP_LOG_INFO(L"Send request failed. Result code = <%u>, Curl code = <%u> , Redirect = <%s>",
				retcode, res, redict ? L"TRUE" : L"FALSE");
			return retcode;
		}

		curl_off_t lrequest_size = 0; curl_off_t lrequest_time = 0;
		double upload_size =0.;

		// get send request infomation
		if (curl_easy_getinfo(m_curl, CURLINFO_REQUEST_SIZE, &lrequest_size) == CURLE_OK)
		{
			m_sent_size += lrequest_size;
		}
		if (curl_easy_getinfo(m_curl, CURLINFO_SIZE_UPLOAD, &upload_size) == CURLE_OK)
		{
			m_sent_size += upload_size;
		}

		if (curl_easy_getinfo(m_curl, CURLINFO_TOTAL_TIME_T, &lrequest_time) == CURLE_OK)
		{
			m_request_time += double(lrequest_time)/ 1000000;
		}

		long http_code = 0;
		curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &http_code);

		m_response->m_status = http_code;

		if (http_code == HTTPCode::MOVED_PERMANENTLY)
		{
			this->InitClearResponse();
			
			char* url = NULL;
			curl_easy_getinfo(m_curl, CURLINFO_REDIRECT_URL, &url);
			if (url) m_response->m_redirect_url = url;

			if (m_option.m_auto_redirect)
			{
				Uri redirect_uri = uri;
				redirect_uri.location = m_response->m_redirect_url;
				retcode = SendRequest(redirect_uri, TRUE);
			}
		}

		if (!redict)
		{
			auto str_size = bytes_to_text(m_sent_size);
			KY_HTTP_LOG_INFO(L"HTTP request sent. Result code = <%u> [duration = <%.2f second>, size = <%s>] ",
				retcode, m_request_time, str_size.c_str());
		}
		return retcode;
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
	virtual HttpErrorCode Request(IN HttpMethod method, IN const Uri uri, IN HttpRequest* request)
	{
		HttpErrorCode request_code = HttpErrorCode::KY_HTTP_FAILED;

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

	virtual HttpErrorCode Post(IN const Uri uri, IN HttpRequest* request)
	{
		if (nullptr == request)
		{
			std::cout << "[err-post] - Post request nullptr" << std::endl;
			return HttpErrorCode::KY_HTTP_FAILED;
		}

		if (HttpErrorCode::KY_HTTP_OK != this->InitHttpRequest(HttpMethod::POST))
		{
			KY_HTTP_LOG_ERROR(L" ->[Failed] init http request failed !");
			return HttpErrorCode::KY_HTTP_CREATE_REQUEST_FAIL;
		}

		if (this->CreateRequestData(HttpMethod::POST, request) != HTTPStatusCode::OK)
		{
			KY_HTTP_LOG_ERROR(L" ->[Failed] set request parameter failed !");
			return HttpErrorCode::KY_HTTP_CREATE_REQUEST_FAIL;
		}

		return SendRequest(uri);
	}


	virtual HttpErrorCode Get(IN const Uri uri, IN HttpRequest* request)
	{
		if (HttpErrorCode::KY_HTTP_OK != this->InitHttpRequest(HttpMethod::GET))
		{
			KY_HTTP_LOG_ERROR(L" ->[Failed] init http request failed !");
			return HttpErrorCode::KY_HTTP_CREATE_REQUEST_FAIL;
		}

		if (request && this->CreateRequestData(HttpMethod::GET, request) != HttpErrorCode::KY_HTTP_OK)
		{
			KY_HTTP_LOG_ERROR(L" ->[Failed] set request parameter failed !");
			return HttpErrorCode::KY_HTTP_CREATE_REQUEST_FAIL;
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