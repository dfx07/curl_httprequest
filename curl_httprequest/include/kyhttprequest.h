#pragma once

#include "kyhttp.h"

__BEGIN_NAMESPACE__

struct RequestUri
{
public:
	std::wstring uri;
};


interface HttpHeaderInfo
{
protected:
	virtual int		Create() = 0;
	virtual void*	GetBase() = 0;

public:
	virtual std::string		GetRawContent() = 0;
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
	HttpClientProgress		m_progress;
};


__END___NAMESPACE__