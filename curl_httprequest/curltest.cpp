#include "kyhttp_curl.h"
#include "kyhttp_buffer.h"



int main()
{
	//kyhttp::RequestUri uri;
	////uri.uri = "https://webhook.site/304543bb-1f4e-4406-bd0c-3350941545f8";
	//uri.uri = "https://www.google.com.sg/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png";
	////uri.uri = "http://192.168.111.247:8080/auth/login";

	//kyhttp::HttpClientOption option;
	//kyhttp::WebProxy proxy;
	//kyhttp::HttpCookie cookie;


	//kyhttp::HttpClient* client = new kyhttp::HttpClient();
	//client->SetConfigunation(option);
	//client->SettingProxy(proxy);
	//client->SettingCookie(cookie);

	//kyhttp::HttpRequest* request = new kyhttp::HttpRequest();

	//std::string data = "3242352352352623";

	////1. post example test
	////kyhttp::CurlHttpMultipartContent* content = new kyhttp::CurlHttpMultipartContent();
	////content->AddPartKeyValue("key1", "123445");
	////content->AddPartKeyValue("key2", "123445");
	////content->AddPartFile("key3", "simpson", data.c_str(), data.length());

	////kyhttp::CurlHttpUrlEncodedContent* content = new kyhttp::CurlHttpUrlEncodedContent();
	////content->AddKeyValue("12", 1000);
	////content->AddKeyValue("thuong", "ngo vanw thuong");

	////2. get example test

	////client->SettingProxy();
	////client->SettingCookie();

	//kyhttp::HTTPStatusCode err = client->Post(uri, request);
	//auto a = client->Response();

	//kyhttp::HTTPCode code = a->GetStatusCode();

	////request->SetAccept("xyz");

	////request->SetContent(content);

	////client->Post(uri, request);

	////delete content;
	//delete client;
	//delete request;


	//std::string data = "122324324532543655";

	//HttpBuffer* buff = new HttpBuffer();

	////buff->Reserve(100);
	//buff->append(data.c_str(), data.length());

	//std::wstring txt = buff->buffer();


	//std::cout << buff->buffer() << std::endl;

	//KY_HTTP_ENTRY(L"%s" , L"thuowngf day");

	//KY_HTTP_TRACE_ERROR(L"fadfsdf");
	//KY_HTTP_TRACE_WARNING(L"fadfsdf");
	//KY_HTTP_TRACE_ASSERT(L"fadfsdf");

	//KY_HTTP_LEAVE(L"");

	KYHTTP_LOG(L"thong tin khong dung = %d", 1234);

	//KY_HTTP_LOG_INFO(L"GET - %s", L"thong tin chung");

	getchar();
}