#pragma once

#include "kyhttp.h"

__BEGIN_NAMESPACE__

interface HttpHeaderInfo
{
private:
	HttpHeaderData			m_header_data;

protected:
	virtual int				Create() = 0;
	virtual void*			GetBase() = 0;

public:
	virtual std::string		GetRawContent() = 0;

public:
	void set_request_param(const char* req_param)
	{
		m_header_data.m_request_param = req_param;
	}
	void set_content_type(HttpContentType contenttype)
	{
		m_header_data.m_content_type = contenttype;
	}
	void set_accpet(const char* accept)
	{
		m_header_data.m_accept_encoding = accept;
	}
	void set_host(const char* host)
	{
		m_header_data.m_host = host;
	}
};

interface HttpContentInfo
{
protected:
	virtual HttpContentType		GetType() = 0;
	virtual int					Create(HttpClient* request = NULL) = 0;
	virtual void*				GetBase() = 0;

public:
	virtual std::string			GetRawContent() = 0;
};


interface HttpRequest
{
protected:
	HttpHeaderInfo*		m_header;
	HttpContentInfo*	m_content;

protected:
	HttpRequestOption		m_option;
	HttpMethod				m_method;
};


__END___NAMESPACE__