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

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>

#include "kyhttp_types.h"
#include "kyhttp_buffer.h"
#include <kyhttp_logger.h>

__BEGIN_NAMESPACE__


#define CURL_STATICLIB

#ifdef _DEBUG
#pragma comment (lib, "libcurl_debug.lib")
#else
#pragma comment (lib, "libcurl.lib")
#endif // _DEBUG

#pragma comment (lib, "Normaliz.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "advapi32.lib")

#define SSL_CERTIFICATE_TYPE_PEM   "PEM"


/*==================================================================================
	HttpContent :
		-> HttpRawContent			 : text, json, xml, javascript, html
		-> HttpUrlEncodedContent	 : encode param to body request
		-> HttpMultipartContent		 : post multipart data
	HttpRequest :
	HttpResponse:
	HttpClient  :
===================================================================================*/

/*==================================================================================
* Class HttpRawContent
* Content support: text, json, xml, javascript, html
===================================================================================*/
class HttpRawContent : public HttpContent
{
public:
	enum RAW_TYPE
	{
		text,
		json,
		xml,
		javascript,
		html
	};
private:
	HttpBuffer	  m_rawbuffer;

	RAW_TYPE      m_rawtype;
	std::string   m_str_rawtype; 

private:
	virtual ContentType GetType() const
	{
		return ContentType::raw;
	}

	virtual void* InitContent(IN void* base)
	{
		return &m_rawbuffer;
	}

private:
	static std::string ConvertTypeToString(RAW_TYPE type)
	{
		switch (type)
		{
		case kyhttp::HttpRawContent::text:
			return "Content-Type: text/plain";
			break;
		case kyhttp::HttpRawContent::json:
			return "Content-Type: application/json";
			break;
		case kyhttp::HttpRawContent::xml:
			return "Content-Type: application/xml";
			break;
		case kyhttp::HttpRawContent::javascript:
			return "Content-Type: application/javascript";
			break;
		case kyhttp::HttpRawContent::html:
			return "Content-Type: text/html";
			break;
		default:
			break;
		}
		return "";
	}
public:
	void SetRawData(const void* data, const unsigned int& size)
	{
		m_rawbuffer.set(data, size);
	}
	
	void SetRawData(const wchar_t* path)
	{
		void* data = NULL;
		int nbytes = kyhttp::read_data_file(path, &data);

		if (nbytes > 0)
		{
			this->SetRawData(data, nbytes);
		}
		else
		{
			KY_HTTP_LOG_WARN(L"[RawContent] Miss read data file: %s", data);
		}
		delete[] data;
	}

	void SetRawType(RAW_TYPE type)
	{
		m_rawtype = type;
		m_str_rawtype = ConvertTypeToString(type);
	}

	RAW_TYPE GetRawType()
	{
		return m_rawtype;
	}
	const char* GetRawTypeToString()
	{
		return m_str_rawtype.c_str();
	}
};

/*==================================================================================
* Class HttpUrlEncodedContent
* Container for name/value tuples encoded using application/x-www-form-urlencoded MIME type
* The query parameter is contained in the content request
===================================================================================*/
class HttpUrlEncodedContent : public HttpContent, public IKeyValue
{
private:
	// It is cache for later use -> please don't delete
	std::string		m_str_curl_content_cache;
	HttpBuffer		m_urlencode_content;
	std::string		m_raw_data;

public:
	HttpUrlEncodedContent() : m_str_curl_content_cache(),
		m_raw_data()
	{
	}
private:
	void clear_urlencode_data()
	{
		m_urlencode_content.clear();
	}

	void add_urlencode_data(const KeyValueParam* kv)
	{
		if (!kv) return;
		std::string temp;

		// append key value data
		if (m_urlencode_content.empty())
			temp = kv->key + '=' + kv->value;
		else
			temp = '&' + kv->key + '=' + kv->value;
		m_urlencode_content.append(temp.c_str(), temp.length());
	}

private:
	virtual ContentType GetType() const
	{
		return ContentType::urlencoded;
	}

	virtual void* InitContent(IN void* base)
	{
		CURL* curl = static_cast<CURL*>(base);
		if (!curl) return NULL;

		this->clear_urlencode_data();

		// append raw data user custom
		for (int i = 0; i < m_keyvalue.size(); i++)
		{
			this->add_urlencode_data(&m_keyvalue[i]);
		}

		m_urlencode_content.append(m_raw_data.c_str(), m_raw_data.length());

		return &m_urlencode_content;
	}

public:
	void SetRawData(const void* data, const unsigned int& size)
	{
		m_raw_data.assign((char*)data, size);
	}
};

/*==================================================================================
* Class HttpUrlEncodedContent
* Provides a container for content encoded using multipart/form-data MIME type
===================================================================================*/
class HttpMultipartContent : public HttpContent
{
	struct HttpContentPart
	{
		std::string			 m_name;
		std::string			 m_value;
		std::string			 m_filename;

		//[field] content-type and parameter data part
		struct
		{
			HttpContentType	 m_content_type;   // specify content-type
			std::string		 m_content_parameter; // parameter after content-type when content-type != Auto
		};

		//[field] set a mime part's custom headers
		std::string			 m_header_custom;

		//[field] data field part ! NO COPY
		struct
		{
			const void*		 m_data;
			size_t			 m_size;
		};
	};

private:
	struct curl_mime*			 m_curl_mine;
	std::vector<HttpContentPart> m_part_list;

private:
	void FreeCurlMine()
	{
		curl_mime_free(m_curl_mine);
		m_curl_mine = NULL;
	}

	void InitCurlMine(IN CURL* curl)
	{
		if (m_curl_mine)
			this->FreeCurlMine();
		m_curl_mine = curl_mime_init(curl);
	}

	BOOL AddPart(const HttpContentPart& part_data)
	{
		if (!m_curl_mine) return FALSE;

		/* Fill in the file upload field */
		curl_mimepart* pPart = curl_mime_addpart(m_curl_mine);

		if (!pPart) return FALSE;

		if (!part_data.m_header_custom.empty())
		{
			// headers auto free -> no need use slist free here
			struct curl_slist* headers = NULL;
			headers = curl_slist_append(headers, part_data.m_header_custom.c_str());

			curl_mime_headers(pPart, headers, TRUE);
		}

		// content-type for part not disposition_type
		if (part_data.m_content_type != HttpContentType::Auto)
		{
			std::string str_ct_type = kyhttp::get_string_content_type(part_data.m_content_type);

			if (!part_data.m_content_parameter.empty())
			{
				str_ct_type.append("; " + part_data.m_content_parameter);
			}
			curl_mime_type(pPart, str_ct_type.c_str());
		}

		if (part_data.m_data)
		{
			curl_mime_name(pPart, part_data.m_name.c_str());
			curl_mime_filename(pPart, part_data.m_filename.c_str());
			curl_mime_data(pPart, (const char*)part_data.m_data, part_data.m_size);
		}
		else
		{
			curl_mime_name(pPart, part_data.m_name.c_str());
			curl_mime_data(pPart, part_data.m_value.c_str(), CURL_ZERO_TERMINATED);
		}

		return TRUE;
	}

public:
	HttpMultipartContent(): m_curl_mine(NULL)
	{

	}

	void AddPartKeyValue(const char* name, const char* value,
						 HttpContentType	content_type      = Auto,
						 const char*		content_parameter = NULL,
						 const char*		header_custom	  = NULL)
	{
		HttpContentPart part;

		part.m_content_type = content_type;
		part.m_content_parameter = content_parameter;
		part.m_name  = name;
		part.m_value = value;
		part.m_data  = NULL;
		part.m_size  = 0;
		part.m_header_custom = header_custom ? header_custom : "";

		m_part_list.push_back(part);
	}

	void AddPartFile(const char* name, const char* filename,
					const void* data = NULL, size_t data_size = 0,	  // file data
					HttpContentType		content_type	  = Auto,
					const char*			content_parameter = NULL,
					const char*			header_custom     = NULL)
	{
		HttpContentPart part;

		part.m_content_type = content_type;
		part.m_content_parameter = content_parameter ? content_parameter : "";
		part.m_name     = name;
		part.m_filename = filename;
		part.m_data     = data;
		part.m_size     = data_size;

		part.m_header_custom = header_custom ? header_custom : "";

		m_part_list.push_back(part);
	}

protected:
	virtual ContentType GetType() const
	{
		return ContentType::multipart;
	}

	virtual void* InitContent(IN void* base)
	{
		CURL* curl = static_cast<CURL*>(base);
		if (!curl) return NULL;

		this->InitCurlMine(curl);

		for (int i = 0; i < m_part_list.size(); i++)
		{
			if (FALSE == this->AddPart(m_part_list[i]))
			{
				std::cout << "[erro] : add part failed" << std::endl;
			}
		}

		return m_curl_mine;
	}
};

class HttpRequest : public std::enable_shared_from_this<HttpRequest>
{
protected: // property header data
	HttpHeaderData		m_header_data;
	HttpContent*		m_content;

private: // curl header data 
	struct curl_slist*  m_curl_slist;
	std::string		    m_buffer;

public:
	~HttpRequest()
	{
		this->CurlHeaderFree();
	}

private:
	void CurlHeaderFree()
	{
		curl_slist_free_all(m_curl_slist);
	}

	// TODO:
	void SetCurlSlist()
	{

	}

private:
	void* CreateHeaderData()
	{
		this->CurlHeaderFree();

		auto append_curl_header = [](curl_slist** curl_slist, const char* format, ...)
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
			*curl_slist = curl_slist_append(*curl_slist, buf);
		};

		if (m_header_data.m_content_type != HttpContentType::Auto)
			append_curl_header(&m_curl_slist, "Content-Type: %s", get_string_content_type(m_header_data.m_content_type).c_str());
		if (!m_header_data.m_host.empty())
			append_curl_header(&m_curl_slist, "Host: %s", m_header_data.m_host.c_str());
		if (!m_header_data.m_accept.empty())
			append_curl_header(&m_curl_slist, "Accept: %s", m_header_data.m_accept.c_str());

		if (!m_header_data.m_accept_encoding.empty())
		{
			append_curl_header(&m_curl_slist, "Accept-encoding: %s", m_header_data.m_accept_encoding.c_str());
		}
		else
		{
			append_curl_header(&m_curl_slist, "Accept-encoding: %s", "gzip, deflate, br");
		}

		for (int i = 0; i < m_header_data.m_extension.size(); i++)
		{
			append_curl_header(&m_curl_slist, m_header_data.m_extension[i].c_str());
		}
		append_curl_header(&m_curl_slist, "User-Agent: kohyoung");
		append_curl_header(&m_curl_slist, "Connection: %s", "Keep-Alive");

		return m_curl_slist;
	}

	void* CreateContentData(IN void* base, IN ContentType& type)
	{
		if (!m_content || !base)
			return NULL;

		CURL* curl = static_cast<CURL*>(base);
		void* content = m_content->InitContent(curl);

		type = m_content->GetType();
		
		return content;
	}

	friend class HttpClient;

public:

	virtual void SetRequestParam(IN const char* req_param)
	{
		m_header_data.m_request_param = req_param;
	}

	virtual void SetContentType(IN HttpContentType content_type)
	{
		m_header_data.m_content_type = content_type;
	}

	virtual void SetAccept(IN const char* accept)
	{
		m_header_data.m_accept = accept;
	}

	virtual void SetAcceptEncoding(IN const char* accept_encoding)
	{
		m_header_data.m_accept_encoding = accept_encoding;
	}

	virtual void SetHost(IN const char* host)
	{
		m_header_data.m_host = host;
	}

	virtual void SetContent(IN HttpContent* content)
	{
		m_content = content;
	}
};


class HttpResponse
{
private:
	LONG			m_status;

	HttpBuffer		m_header;
	HttpBuffer		m_content;

	std::string		m_redirect_url;
protected:

public:
	HttpResponse() : m_status(HttpStatusCode::NODEFINE)
	{
		m_header.reserve(1000);
		m_content.reserve(1000);
	}

protected:
	virtual void Clear()
	{
		m_status = HttpStatusCode::NODEFINE;
		m_header.clear();
		m_content.clear();
		m_redirect_url.clear();
	}

public:
	virtual void SaveToFile(const wchar_t* path_fileout, BOOL bExpStatusCode = FALSE, BOOL bRemoveOld = TRUE)
	{
		if (bRemoveOld)
		{
			kyhttp::write_data_file(path_fileout, NULL, 0);
		}

		if (bExpStatusCode)
		{
			char pre[500];
			memset(pre, 0, 500);

			snprintf(pre, 500, "Response Status: %u \n", m_status);
			kyhttp::write_data_file_append(path_fileout, pre, strlen(pre));
		}

		kyhttp::write_data_file_append(path_fileout, m_content.buffer(), m_content.length());
	}

	virtual HttpStatusCode GetStatusCode() const
	{
		return static_cast<HttpStatusCode>(m_status);
	}

	std::string GetRedirectUrl()
	{
		return m_redirect_url;
	}

	virtual const HttpBuffer* Header() const
	{
		return &m_header;
	}

	virtual const HttpBuffer* Content() const
	{
		return &m_content;
	}

	friend class HttpClient;
};


class HttpClient : public IHttpClient
{
private:
	CURL*				m_curl;
private:
	HttpRequestPtr		m_request;
	HttpResponsePtr		m_response;

	HttpClientOption	m_option;
	HttpCookie			m_cookie;
	SSLSetting			m_ssl_setting;
	WebProxy			m_proxy;
	HttpMethod			m_request_method;

	HttpClientProgress	m_progress;

	double				m_request_time;
	double				m_upload_size;
	double				m_download_size;

	double				m_upload_speed;
	double				m_download_speed;

	int					m_use_openssl = false; // curl build = schannel = false | openssl = true
	int					m_use_custom_ssl = false;

public:
	HttpClient(): m_curl(nullptr),
		m_request(nullptr), m_response(nullptr),
		m_use_openssl(false),
		m_use_custom_ssl(false)
	{

	}
	~HttpClient()
	{
		this->Curl_Destroy();
	}

private:
	static int HttpProgressFunc(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
	{
		if (ptr == NULL)
			return 0;

		HttpClientProgress* progress = static_cast<HttpClientProgress*> (ptr);

		// stop download or upload data network
		if (progress && progress->m_force_stop)
		{
			return 1;
		}

		if (TotalToUpload > 0 && progress)
		{
			progress->m_action = 1;
			progress->m_cur_upload = NowUploaded;
			progress->m_total_upload = TotalToUpload;
		}
		else if (TotalToDownload > 0 && progress)
		{
			progress->m_action = 2;
			progress->m_cur_download = NowDownloaded;
			progress->m_total_download = TotalToDownload;
		}

		return 0;
	}
	static int HttpReceiveResponseFunc(void* contents, size_t size, size_t nmemb, void* user_data)
	{
		if (user_data)
		{
			char* test = static_cast<char*>(contents);
			((HttpBuffer*)user_data)->append((char*)contents, size * nmemb);
		}
		return size * nmemb;
	}

private:
	/******************************************************************************
	*! @brief  : initialize , destroy curl
	*! @author : thuong.nv - [Date] : 03/10/2022
	*! @parameter:	header : header info struct
	*! @return : bool : TRUE / FALSE
	******************************************************************************/
	BOOL Curl_Initialize()
	{
		if (m_curl) 
			this->Curl_Destroy();

		m_curl = curl_easy_init();

		return m_curl ? TRUE :FALSE;
	}

	void Curl_Destroy()
	{
		curl_easy_cleanup(m_curl);
		m_curl = NULL;
	}

	CURLcode Curl_Execute(CURL* curl, const char* url, bool out_log = true)
	{
		auto curl_get_time_second = [](CURL* _curl) // get time request information
		{
			curl_off_t lrequest_time = 0;
			if (curl_easy_getinfo(_curl, CURLINFO_TOTAL_TIME_T, &lrequest_time) == CURLE_OK)
			{
				return double(lrequest_time) / 1000000.0;
			}
			return 0.0;
		};

		curl_easy_setopt(m_curl, CURLOPT_URL, url);

		if (out_log)
		{
			KY_HTTP_LOG("%s > %s, ssl_check=%s, timeout=%u, auto_redirect=%s",
						get_string_method(m_request_method).c_str(), url,
						m_ssl_setting.m_disable_verify_ssl_certificate ? "false" : "true",
						m_option.m_connect_timout,
						m_option.m_auto_redirect ? "true" : "false");
		}

		CURLcode curlret = curl_easy_perform(m_curl);
		m_request_time += curl_get_time_second(m_curl);

		unsigned int iTry = 0; // try connection
		while ((CURLcode::CURLE_OPERATION_TIMEDOUT == curlret ||
				CURLcode::CURLE_COULDNT_CONNECT == curlret) &&
				iTry < m_option.m_retry_connet)
		{
			KY_HTTP_LOG_WARN("Connection time out! %s -> Trying: %u.", url, iTry + 1);
			curlret = curl_easy_perform(m_curl);
			m_request_time += curl_get_time_second(m_curl);

			iTry++;
		}

		return curlret;
	}

	CURLcode Curl_GetRequestInfo(CURL* curl)
	{
		curl_off_t lrequest_size = 0;
		double value_size = 0.0;

		// upload request information
		if (curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &lrequest_size) == CURLE_OK)
		{
			m_upload_size += lrequest_size;
		}
		if (curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD, &value_size) == CURLE_OK)
		{
			m_upload_size += value_size;
		}

		// download request information
		value_size = 0.0;
		if (curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &value_size) == CURLE_OK)
		{
			m_download_size += value_size;
		}

		// send_recv data speed
		curl_easy_getinfo(m_curl, CURLINFO_SPEED_UPLOAD,   &m_upload_speed);
		curl_easy_getinfo(m_curl, CURLINFO_SPEED_DOWNLOAD, &m_download_speed);

		// get status code
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &m_response->m_status);

		return CURLcode::CURLE_OK;
	}

	CURLcode Curl_GetCookie(CURL* curl)
	{
		/* export cookies to this file when closing the handle */
		//curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.txt");

		m_cookie.m_cookie_recv.clear();

		struct curl_slist* cookies = NULL;
		CURLcode res = curl_easy_getinfo(m_curl, CURLINFO_COOKIELIST, &cookies);
		if (!res && cookies) {
			/* a linked list of cookies in cookie file format */
			struct curl_slist* each = cookies;
			while (each)
			{
				m_cookie.m_cookie_recv.append(each->data);
				each = each->next;
			}
			/* we must free these cookies when we are done */
			curl_slist_free_all(cookies);
		}

		return CURLcode::CURLE_OK;
	}

	void Curl_WriteLogRequestInfo(CURLcode curlret)
	{
		if (curlret == CURLcode::CURLE_OK)
		{
			KY_HTTP_LOG("[*] Sent request DONE. total_upload=%s, totaltime=%.5f sec, upload_speed=%s/sec",
				kyhttp::convert_bytes_to_text(m_upload_size).c_str(), m_request_time,
				kyhttp::convert_bytes_to_text(m_upload_speed).c_str());
			KY_HTTP_LOG("[*] Received response DONE. status_code:%u, total_download=%s, download_speed=%s/sec.", m_response->m_status,
				kyhttp::convert_bytes_to_text(m_download_size).c_str(),
				kyhttp::convert_bytes_to_text(m_download_speed).c_str());
		}
		else
		{
			KY_HTTP_LOG("[*] Sent request FAILED. total_upload=%s, totaltime=%.5f sec, upload_speed=%s/sec",
				kyhttp::convert_bytes_to_text(m_upload_size).c_str(),
				m_request_time,
				kyhttp::convert_bytes_to_text(m_download_speed).c_str());

			KY_HTTP_LOG("============================ Error information =============================");
			KY_HTTP_LOG("ErrorCode=<0x%x> | CURLcode=<%u>", ConvertCURLCodeToHTTPCode(curlret), curlret);
			KY_HTTP_LOG("%s", curl_easy_strerror(curlret));
			KY_HTTP_LOG("============================================================================");
		}
	}

	static std::string GetStringErrorCode(HttpErrorCode err)
	{
		switch (err)
		{
		case kyhttp::KY_HTTP_FAILED:
			return "Failed";
			break;
		case kyhttp::KY_HTTP_OK:
			return "OK";
			break;
		case kyhttp::KY_HTTP_COULDNT_RESOLVE_PROXY:
			return "Couldn't resolve proxy name";
			break;
		case kyhttp::KY_HTTP_COULDNT_RESOLVE_HOST:
			return "Coundn't resolve host name ->IP";
			break;
		case kyhttp::KY_HTTP_COULDNT_CONNECT:
			return "Couldn't connect to server";
			break;
		case kyhttp::KY_HTTP_OUT_OF_MEMORY:
			return "Out of memory";
			break;
		case kyhttp::KY_HTTP_SSL_HANDSHAKE_FAIL:
			return "SSL connect error";
			break;
		case kyhttp::KY_HTTP_SERVER_FAILED_VERIFICATION:
			return "SSL peer certificate or SSH remote key was not OK";
			break;
		case kyhttp::KY_HTTP_SEND_ERROR:
			return "Send data error";
			break;
		case kyhttp::KY_HTTP_RECV_ERROR:
			return "Receive data error";
			break;
		case kyhttp::KY_HTTP_SSL_CERTPROBLEM:
			return "Problem with the local SSL certificate";
			break;
		case kyhttp::KY_HTTP_REQUEST_TIMEOUT:
			return "Connetion timeout";
			break;
		case kyhttp::KY_HTTP_CREATEDATA_REQUEST_FAIL:
			return "Create request data failed";
			break;
		case kyhttp::KY_HTTP_INIT_REQUEST_FAIL:
			return "init request failed";
			break;
		default:
			break;
		}
		return "Undefine error";
	}

	static HttpErrorCode ConvertCURLCodeToHTTPCode(CURLcode curl_code)
	{
		HttpErrorCode kyhttp_error_code = KY_HTTP_FAILED;
	
	    switch (curl_code)
	    {
	        case CURLE_OK :
				kyhttp_error_code = KY_HTTP_OK;
	
	            break;
	        case CURLE_COULDNT_RESOLVE_PROXY :
				kyhttp_error_code = KY_HTTP_COULDNT_RESOLVE_PROXY;
	
	            break;
	        case CURLE_COULDNT_RESOLVE_HOST :
				kyhttp_error_code = KY_HTTP_COULDNT_RESOLVE_HOST;
	
	            break;
	        case CURLE_COULDNT_CONNECT :
				kyhttp_error_code = KY_HTTP_COULDNT_CONNECT;
	
	            break;
	        case CURLE_OUT_OF_MEMORY :
				kyhttp_error_code = KY_HTTP_OUT_OF_MEMORY;
	
	            break;
	        case CURLE_SSL_CONNECT_ERROR :
				kyhttp_error_code = KY_HTTP_SSL_HANDSHAKE_FAIL;
	
	            break;
	        case CURLE_PEER_FAILED_VERIFICATION :
				kyhttp_error_code = KY_HTTP_SERVER_FAILED_VERIFICATION;
	
	            break;
	        case CURLE_SEND_ERROR :
				kyhttp_error_code = KY_HTTP_SEND_ERROR;
	
	            break;
	        case CURLE_RECV_ERROR :
				kyhttp_error_code = KY_HTTP_RECV_ERROR;
	
	            break;
	        case CURLE_SSL_CERTPROBLEM :
				kyhttp_error_code = KY_HTTP_SSL_CERTPROBLEM;
	
	            break;
	        case CURLE_OPERATION_TIMEDOUT  :
				kyhttp_error_code = KY_HTTP_REQUEST_TIMEOUT;
	
	            break;
	        default :
				kyhttp_error_code = KY_HTTP_FAILED;
	
	            break;
	    }
	
	    return kyhttp_error_code;
	}

	void SetRequestMethod(HttpMethod method)
	{
		if (method == HttpMethod::POST)
			curl_easy_setopt(m_curl, CURLOPT_POST, 1L);
		else
			curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1L); // Default get

		m_request_method = method;
	}

	HttpErrorCode ResetRequestInformation()
	{
		m_download_size = 0.0;
		m_upload_size   = 0.0;
		m_request_time  = 0.0;

		m_progress.m_cur_download   = 0.0;
		m_progress.m_total_download = 0.0;
		m_progress.m_cur_upload     = 0.0;
		m_progress.m_total_upload   = 0.0;

		return HttpErrorCode::KY_HTTP_OK;
	}

	HttpErrorCode CreateConfig(const HttpClientOption& option)
	{
		CURLcode curlcode = CURLcode::CURLE_OK;

		if (m_option.m_show_request)
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L));

		if (option.m_connect_timout > 0)
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT_MS, option.m_connect_timout));

		// Internal CURL progressmeter must be disabled if we provide our own callback
		PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, FALSE));
		PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, &m_progress));

		// Install the callback function
		PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, &HttpClient::HttpProgressFunc));

		if (option.m_max_upload_speed > 0)
		{
			// limit upload kb.s
			curl_off_t max_speed = 1024L * m_option.m_max_upload_speed; // bytes/s
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_MAX_SEND_SPEED_LARGE, max_speed));
		}
		if (option.m_max_download_speed > 0)
		{
			// limit download kb.s
			curl_off_t max_speed = 1024L * m_option.m_max_download_speed; // bytes/s
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_MAX_RECV_SPEED_LARGE, m_option.m_max_download_speed));
		}

		// auto redirected
		if (TRUE == option.m_auto_redirect)
		{
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 0L));
		}

		/* enable the cookie engine */
		//PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_COOKIEFILE, ""); );
		//curl_easy_setopt(m_curl, CURLOPT_COOKIEFILE, "");
		//curl_easy_setopt(m_curl, CURLOPT_COOKIEJAR, "cookies.txt");

		HttpErrorCode retcode = ConvertCURLCodeToHTTPCode(curlcode);
		return retcode;
	}

	HttpErrorCode InitClearResponse(int bForceCreate = false)
	{
		if (bForceCreate || !m_response)
		{
			m_response = std::make_shared<HttpResponse>();
		}
		else // clear old data
		{
			m_response->Clear();
		}

		//curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, &HttpClient::HttpReceiveResponseFunc);
		//curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &m_response->m_header);
		curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &HttpClient::HttpReceiveResponseFunc);
		curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_response->m_content);

		return HttpErrorCode::KY_HTTP_OK;
	}

	HttpErrorCode CreateSSLOption(const SSLSetting* ssl_setting)
	{
		// Verify manual confirmation using file *.pem
		if (m_use_openssl)
		{
			curl_easy_setopt(m_curl, CURLOPT_PROXY_SSLCERTTYPE, SSL_CERTIFICATE_TYPE_PEM);
			curl_easy_setopt(m_curl, CURLOPT_PROXY_SSL_VERIFYPEER, 1);
			curl_easy_setopt(m_curl, CURLOPT_PROXY_SSL_VERIFYHOST, 1);
			curl_easy_setopt(m_curl, CURLOPT_PROXY_CAINFO, "./cacert.pem");
		}
		// libcurl schanel build : Verify window certificate
		else
		{
			// Verify the server's SSL certificate. - auto
			curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 1L);
			curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);

			if(ssl_setting->m_disable_verify_ssl_certificate)
				curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
			if (ssl_setting->m_disable_verify_host_certificate)
				curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
		}

		return HttpErrorCode::KY_HTTP_OK;
	}

	HttpErrorCode CreateProxyOption(const WebProxy* proxy)
	{
		// Set proxy options if proxy is used.
		if (!m_proxy.m_hostname.empty())
		{
			curl_easy_setopt(m_curl, CURLOPT_PROXY, m_proxy.m_hostname.c_str());
			curl_easy_setopt(m_curl, CURLOPT_PROXYPORT, m_proxy.m_port);
			
			// use libcurl build openssl
			if (m_use_openssl)
			{
				curl_easy_setopt(m_curl, CURLOPT_PROXYTYPE, m_proxy.m_proxy_type);
				curl_easy_setopt(m_curl, CURLOPT_PROXY_SSLCERTTYPE, SSL_CERTIFICATE_TYPE_PEM);
				curl_easy_setopt(m_curl, CURLOPT_PROXY_CAINFO, "./cacert.pem");
			}
			// use libcurl build schannel
			else
			{
				// Schannel not support HTTPs-proxy
				auto proxytype = m_proxy.m_proxy_type == kyhttp::KY_PROXY_HTTPS ?
					kyhttp::KY_PROXY_HTTP : m_proxy.m_proxy_type;
				curl_easy_setopt(m_curl, CURLOPT_PROXYTYPE, proxytype);
			}

		    if (!m_proxy.m_username.empty())
		    {
	            curl_easy_setopt(m_curl, CURLOPT_PROXYUSERNAME, m_proxy.m_username.c_str());
		        curl_easy_setopt(m_curl, CURLOPT_PROXYPASSWORD, m_proxy.m_password.c_str());
		    }
		}
		return HttpErrorCode::KY_HTTP_OK;
	}

	/******************************************************************************
	*! @brief  : curl initialization and related settings
	*! @author : thuong.nv - [Date] : 11/11/2022
	*! @parameter:	method : GET / POST
	*! @return : HttpErrorCode 
	******************************************************************************/
	HttpErrorCode InitHttpRequest()
	{
		HttpErrorCode err_code = HttpErrorCode::KY_HTTP_OK;

		if (!Curl_Initialize())
		{
			KY_HTTP_LOG_ERROR(L"[err] : init_curl failed !");
			return HttpErrorCode::KY_HTTP_FAILED;
		}

		PASS_ERROR_CODE(err_code, this->ResetRequestInformation());
		PASS_ERROR_CODE(err_code, this->CreateConfig(m_option));
		PASS_ERROR_CODE(err_code, this->CreateSSLOption(&m_ssl_setting));
		PASS_ERROR_CODE(err_code, this->CreateProxyOption(&m_proxy));
		PASS_ERROR_CODE(err_code, this->InitClearResponse());

		KY_HTTP_LOG("HttpRequest initialization done <0x%x>", err_code);
		return err_code;
	}

	/******************************************************************************
	*! @brief  : setup header and content data for httprequest
	*! @author : thuong.nv - [Date] : 11/11/2022
	*! @parameter:	method : GET / POST
	*! @parameter:	HttpRequest : Contains header and content information
	*! @return : HttpErrorCode
	******************************************************************************/
	HttpErrorCode CreateRequestData(IN HttpMethod method, IN HttpRequest* request)
	{
		// please set the method before setting the request data
		this->SetRequestMethod(method);

		if (HttpMethod::GET == method && NULL == request)
			return HttpErrorCode::KY_HTTP_OK;

		if (HttpMethod::POST == method && NULL == request)
			return HttpErrorCode::KY_HTTP_FAILED;

		m_request = request->shared_from_this();
		
		CURLcode curlcode = CURLE_OK;

		// create content request data
		ContentType type = ContentType::none;
		void* content_request = request->CreateContentData(m_curl, type);

		// use when post not data content
		if (HttpMethod::POST == method && (content_request == NULL || ContentType::none == type))
		{
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, 0));
		}
		else if (ContentType::multipart == type)
		{
			curl_mime* data_post = static_cast<curl_mime*>(content_request);
			PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_MIMEPOST, data_post));
		}
		else if (ContentType::urlencoded == type)
		{
			HttpBuffer* buff = static_cast<HttpBuffer*>(content_request);
			if (buff)
			{
				PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, buff->buffer()));
				PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, buff->length()));
			}
		}
		else if (ContentType::raw == type)
		{
			HttpBuffer* buff = static_cast<HttpBuffer*>(content_request);
			if (buff)
			{
				PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, buff->buffer()));
				PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, buff->length()));
			}
		}

		// create header request data
		curl_slist* header = static_cast<curl_slist*>(request->CreateHeaderData());
		if (header && ContentType::raw == type)
		{
			HttpRawContent* rawHttp = static_cast<HttpRawContent*>(request->m_content);
			if (rawHttp)
				curl_slist_append(header, rawHttp->GetRawTypeToString());
		}
		curl_easy_setopt(m_curl, CURLOPT_TCP_KEEPALIVE, 1L);

		PASS_CURL_EXEC(curlcode, curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, header));

		HttpErrorCode retcode = ConvertCURLCodeToHTTPCode(curlcode);
		KY_HTTP_LOG(L"Create request data done <0x%x>", retcode);

		return retcode;
	}

private:
	static std::string get_string_method(IN HttpMethod method)
	{
		switch (method)
		{
		case kyhttp::POST:
			return "POST";
			break;
		case kyhttp::GET:
			return "GET";
			break;
		default:
			break;
		}
		return "";
	}
private:
	HttpErrorCode SendRequest(IN const Uri& uri, BOOL redirect = FALSE)
	{
		if (!m_curl)
			return HttpErrorCode::KY_HTTP_FAILED;

		std::string url = uri.get_url();

		if (redirect)
			KY_HTTP_LOG("[*] Redirect to : %s", url.c_str());

		CURLcode curlret = this->Curl_Execute(m_curl, url.c_str(), redirect? false:true);
		PASS_CURL_EXEC(curlret, this->Curl_GetRequestInfo(m_curl));

		if (m_response->m_status == HttpStatusCode::MOVED_PERMANENTLY)
		{
			this->InitClearResponse();
			
			char* redirect_url = NULL;
			curl_easy_getinfo(m_curl, CURLINFO_REDIRECT_URL, &redirect_url);

			if (redirect_url) 
				m_response->m_redirect_url = redirect_url;

			if (m_option.m_auto_redirect)
			{
				Uri redirect_uri = uri;
				redirect_uri.location = m_response->GetRedirectUrl();
				return SendRequest(redirect_uri, TRUE);
			}
		}
		this->Curl_GetCookie(m_curl);
		this->Curl_WriteLogRequestInfo(curlret);

		HttpErrorCode retcode = ConvertCURLCodeToHTTPCode(curlret);

		return retcode;
	}
public:
	virtual void Configunation(IN HttpClientOption& option)
	{
		m_option = option;
	}

	virtual void SettingProxy(IN WebProxy& proxy_info)
	{
		m_proxy = proxy_info;
	}

	virtual void SettingSSL(IN SSLSetting& ssl_setting)
	{
		m_ssl_setting = ssl_setting;
	}

	virtual void SettingCookie(IN HttpCookie& cookie)
	{
		m_cookie = cookie;
	}

public:
	virtual HttpErrorCode Request(IN HttpMethod method, IN const Uri uri, IN HttpRequest* request)
	{
		HttpErrorCode request_code = HttpErrorCode::KY_HTTP_FAILED;

		switch (method)
		{
		case kyhttp::POST:
			request_code = this->Post(uri, request);
			break;
		case kyhttp::GET:
			request_code = this->Get(uri, request);
			break;
		default:
			break;
		}

		return request_code;
	}

	virtual HttpErrorCode Post(IN const Uri uri, IN HttpRequest* request)
	{
		KY_HTTP_LOG("/////////////////////////////////////////////////////////////////////");
		if (nullptr == request)
		{
			KY_HTTP_LOG_ERROR("Post request nulls is not allowed !");
			return HttpErrorCode::KY_HTTP_FAILED;
		}

		HttpErrorCode retcode = HttpErrorCode::KY_HTTP_OK;

		if (!CHECK_HTTP_ERROR_OK(retcode, this->InitHttpRequest()))
		{
			KY_HTTP_LOG_ERROR("Init request failed. %s", GetStringErrorCode(retcode).c_str());
			return HttpErrorCode::KY_HTTP_INIT_REQUEST_FAIL;
		}

		if (!CHECK_HTTP_ERROR_OK(retcode, this->CreateRequestData(HttpMethod::POST, request)))
		{
			KY_HTTP_LOG_ERROR("Created data request failed. %s", GetStringErrorCode(retcode).c_str());
			return HttpErrorCode::KY_HTTP_CREATEDATA_REQUEST_FAIL;
		}

		return SendRequest(uri);
	}

	virtual HttpErrorCode Get(IN const Uri uri, IN HttpRequest* request)
	{
		KY_HTTP_LOG("/////////////////////////////////////////////////////////////////////");
		HttpErrorCode retcode = HttpErrorCode::KY_HTTP_OK;

		if (!CHECK_HTTP_ERROR_OK(retcode, this->InitHttpRequest()))
		{
			KY_HTTP_LOG_ERROR("Init request failed. %s", GetStringErrorCode(retcode).c_str());
			return HttpErrorCode::KY_HTTP_INIT_REQUEST_FAIL;
		}

		if (!CHECK_HTTP_ERROR_OK(retcode, this->CreateRequestData(HttpMethod::GET, request)))
		{
			KY_HTTP_LOG_ERROR("Created data request failed. %s", GetStringErrorCode(retcode).c_str());
			return HttpErrorCode::KY_HTTP_CREATEDATA_REQUEST_FAIL;
		}

		return SendRequest(uri);
	}

	virtual HttpResponsePtr Response() const
	{
		return m_response;
	}

	friend class HttpReponse;
};


__END___NAMESPACE__