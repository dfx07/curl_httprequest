#pragma once

#include "kyhttp.h"
#include "kyhttprequest.h"

__BEGIN_NAMESPACE__

interface HttpClient
{
protected:
	HttpClientProgress	m_progress;
public:
	virtual HTTPStatusCode Post(RequestUri	uri, HttpRequest* request) = 0;
	virtual HTTPStatusCode Get(RequestUri	uri, HttpRequest* request) = 0;
};

__END___NAMESPACE__