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

interface HttpHeaderInfo
{
	struct HttpHeaderData
	{
		std::string		 m_request_param;	 // :))
		std::string		 m_host;			 // :))
		HttpContentType  m_content_type;	 // :))
		std::string		 m_accept_encoding;	 // :))
		std::string		 m_accept;			 // :))
	};

private:
	HttpHeaderData	m_header_data;

protected:
	virtual int		Create() = 0;
	virtual void*	GetBase() = 0;

public:
	virtual std::string GetRawContent() = 0;

public:
	void set_request_param(IN const char* req_param)
	{
		m_header_data.m_request_param = req_param;
	}
	void set_content_type(IN HttpContentType contenttype)
	{
		m_header_data.m_content_type = contenttype;
	}
	void set_accpet(IN const char* accept)
	{
		m_header_data.m_accept_encoding = accept;
	}
	void set_host(IN const char* host)
	{
		m_header_data.m_host = host;
	}
};

interface HttpContentInfo
{
protected:
	virtual HttpContentType GetType() const = 0;
	virtual int Create(IN HttpClient* request = NULL) = 0;
	virtual void* GetData() = 0;

public:
	virtual std::string	GetRawContent() = 0;
};

/*==================================================================================
* interface HttpRequest
===================================================================================*/
interface HttpRequest
{
protected:
	HttpHeaderInfo*		m_header;
	HttpContentInfo*	m_content;

protected:
	HttpRequestOption	m_option;
	HttpMethod			m_method;
};

/*==================================================================================
* interface HttpResponse
===================================================================================*/
interface HttpResponse
{
	virtual HTTPStatusCode Status() const = 0;
	virtual std::string	   Header() const = 0;
	virtual std::string	   Content() const = 0;
};

/*==================================================================================
* interface HttpResponse
===================================================================================*/
interface HttpClient
{
protected:
	HttpClientProgress	m_progress;
public:
	virtual HTTPStatusCode Post(IN const RequestUri uri ,IN HttpRequest* request) = 0;
	virtual HTTPStatusCode Get(IN const RequestUri uri,IN HttpRequest* request) = 0;
};


__END___NAMESPACE__