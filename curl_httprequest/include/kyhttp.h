#pragma once

#include <Windows.h>
#include <string>
#include <memory>


#define __BEGIN_NAMESPACE__ namespace kyhttp {
#define __END___NAMESPACE__ }


__BEGIN_NAMESPACE__


interface HttpClient;
std::shared_ptr<HttpClient> HttpClienttPtr;

interface HttpHeaderInfo;
std::shared_ptr<HttpHeaderInfo> HttpHeaderInfoPtr;

interface HttpContentInfo;
std::shared_ptr<HttpContentInfo> HttpContentInfoPtr;

interface HttpRequest;
std::shared_ptr<HttpRequest> HttpRequestPtr;

interface HttpResponse;
std::shared_ptr<HttpResponse> HttpResponsePtr;



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
	MULTIPART,
	URLENCODE
};

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
public:
	std::wstring uri;
};

struct HttpRequestOption
{
	int		m_retry_connet;		  // count
	float	m_connect_timout;	  // milliseconds
	float	m_max_download_speed; // kb/s
	float	m_max_upload_speed;	  // kb/s
};

struct HttpClientProgress
{
public:
	int		m_action;
	int		m_force_stop = false;

	double	m_cur_upload;
	double	m_cur_download;

	double	m_total_download;
	double	m_total_upload;
};

struct HttpHeaderData
{
	std::string		 m_request_param;	 // :))
	std::string		 m_host;			 // :))
	HttpContentType  m_content_type;	 // :))
	std::string		 m_accept_encoding;	 // :))
	std::string		 m_accept;			 // :))
};

__END___NAMESPACE__