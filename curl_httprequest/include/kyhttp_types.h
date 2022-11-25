/*!**********************************************************************
* @copyright Copyright (C) 2022 thuong.nv -( mark.ngo@kohyoung.com.)\n
*            All rights reserved.
*************************************************************************
* @file     kyhttp.h
* @date     Jul 11, 2022
* @brief    HTTP client libcurl implementation file.
*
** HTTP client implementation file using LibCurl.
*************************************************************************/
#pragma once

#include <Windows.h>
#include <string>
#include <memory>

#include "kyhttpdef.h"


__BEGIN_NAMESPACE__

class HttpClient;
typedef std::shared_ptr<HttpClient> HttpClientPtr;

interface HttpContent;
typedef std::shared_ptr<HttpContent> HttpContentPtr;

class HttpRequest;
typedef std::shared_ptr<HttpRequest> HttpRequestPtr;

class HttpResponse;
typedef std::shared_ptr<HttpResponse> HttpResponsePtr;

class HttpMultipartContent;
typedef std::shared_ptr<HttpMultipartContent> HttpMultipartContentPtr;

class HttpRawContent;
typedef std::shared_ptr<HttpRawContent> HttpRawContentPtr;

class HttpUrlEncodedContent;
typedef std::shared_ptr<HttpUrlEncodedContent> HttpUrlEncodedContentPtr;

class Uri;


enum HttpStatusCode
{
	NODEFINE							= -1 , // No support
	SUCCESS								= 200, // The request succeeded
	ACCEPTED							= 202, // The request has been received but not yet acted upon
	NO_CONTENT							= 204, // There is no content to send for this request, but the headers may be useful
	MOVED_PERMANENTLY					= 301, // The URL of the requested resource has been changed permanently. The new URL is given in the response.
	SEE_OTHER							= 303, // This response code means that the URI of requested resource has been changed temporarily
	BAD_REQUEST							= 400, // The server cannot or will not process the request due to something that is perceived to be a client error 
	UNAUTHORIZED						= 401, // Although the HTTP standard specifies "unauthorized"
	FORBIDDEN							= 403, // The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource
	NOT_FOUND							= 404, // The server cannot find the requested resource
	METHOD_NOT_ALLOWED					= 405, // The request method is known by the server but is not supported by the target resource
	INTERNAL_SERVER_ERROR				= 500, // The server has encountered a situation it does not know how to handle
	NOT_IMPLEMENTED						= 501, // The request method is not supported by the server and cannot be handled
	SERVICE_UNAVAILABLE					= 503, // The server is not ready to handle the request
};

enum HttpErrorCode
{
	KY_HTTP_FAILED						= -1,							  // Error not define
	KY_HTTP_OK							=  0,
	KY_HTTP_COULDNT_RESOLVE_PROXY		= KY_HTTP_ERR_BEGIN + 0x00000001, // Couldn't resolve proxy name
	KY_HTTP_COULDNT_RESOLVE_HOST		= KY_HTTP_ERR_BEGIN + 0x00000002, // Coundn't resolve host name ->IP
	KY_HTTP_COULDNT_CONNECT				= KY_HTTP_ERR_BEGIN + 0x00000003, // Couldn't connect to server
	KY_HTTP_OUT_OF_MEMORY				= KY_HTTP_ERR_BEGIN + 0x00000004, // Out of memory
	KY_HTTP_SSL_HANDSHAKE_FAIL			= KY_HTTP_ERR_BEGIN + 0x00000005, // SSL connect error
	KY_HTTP_SERVER_FAILED_VERIFICATION	= KY_HTTP_ERR_BEGIN + 0x00000006, // SSL peer certificate or SSH remote key was not OK
	KY_HTTP_SEND_ERROR					= KY_HTTP_ERR_BEGIN + 0x00000007, // Send data error
	KY_HTTP_RECV_ERROR					= KY_HTTP_ERR_BEGIN + 0x00000008, // Receive data error
	KY_HTTP_SSL_CERTPROBLEM				= KY_HTTP_ERR_BEGIN + 0x00000009, // Problem with the local SSL certificate
	KY_HTTP_REQUEST_TIMEOUT				= KY_HTTP_ERR_BEGIN + 0x00000010, // Connetion timeout
	KY_HTTP_CREATEDATA_REQUEST_FAIL		= KY_HTTP_ERR_BEGIN + 0x00000011, // CUSTOM: create request data failed
	KY_HTTP_INIT_REQUEST_FAIL			= KY_HTTP_ERR_BEGIN + 0x00000012, // CUSTOM: init request failed
	KY_HTTP_USER_FORCE_STOP				= KY_HTTP_ERR_BEGIN + 0x00000013, // CUSTOM: user force stop
};

#define PASS_ERROR_CODE(code, exec) if(code == HttpErrorCode::KY_HTTP_OK) { code = exec;}
#define PASS_CURL_EXEC(code, exec)  if(code == CURLcode::CURLE_OK) { code = exec;}
#define CHECK_HTTP_ERROR_OK(code, exec) ((code = exec) == HttpErrorCode::KY_HTTP_OK)

#define COOKIE_SEP  "\t"

enum ContentType
{
	none		,
	urlencoded	,
	multipart	,
	raw			,
	audio		,
};

enum HttpContentType
{
	// application
	Auto,
	application_x_www_form_urlencoded,
	application_octet_stream		,
	application_xml					,
	application_json				,

	text_html						,
	text_javascript					,
	text_json						,
	text_plain						,
	text_xml						,

	multipart_mixed					,
	multipart_alternative			,
	multipart_related				,
	multipart_form_data				,
};

static std::string get_string_content_type(HttpContentType type)
{
	switch (type)
	{
	case kyhttp::Auto:
		break;
	case kyhttp::application_x_www_form_urlencoded:
		return "application/x-www-form-urlencoded";
		break;
	case kyhttp::application_octet_stream:
		return "application/octet-stream";
		break;
	case kyhttp::application_xml:
		return "application/xml";
		break;
	case kyhttp::application_json:
		return "application/json";
		break;
	case kyhttp::text_html:
		return "text/html";
		break;
	case kyhttp::text_javascript:
		break;
	case kyhttp::text_json:
		break;
	case kyhttp::text_plain:
		return "text/plain";
		break;
	case kyhttp::text_xml:
		break;
	case kyhttp::multipart_mixed:
		break;
	case kyhttp::multipart_alternative:
		break;
	case kyhttp::multipart_related:
		break;
	case kyhttp::multipart_form_data:
		return "multipart/form-data";
		break;
	default:
		break;
	}

	return "";
}

enum HttpMethod
{
	POST,
	GET,
};

enum ProxyType
{
	KY_PROXY_HTTP    =  CURLPROXY_HTTP, //(default)
	KY_PROXY_HTTPS   =  CURLPROXY_HTTPS,
	KY_PROXY_SOCKS4  =  CURLPROXY_SOCKS4,
	KY_PROXY_SOCKS4A =  CURLPROXY_SOCKS4A,
	KY_PROXY_SOCKS5  =  CURLPROXY_SOCKS5,
};


struct WebProxy
{
	std::string  m_hostname;				// Host ip		:Ex: 192.168.111.124
	UINT		 m_port;					// Port number  :Ex: 80
	ProxyType	 m_proxy_type;				// Connect type :HTTP, HTTPS, SOCKS4, SOCKS4A, SOCKS5
	std::string	 m_username;				// User name
	std::string	 m_password;				// Password
};

struct SSLSetting
{
	BOOL m_verify_ssl_certificate  = TRUE;  // Verify ssl certificate
	BOOL m_verify_host_certificate = TRUE;  // Verify host certificate
};

struct HttpCookieData
{
	std::string		m_domain_name;			// Domain name
	BOOL			m_include_subdomains;	// Include subdomains boolean  TRUE|FALSE
	std::string		m_path;					// Path
	BOOL			m_secure;				// Set over a secure transport
	time_t			m_expires;				// Expires at – seconds since Jan 1st 1900, or 0
	std::string		m_name;					// Name of the cookie
	std::string		m_content;				// Value of the cookie
};

struct HttpClientOption
{
	BOOL	m_show_request = TRUE;			// Show request
	UINT	m_retry_connet;					// Number of connection attempts if failed
	ULONG	m_connect_timout = 0;			// Time-out connect operations after this amount of seconds				- milliseconds
	ULONG	m_max_download_speed;			// Limit-rate: maximum number of bytes per second to receive			- kb/s
	ULONG	m_max_upload_speed;				// Limit-rate: maximum number of bytes per second to send				- kb/s
	BOOL	m_auto_redirect = FALSE;		// automatically send request if response is move MOVED_PERMANENTLY		|TRUE / FALSE
	BOOL	m_process_cookie = FALSE;		// does not process cookies received									|TRUE / FALSE
	BOOL	m_get_server_time = FALSE;		// flag get system time information based on response					|TRUE / FALSE
};

struct HttpClientProgress
{
	int		m_action;
	BOOL	m_force_stop = FALSE;

	double	m_cur_upload;
	double	m_cur_download;

	double	m_total_download;
	double	m_total_upload;
};

struct HttpHeaderData
{
	std::string			m_request_param;	// :custom request param
	std::string			m_host;				// :custom host request
	HttpContentType		m_content_type;		// :n/a
	std::string			m_accept_encoding;	// :specify accept encoding 
	std::string			m_accept;			// :specify accept header

	std::vector<std::string> m_extension;
};

/*==================================================================================
* interface HttpContent
===================================================================================*/
interface HttpContent
{
protected:
	virtual void* InitContent(IN void* base = NULL) = 0;
	virtual ContentType GetType() const = 0;

	friend class HttpRequest;
};

/*==================================================================================
* interface HttpResponse
===================================================================================*/
interface IHttpClient
{
	virtual HttpErrorCode   Request(IN const HttpMethod method, IN const Uri uri, IN HttpRequest* request) = 0;
	virtual HttpErrorCode   Post(IN const Uri uri, IN HttpRequest* request) = 0;
	virtual HttpErrorCode   Get(IN const Uri uri, IN HttpRequest* request) = 0;

	virtual void			Configunation(IN HttpClientOption& option) = 0;
	virtual void			SettingProxy(IN WebProxy& proxy_info)  = 0;
	virtual void			SettingSSL(IN SSLSetting& ssl_setting) = 0;
	virtual void			AddCookie(IN const char* str_cookie) = 0;
	virtual HttpResponsePtr Response() const = 0;
};

/*==================================================================================
* class IKeyValue : List key value 
===================================================================================*/
class IKeyValue
{
protected:
	struct KeyValueParam;
	typedef std::vector<KeyValueParam> KEY_VALUE_LIST;

protected:
	struct KeyValueParam
	{
		std::string key;
		std::string value;
	};

protected:
	KEY_VALUE_LIST	m_keyvalue;

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

/*==================================================================================
* class Uri : Uniform Resource Identifier
* Include : location, query param
===================================================================================*/
class Uri : private IKeyValue
{
private:
	std::string		location;

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

	std::string get_url() const
	{
		std::string url = location;
		std::string query_param = get_query_param();

		if (!query_param.empty())
		{
			url.append("?" + query_param);
		}

		return url;
	}

	friend class HttpClient;
};

/*==================================================================================
* Template class ArrayObject : list vector share pointer object
===================================================================================*/
template<typename T>
class ArrayObject
{
public:
	typedef std::shared_ptr<T>   ObjectPtr;
	typedef std::vector<ObjectPtr> OBJECT_LIST;

protected:
	OBJECT_LIST m_data_list;

public:
	ObjectPtr operator[](const size_t i)
	{
		return this->Get(i);
	}

	ObjectPtr Get(const size_t i)
	{
		if (i >= 0 && i < m_data_list.size())
		{
			return m_data_list[i];
		}
		return nullptr;
	}

	virtual void Clear()
	{
		m_data_list.clear();
	}

	bool Empty()
	{
		return m_data_list.empty();
	}
};

__END___NAMESPACE__