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

#include <Windows.h>
#include <string>
#include <memory>

#include "kyhttpdef.h"


__BEGIN_NAMESPACE__

interface HttpClient;
typedef std::shared_ptr<HttpClient> HttpClienttPtr;

interface HttpHeader;
typedef std::shared_ptr<HttpHeader> HttpHeaderPtr;

interface HttpContent;
typedef std::shared_ptr<HttpContent> HttpContentPtr;

interface HttpRequest;
typedef std::shared_ptr<HttpRequest> HttpRequestPtr;

interface HttpResponse;
typedef std::shared_ptr<HttpResponse> HttpResponsePtr;

class Uri;


enum HTTPCode
{
	NODEFINE				= -1 , // No support
	SUCCESS					= 200, // The request succeeded
	ACCEPTED				= 202, // The request has been received but not yet acted upon
	NO_CONTENT				= 204, // There is no content to send for this request, but the headers may be useful
	MOVED_PERMANENTLY		= 301, // The URL of the requested resource has been changed permanently. The new URL is given in the response.
	SEE_OTHER				= 303, // This response code means that the URI of requested resource has been changed temporarily
	BAD_REQUEST				= 400, // The server cannot or will not process the request due to something that is perceived to be a client error 
	UNAUTHORIZED			= 401, // Although the HTTP standard specifies "unauthorized"
	FORBIDDEN				= 403, // The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource
	NOT_FOUND				= 404, // The server cannot find the requested resource
	METHOD_NOT_ALLOWED		= 405, // The request method is known by the server but is not supported by the target resource
	INTERNAL_SERVER_ERROR	= 500, // The server has encountered a situation it does not know how to handle
	NOT_IMPLEMENTED			= 501, // The request method is not supported by the server and cannot be handled
	SERVICE_UNAVAILABLE		= 503, // The server is not ready to handle the request
};

enum HTTPStatusCode
{
	OK						= 0x00000,
	FAILED					= 0x00002,
	NONE					= 0x00004,
	CONNECT_FAILED			= 0x00008,
	POST_FAILED				= 0x00016,
	CREATE_FAILED			= 0x00032,
};

enum HttpErrorCode
{
	KY_HTTP_FAILED						= 99999,
	KY_HTTP_OK							= 0,
	KY_HTTP_COULDNT_RESOLVE_PROXY		= KY_HTTP_ERR_BEGIN + 1,
	KY_HTTP_COULDNT_RESOLVE_HOST		= KY_HTTP_ERR_BEGIN + 2,
	KY_HTTP_COULDNT_CONNECT				= KY_HTTP_ERR_BEGIN + 3,
	KY_HTTP_OUT_OF_MEMORY				= KY_HTTP_ERR_BEGIN + 4,
	KY_HTTP_SSL_HANDSHAKE_FAIL			= KY_HTTP_ERR_BEGIN + 5,
	KY_HTTP_SERVER_FAILED_VERIFICATION	= KY_HTTP_ERR_BEGIN + 6,
	KY_HTTP_SEND_ERROR					= KY_HTTP_ERR_BEGIN + 7,
	KY_HTTP_RECV_ERROR					= KY_HTTP_ERR_BEGIN + 8,
	KY_HTTP_SSL_CERTPROBLEM				= KY_HTTP_ERR_BEGIN + 9,
	KY_HTTP_REQUEST_TIMEOUT				= KY_HTTP_ERR_BEGIN + 10,
	KY_HTTP_CREATE_REQUEST_FAIL			= KY_HTTP_ERR_BEGIN + 11,
};

#define PASS_ERROR_CODE(code, exec) if(code == HttpErrorCode::KY_HTTP_OK) { code = exec;}
#define PASS_CURL_EXEC(code, exec) if(code == CURLcode::CURLE_OK) { code = exec;}

enum HttpContentType
{
	none		,
	urlencoded	,
	multipart	,
	raw			,
	audio		,
};

enum HttpDetailContentType
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

enum HttpContentDispositionType
{
	_Auto,
	_form_data,
};

static std::string get_string_content_disposition_type(HttpContentDispositionType type)
{
	switch (type)
	{
	case kyhttp::Auto:
		break;
	case kyhttp::_form_data:
		return "form-data";
		break;
	}

	return "";
}

static std::string get_string_content_type(HttpDetailContentType type)
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
	std::string  m_hostname;
	unsigned int m_port;
	ProxyType	 m_proxy_type;
	std::wstring m_username;
	std::string  m_proxy_domain;
	std::wstring m_password;
};

struct SSLSetting
{
	bool m_disable_verify_ssl_certificate  = false; // verify ssl certificate
	bool m_disable_verify_host_certificate = false; // verify host certificate
};

struct HttpCookie
{

};

struct HttpRequestOption
{
	int		m_retry_connet;		  // count
	float	m_connect_timout;	  // milliseconds
	float	m_max_download_speed; // kb/s
	float	m_max_upload_speed;	  // kb/s
};

struct HttpClientOption
{
	bool	m_show_request = true;		// show request
	INT		m_retry_connet;				// count
	LONG	m_connect_timout;			// milliseconds
	float	m_max_download_speed;		// kb/s
	float	m_max_upload_speed;			// kb/s
	BOOL	m_auto_redirect = FALSE;	// auto redict  | TRUE / FALSE
};

struct HttpClientProgress
{
	int		m_action;
	int		m_force_stop = false;

	double	m_cur_upload;
	double	m_cur_download;

	double	m_total_download;
	double	m_total_upload;
};

struct HttpHeaderData
{
	std::string				 m_request_param;		// :))
	std::string				 m_host;				// :))
	HttpDetailContentType	 m_content_type;		// :))
	std::string				 m_accept_encoding;		// :))
	std::string				 m_accept;				// :))

	std::vector<std::string> m_extension;
};

/*==================================================================================
* interface HttpContent
===================================================================================*/
interface HttpContent
{
	virtual void* InitContent(IN void* base = NULL) = 0;
	virtual HttpContentType GetType() const = 0;
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
	virtual void			SettingCookie(IN HttpCookie& cookie) = 0;
	virtual HttpResponsePtr Response() const = 0;
};

/*==================================================================================
* class IKeyValue : List key value 
===================================================================================*/
class IKeyValue
{
	struct KeyValueParam;
	typedef std::vector<KeyValueParam> KEY_VALUE_LIST;

	struct KeyValueParam
	{
		std::string key;
		std::string value;
	};

protected:
	KEY_VALUE_LIST m_keyvalue;

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
* Include : + location
*			+ query param
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

	friend class HttpClient;
};

__END___NAMESPACE__