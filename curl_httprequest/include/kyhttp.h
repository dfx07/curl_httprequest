#pragma once

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



enum HTTPStatusCode
{
	OK						= 0x00000,
	FAILED					= 0x00002,
	NONE					= 0x00004,
	CONNECT_FAILED			= 0x00008,
	POST_FAILED				= 0x00016,
	CREATE_FAILED			= 0x00032,
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
	std::wstring m_user;
	std::wstring m_pass;
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
	bool	m_show_request;		  // show request
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

/*==================================================================================
* interface HttpHeader
===================================================================================*/
interface HttpHeader
{
	virtual int		Create(IN void* base, IN HttpHeaderData& header_data) = 0;
};

/*==================================================================================
* interface HttpContent
===================================================================================*/
interface HttpContent
{
	virtual HttpContentType GetType() const = 0;
	virtual int Create(IN void* base = NULL) = 0;
};

/*==================================================================================
* interface HttpRequest
===================================================================================*/
interface HttpRequest
{
	virtual int  Create(IN void* base) = 0;

	virtual void SetRequestParam(IN const char* req_param) = 0;
	virtual void SetContentType(IN HttpDetailContentType type) = 0;
	virtual void SetAccept(IN const char* accept) = 0;
	virtual void SetAcceptEncoding(IN const char* accept_encoding) = 0;
	virtual void SetHost(IN const char* host) = 0;
};

/*==================================================================================
* interface HttpResponse
===================================================================================*/
interface HttpResponse
{
	virtual void		   Clear() = 0;
	virtual HTTPStatusCode Status() const = 0;
	virtual std::string	   Header() const = 0;
	virtual std::string	   Content() const = 0;
};

/*==================================================================================
* interface HttpResponse
===================================================================================*/
interface HttpClient
{
	virtual HTTPStatusCode Post(IN const RequestUri uri ,IN HttpRequest* request) = 0;
	virtual HTTPStatusCode Get(IN const RequestUri uri,IN HttpRequest* request) = 0;
};


__END___NAMESPACE__