#include "kyhttp_curl.h"


//test website use : https://badssl.com/

int main2()
{
	kyhttp::Uri uri;
	uri.set_location("https://webhook.site/304543bb-1f4e-4406-bd0c-3350941545f8");
	//uri.set_location("https://expired.badssl.com/"); // ssl expired certificate
	//uri.set_location("https://wrong.host.badssl.com/"); // wrong expired certificate
	//uri.set_location("www.google.com:81"); // request timeout
	//uri.set_location("http://10.10.1.153:8080/secure/Dashboard.jspa");
	//uri.set_location("https://www.google.com.sg/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png");
	//uri.set_location("https://www.youtube.com");

	uri.add_query_param("dfsad", 25.5f);

	kyhttp::HttpClientOption option;
	//option.m_show_request = TRUE;
	//option.m_auto_redirect = TRUE;
	//option.m_connect_timout = 1000; //ms
	//option.m_retry_connet = 3;

	kyhttp::WebProxy proxy;
	//proxy.m_hostname = "85.208.107.192";
	//proxy.m_port = 1337;

	kyhttp::HttpCookie cookie;

	kyhttp::SSLSetting sslsetting;
	//sslsetting.m_disable_verify_ssl_certificate = true;
	//sslsetting.m_disable_verify_host_certificate = true;

	kyhttp::HttpClient* client = new kyhttp::HttpClient();
	client->Configunation(option);
	client->SettingProxy(proxy);
	client->SettingSSL(sslsetting);
	client->SettingCookie(cookie);

	kyhttp::HttpRequest* request = new kyhttp::HttpRequest();

	//========== 1. POST test ============
	//std::string data = "3242352352352623";
	//kyhttp::HttpMultipartContent* content = new kyhttp::HttpMultipartContent();
	//content->AddPartKeyValue("key1", "123445");
	//content->AddPartKeyValue("key2", "123445");
	//content->AddPartFile("key3", "simpson", data.c_str(), data.length());

	////kyhttp::HttpUrlEncodedContent* content = new kyhttp::HttpUrlEncodedContent();
	////content->AddKeyValue("12", 1000);
	////content->AddKeyValue("thuong", "ngo vanw thuong");

	//request->SetContent(content);

	//kyhttp::HttpErrorCode err = client->Post(uri, request);

	std::string data = "{ \"ten\" : \"ngo van thuong\"}";

	kyhttp::HttpRawContent* content = new kyhttp::HttpRawContent();
	content->SetRawType(kyhttp::HttpRawContent::html);
	content->SetRawData(data.c_str(), data.length());

	////========== 2. GET test ============
	//client->SettingProxy();
	//client->SettingCookie();


	request->SetContent(content);

	kyhttp::HttpErrorCode err = client->Request(kyhttp::POST, uri, request);

	auto a = client->Response();
	kyhttp::HttpStatusCode code = a->GetStatusCode();

	delete client;
	delete request;


	//std::string data = "122324324532543655";

	//HttpBuffer* buff = new HttpBuffer();

	////buff->Reserve(100);
	//buff->append(data.c_str(), data.length());

	//std::wstring txt = buff->buffer();


	//std::cout << buff->buffer() << std::endl;


	getchar();

	return 1;
}

int main()
{
	//CURL* curl;
	//curl = curl_easy_init();
	//curl_mime* multipart = curl_mime_init(curl);

	//curl_mimepart* part = curl_mime_addpart(multipart);

	////struct curl_slist* headers = NULL;
	////headers = curl_slist_append(headers, "AgentId=\"492F183D-404E-4088-B49C-0A183F5ADA4E\";");
	////curl_mime_headers(part, headers, FALSE);

	///* use these headers, please take ownership */

	////curl_mime_type(part, "application/octet-stream;");
	//


	///* pass on this data */


	//curl_mime_data(part, "12345679", CURL_ZERO_TERMINATED);


	///* set name */

	//curl_mime_name(part, "numbers");

	///* Set the form info */
	//curl_easy_setopt(curl, CURLOPT_URL, "http://webhook.site/304543bb-1f4e-4406-bd0c-3350941545f8");
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	//curl_easy_setopt(curl, CURLOPT_POST, 1L);
	//curl_easy_setopt(curl, CURLOPT_MIMEPOST, multipart);

	//auto ret = curl_easy_perform(curl); /* post away! */

	// /* Clean-up. */
	//curl_easy_cleanup(curl);
	//curl_mime_free(multipart);

		  //      case CURLE_COULDNT_RESOLVE_PROXY:
				//	kyhttp_error_code = KY_HTTP_COULDNT_RESOLVE_PROXY;

				//	break;
				//case CURLE_COULDNT_RESOLVE_HOST:
				//	kyhttp_error_code = KY_HTTP_COULDNT_RESOLVE_HOST;

				//	break;
				//case CURLE_COULDNT_CONNECT:
				//	kyhttp_error_code = KY_HTTP_COULDNT_CONNECT;

				//	break;
				//case CURLE_OUT_OF_MEMORY:
				//	kyhttp_error_code = KY_HTTP_OUT_OF_MEMORY;

				//	break;
				//case CURLE_SSL_CONNECT_ERROR:
				//	kyhttp_error_code = KY_HTTP_SSL_HANDSHAKE_FAIL;

				//	break;
				//case CURLE_PEER_FAILED_VERIFICATION:
				//	kyhttp_error_code = KY_HTTP_SERVER_FAILED_VERIFICATION;

				//	break;
				//case CURLE_SEND_ERROR:
				//	kyhttp_error_code = KY_HTTP_SEND_ERROR;

				//	break;
				//case CURLE_RECV_ERROR:
				//	kyhttp_error_code = KY_HTTP_RECV_ERROR;

				//	break;
				//case CURLE_SSL_CERTPROBLEM:
				//	kyhttp_error_code = KY_HTTP_SSL_CERTPROBLEM;

				//	break;
				//case CURLE_OPERATION_TIMEDOUT:
				//	kyhttp_error_code = KY_HTTP_REQUEST_TIMEOUT;

				//	break;
				//default:
				//	kyhttp_error_code = KY_HTTP_FAILED;
	std::string a = curl_easy_strerror(CURLE_COULDNT_RESOLVE_HOST);
	std::string a1 = curl_easy_strerror(CURLE_COULDNT_CONNECT);
	std::string a2 = curl_easy_strerror(CURLE_OUT_OF_MEMORY);
	std::string a3 = curl_easy_strerror(CURLE_SSL_CERTPROBLEM);
	std::string a4 = curl_easy_strerror(CURLE_OPERATION_TIMEDOUT);
	getchar();
	return 1;
}