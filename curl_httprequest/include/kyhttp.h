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

#include <Windows.h>
#include <string>
#include <memory>


#define __BEGIN_NAMESPACE__ namespace kyhttp {
#define __END___NAMESPACE__ }


__BEGIN_NAMESPACE__


#ifndef interface
#define interface struct
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif


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


#define KY_HTTP_ERR_BEGIN 0x0111122

enum KYHttpErrorCode
{
	KY_HTTP_FAILED						=-1,
	KY_HTTP_OK							= 123,
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
};

enum HttpContentType
{
	none		,
	application	,
	multipart	,
	image		,
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

struct WebProxy
{
	std::string  m_hostname;
	std::string  m_port;
	LONG		 m_proxy_type;
	std::wstring m_username;
	std::string  m_proxy_domain;
	std::wstring m_password;
};

struct SSLSetting
{

};

struct HttpCookie
{

};



struct RequestUri
{
	std::string uri;
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
	bool	m_show_request = true;		  // show request
	int		m_retry_connet;		  // count
	float	m_connect_timout;	  // milliseconds
	float	m_max_download_speed; // kb/s
	float	m_max_upload_speed;	  // kb/s
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
	std::string				m_request_param;	// :))
	std::string				m_host;				// :))
	HttpDetailContentType	m_content_type;		// :))
	std::string				m_accept_encoding;	// :))
	std::string				m_accept;			// :))
};

///*==================================================================================
//* interface HttpHeader
//===================================================================================*/
//interface HttpHeader
//{
//	virtual int	 Create(IN void* base) = 0;
//
//	virtual void SetRequestParam(IN const char* req_param) = 0;
//	virtual void SetContentType(IN HttpDetailContentType type) = 0;
//	virtual void SetAccept(IN const char* accept) = 0;
//	virtual void SetAcceptEncoding(IN const char* accept_encoding) = 0;
//	virtual void SetHost(IN const char* host) = 0;
//};

/*==================================================================================
* interface HttpContent
===================================================================================*/
interface HttpContent
{
	virtual int UpdateContent(IN void* base = NULL) = 0;

	virtual HttpContentType GetType() const = 0;
};

///*==================================================================================
//* interface HttpRequest
//===================================================================================*/
//interface HttpRequest
//{
//	virtual int  Create(IN void* base) = 0;
//
//	virtual HttpMethod GetMethod() const = 0;
//	virtual RequestUri GetUri() const = 0;
//};

///*==================================================================================
//* interface HttpResponse
//===================================================================================*/
//interface HttpResponse
//{
//	virtual void		 SetStatusCode(long code) = 0;
//	virtual HTTPCode     GetStatusCode() const = 0;
//	virtual std::string	 Header() const = 0;
//	virtual std::string	 Content() const = 0;
//	virtual void		 Clear() = 0;
//};

/*==================================================================================
* interface HttpResponse
===================================================================================*/
interface IHttpClient
{
	virtual HTTPStatusCode  Request(IN const HttpMethod method, IN const RequestUri uri, IN HttpRequest* request) = 0;
	virtual HTTPStatusCode  Post(IN const RequestUri uri, IN HttpRequest* request) = 0;
	virtual HTTPStatusCode  Get(IN const RequestUri uri, IN HttpRequest* request) = 0;

	virtual void			SetConfigunation(IN HttpClientOption option) = 0;
	virtual void			SettingProxy(IN WebProxy& proxy_info) = 0;
	virtual void			SettingCookie(IN HttpCookie& cookie) = 0;
	virtual HttpResponsePtr Response() const = 0;
};


__END___NAMESPACE__