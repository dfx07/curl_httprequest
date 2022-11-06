#pragma once

#include "kyhttp.h"

__BEGIN_NAMESPACE__

interface HttpResponse
{
	virtual HTTPStatusCode Status() const = 0;
	virtual std::string	   Header() const = 0;
	virtual std::string	   Content() const = 0;
};

__END___NAMESPACE__