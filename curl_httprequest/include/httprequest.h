#pragma once
#include <curl/curl.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

enum content_type
{
	none					=-1, //not define
	form_data				= 0, //multipart/form-data
	octet_stream			= 1, //application/octet-stream
	x_www_form_urlencoded	= 2, //application/x-www-form-urlencoded
	raw						= 3, //text/plain
};

std::string get_string_content_type(content_type type)
{
	switch (type)
	{
	case form_data:
		return "multipart/form-data";
	case octet_stream:
		return "application/octet-stream";
	case x_www_form_urlencoded:
		return "application/x-www-form-urlencoded";
	case raw:
		return "text/plain";
	default:
		break;
	}
	return "";
}

enum HTTPRequestCode
{
	OK						= 0x00000,
	FAILED					= 0x00002,
	NONE					= 0x00004,
	CONNECT_FAILED			= 0x00008,
	POST_FAILED				= 0x00016,
	CREATE_FAILED			= 0x00032,
};

enum post_type
{
	multipart,
	urlencode
};

enum request_action
{
	POST,
	GET,
};


interface body_info;
interface header_info;
interface HTTPRequest;

interface body_info
{
	virtual post_type	type() = 0;
	virtual int			create(HTTPRequest* request = NULL) = 0;
	virtual void*		get() = 0;
};


interface header_info
{
	virtual int   create() = 0;
	virtual void* get() = 0;
};

interface HTTPRequest
{
	virtual void* base() = 0;
	virtual HTTPRequestCode post(std::string url, header_info* header, body_info* post_info) = 0;
	virtual HTTPRequestCode get(std::string url, header_info* header, body_info* body_info) = 0;
};

struct key_value
{
	std::string key;
	std::string value;
};

class curl_header_info : public header_info
{
private:
	struct curl_slist*  m_curl_slist;

private:
	void header_format_replace(const char* format, ...)
	{
		const int nbufsize = 1024;
		char buf[nbufsize];
		memset(buf, 0, nbufsize);

		// list paramater write file follow format
		va_list args;
		va_start(args, format);
		vsnprintf(buf, nbufsize, format, args);
		va_end(args);

		// append buff to output
		m_curl_slist = curl_slist_append(m_curl_slist, buf);
	}

	bool is_header_ok()
	{
		return m_curl_slist? true : false;
	}

protected:
	std::string   m_request_param;
	std::string   m_host;
	content_type  m_content_type;
	std::string   m_accept_encoding;
	std::string   m_accept;

public:
	void set_request_param(const char* req_param)
	{
		m_request_param = req_param;
	}
	void set_content_type(content_type contenttype)
	{
		m_content_type = contenttype;
	}
	void set_accpet(const char* accept)
	{
		m_accept_encoding = accept;
	}
	void set_host(const char* host)
	{
		m_host = host;
	}
public:
	curl_header_info(): m_curl_slist(NULL)
	{
		m_content_type = content_type::form_data;
	}

	void free()
	{
		curl_slist_free_all(m_curl_slist);
		m_curl_slist = NULL;
	}

public:

	void* get()
	{
		return m_curl_slist;
	}

	int create()
	{
		this->free(); // clear old data

		// Example default when not create header
		/*================================================================================
		> POST /upload.php HTTP/1.1
		> Host: 127.0.0.1:8080
		> Accept: *//*
		> Content - Length: 13505
		> Content - Type : multipart / form - data; boundary = ------------------------ee173679459a9212
		================================================================================*/
		// Note : boundary auto create
		//if (!m_request_param.empty())	header_format_replace("POST %s HTTP/1.1", m_request_param.c_str());
		if (m_content_type != none)		header_format_replace("Content-Type: %s", get_string_content_type(m_content_type).c_str());
		if (!m_host.empty())			header_format_replace("Host: %s", m_host.c_str());
		if (!m_accept.empty())			header_format_replace("Accept: %s", m_accept.c_str());
		if (!m_accept_encoding.empty()) header_format_replace("Accept-encoding: %s", m_accept_encoding.c_str());
		

		header_format_replace("Connection: Keep-Alive");
		header_format_replace("User-Agent: Brinicle");
		header_format_replace("Pragma: no-cache");
		header_format_replace("Cache-Control: no-cache");

		if (!is_header_ok()) // create slist failed -> why?
			return -1;

		return 1;
	}


};

class curl_multipart_info : public body_info
{
private:
	struct curl_mime*	   m_curl_mine;
private:
	content_type   m_content_type;
	std::string	   m_strname;
	std::string	   m_strvalue;

	void*		m_data;
	size_t		m_nsize;

public:
	post_type type()
	{
		return post_type::multipart;
	}

	void set_name(const char* name)
	{
		m_strname = name;
	}
	void set_value_name(const char* value)
	{
		m_strvalue = value;
	}

	void set_data(void* data, size_t nsize)
	{
		m_data = data;
		m_nsize = nsize;
	}

private:

	void free()
	{
		curl_mime_free(m_curl_mine);
	}

public:
	curl_multipart_info() : m_data(NULL), m_curl_mine(NULL),
		m_nsize(0), m_strname(""), m_strvalue("")
	{
		m_content_type = content_type::form_data;
	}

	~curl_multipart_info()
	{
		free();
	}

	int create(HTTPRequest* request)
	{
		CURL* curl = static_cast<CURL*>(request->base());
		if (!request || !curl)
			return -1;

		// clear old data
		this->free();

		/* Create the form */
		m_curl_mine = curl_mime_init(curl);

		/* Fill in the file upload field */
		auto field = curl_mime_addpart(m_curl_mine);
		curl_mime_type(field, get_string_content_type(m_content_type).c_str());
		curl_mime_name(field, m_strname.c_str());
		curl_mime_filedata(field, m_strvalue.c_str());
		curl_mime_data(field, (const char*)m_data, m_nsize);
		return 1;
	}

	void* get()
	{
		return m_curl_mine;
	}
};

class curl_urlencode_info : public body_info
{
private:
	std::vector<key_value> m_data;
	std::string			   m_curl_kv;

private:
	void free()
	{
		m_data.clear();
		m_curl_kv.clear();
	}

public:

	void add_data(const char* key, const char* value)
	{
		m_data.push_back({ key, value });
	}

	void add_data(const char* key, const std::string& value)
	{
		m_data.push_back({ key, value });
	}

	// cnv = convert bool(true, false) to int (1,0)
	void add_data(const char* key, const bool value, const bool cnv = false)
	{
		std::string vl;
		if(cnv)	vl = value ? "true" : "false";
		else    vl = value ? "1" : "0";
		m_data.push_back({ key, vl });
	}

	template<typename T, typename std::enable_if<std::is_arithmetic<T>::value,T>::type* = nullptr>
	void add_data(const char* key, const T& value)
	{
		std::string t = std::to_string(value);
		m_data.push_back({ key, t });
	}
public:
	post_type type()
	{
		return post_type::urlencode;
	}

	int create(HTTPRequest* request)
	{
		CURL* curl = static_cast<CURL*>(request->base());
		if (!request || !curl)
			return -1;

		if (!m_data.empty())
			m_curl_kv.append('&' + m_data[0].key + '=' + m_data[0].value);

		for (int i = 1; i < m_data.size(); i++)
		{
			m_curl_kv.append('&' + m_data[0].key + '=' + m_data[0].key);
		}

		return 1;
	}

	void* get()
	{
		return (void*)&m_curl_kv;
	}
};




struct HTTPRequestOption
{
	double  dconnect_timeout = 1000;
	int		bshow_request	= false;
	int		itry_connect	= 5;
	int		buse_process_bar = false;


	double	dlimit_download;		// KB/s
	double  dlimit_upload;		// KB/s
};

struct HTTPRequestProgress
{
public:
	int		m_action;
	int		m_force_stop = false;

	double	m_cur_upload;
	double	m_cur_download;

	double	m_total_download;
	double	m_total_upload;
};

struct HttpResponseBuffer
{
public:
	std::string header;
	std::string body;
	
public:
	void set_head(const char* data, const int nsize)
	{
		this->header.assign(data, nsize);
	}
	void set_head_append(const char* data, const int nsize)
	{
		this->header.append(data, nsize);
	}
	void set_body(const char* data, const int nsize)
	{
		this->header.assign(data, nsize);
	}
	void set_body_append(const char* data, const int nsize)
	{
		this->body.append(data, nsize);
	}


	/*get don't care*/
	const char* get_head() const
	{
		return header.c_str();
	}

	int get_head_length() const
	{
		return header.length();
	}

	const char* get_body()
	{
		return body.c_str();
	}

	int get_body_length() const
	{
		return body.length();
	}

	void clear()
	{
		header.clear();
		body.clear();
	}
};

class CurlHTTPRequest : public HTTPRequest
{

private:

private:
	static int progress_func(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
	{
		// not setup
		if (ptr == NULL)
			return 0;

		HTTPRequestProgress* process = static_cast<HTTPRequestProgress*>(ptr);

		// stop download or upload data network
		if (process && process->m_force_stop)
		{
			return 1;
		}

		if (TotalToUpload > 0 && process)
		{
			process->m_action = 1;
			process->m_cur_upload = NowUploaded;
			process->m_total_upload = TotalToUpload;
		}
		else if (TotalToDownload > 0 && process)
		{
			process->m_action = 2;
			process->m_cur_download = NowDownloaded;
			process->m_total_download = TotalToDownload;
		}
		return 0;
	}

	static size_t write_data_response_callback(void* contents, size_t size, size_t nmemb, void* userp)
	{
		((std::string*)userp)->append((char*)contents, size * nmemb);
		return size * nmemb;
	}

public:
	virtual void* base()
	{
		return m_curl;
	}
	void set_option(HTTPRequestOption option)
	{
		m_option = option;
	}

private:
	HTTPRequestCode convert_code(CURLcode code)
	{
		if (code != CURLE_OK)
			printf("Error: %s\n", curl_easy_strerror(code));

		if (code >= CURLE_COULDNT_RESOLVE_PROXY && code <= CURLE_REMOTE_ACCESS_DENIED)
			return HTTPRequestCode::CONNECT_FAILED;

		return HTTPRequestCode::OK;
	}

private:

	/******************************************************************************
	*! @brief  : apply option to curl 
	*! @author : thuong.nv - [Date] : 03/10/2022
	*! @parameter:	action : get / post
	*!				url : url action
	*! @return : int : nsize / data : buff
	******************************************************************************/
	virtual HTTPRequestCode apply_option(std::string url, request_action action)
	{
		if (!m_curl || url.empty())
			return HTTPRequestCode::CREATE_FAILED;

		curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
		if (action == request_action::POST)
		{
			curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
		}

		if (m_option.bshow_request)
			curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

		if (m_option.dconnect_timeout > 0)
			curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT_MS, m_option.dconnect_timeout);

		// Internal CURL progressmeter must be disabled if we provide our own callback
		if (m_option.buse_process_bar)
		{
			curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, FALSE);
			curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, &m_progess_data);
			// Install the callback function
			curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, &CurlHTTPRequest::progress_func);
		}

		if (m_option.dlimit_upload > 0)
		{
			// limit upload kb.s
			curl_off_t max_speed = 1000 * m_option.dlimit_upload; // 25kB/s
			curl_easy_setopt(m_curl, CURLOPT_MAX_SEND_SPEED_LARGE, max_speed);
		}
		if (m_option.dlimit_download > 0)
		{
			// limit download kb.s
			curl_off_t max_speed = 1000 * m_option.dlimit_download; // 25kB/s
			curl_easy_setopt(m_curl, CURLOPT_MAX_RECV_SPEED_LARGE, m_option.dlimit_download);
		}

		m_response_buffer.clear();

		if (action == request_action::GET)
		{
			curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, &CurlHTTPRequest::write_data_response_callback);
			curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &m_response_buffer.header);
			curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &CurlHTTPRequest::write_data_response_callback);
			curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_response_buffer.body);
		}

		return HTTPRequestCode::OK;
	}

	/******************************************************************************
	*! @brief  : setup header data package
	*! @author : thuong.nv - [Date] : 03/10/2022
	*! @parameter:	header : header info struct
	*! @return : HTTPRequestCode : OK , FAILED
	******************************************************************************/
	virtual HTTPRequestCode apply_header_data(header_info* header)
	{
		if (!m_curl || !header)
			return HTTPRequestCode::FAILED;

		if(header->create() <= 0)
			return HTTPRequestCode::FAILED;


		curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, (curl_slist*)header->get());

		return HTTPRequestCode::OK;
	}

	/******************************************************************************
	*! @brief  : setup body data package
	*! @author : thuong.nv - [Date] : 03/10/2022
	*! @parameter:	header : header info struct
	*! @return : HTTPRequestCode : OK , FAILED
	******************************************************************************/
	virtual HTTPRequestCode apply_body_data(body_info* body)
	{
		if (!m_curl || !body)
			return HTTPRequestCode::FAILED;

		if (body->create(this) <= 0)
			return HTTPRequestCode::FAILED;


		if (body->type() == post_type::multipart)
		{
			curl_easy_setopt(m_curl, CURLOPT_MIMEPOST, (curl_mime*)body->get());
		}
		else if (body->type() == post_type::urlencode)
		{
			std::string& pst_data = *((std::string*)body->get());
			curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, pst_data.c_str());
			curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, pst_data.length());
		}

		return HTTPRequestCode::OK;
	}

	virtual HTTPRequestCode send_request()
	{
		if (!m_curl)
			return HTTPRequestCode::FAILED;

		CURLcode res = curl_easy_perform(m_curl);

		curl_easy_getinfo(m_curl, CURLINFO_SPEED_UPLOAD, &m_speed_send);
		curl_easy_getinfo(m_curl, CURLINFO_TOTAL_TIME, &m_time_send);
		
		curl_easy_getinfo(m_curl, CURLINFO_SPEED_DOWNLOAD, &m_speed_download);

		return convert_code(res);
	}

public:
	virtual HTTPRequestCode post(std::string url, header_info* header, body_info* post)
	{
		if (!m_curl || url.empty())
			return HTTPRequestCode::CREATE_FAILED;

		HTTPRequestCode ret = this->apply_option(url, request_action::POST);
		if (ret != HTTPRequestCode::OK)
			return ret;

		ret = this->apply_header_data(header);
		if (ret != HTTPRequestCode::OK)
			return ret;

		ret = this->apply_body_data(post);
		if (ret != HTTPRequestCode::OK)
			return ret;

		HTTPRequestCode rqret = this->send_request();

		return rqret;
	}

	virtual HTTPRequestCode get(std::string url, header_info* header, body_info* get = NULL)
	{
		if (!m_curl || url.empty())
			return HTTPRequestCode::CREATE_FAILED;

		HTTPRequestCode ret = this->apply_option(url, request_action::GET);
		if (ret != HTTPRequestCode::OK)
			return ret;

		ret = this->apply_header_data(header);
		if (ret != HTTPRequestCode::OK)
			return ret;

		if (get) // no body data
		{
			ret = this->apply_body_data(get);
			if (ret != HTTPRequestCode::OK)
				return ret;
		}

		HTTPRequestCode rqret = this->send_request();

		return rqret;
	}

	virtual const char* get_header_response()
	{
		return m_response_buffer.get_head();
	}

	virtual const char* get_body_response()
	{
		return m_response_buffer.get_body();
	}

public:
	CurlHTTPRequest()
	{
		this->init_curl();
	}

	CurlHTTPRequest(const HTTPRequestOption& option)
	{
		this->init_curl();
		m_option = option;
	}

	~CurlHTTPRequest()
	{
		destroy_curl();
	}

private:
	int init_curl()
	{
		m_curl = curl_easy_init();

		return 1;
	}

	void destroy_curl()
	{
		curl_easy_cleanup(m_curl);
	}
private:
	double					m_speed_send;
	double					m_time_send;
	double					m_speed_download;
	double					m_time_download;

private:
	CURL*					m_curl;
	HTTPRequestOption		m_option;
	HTTPRequestProgress		m_progess_data;

	HttpResponseBuffer		m_response_buffer;

private:
};

typedef std::shared_ptr<header_info>			header_info_ptr;
typedef std::shared_ptr<body_info>				body_info_ptr;
typedef std::shared_ptr<curl_header_info>		curl_header_info_ptr;
typedef std::shared_ptr<curl_multipart_info>	curl_multipart_info_ptr;
typedef std::shared_ptr<curl_urlencode_info>	curl_urlencode_info_ptr;
