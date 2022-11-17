#include "kyhttp_curl.h"


int main()
{
	kyhttp::Uri uri;
	uri.set_location("https://webhook.site/1646c30b-303c-4422-8b17-e7b7065a0cee");
	//uri.set_location("http://10.10.1.153:8080/secure/Dashboard.jspa");
	//uri.set_location("https://www.google.com.sg/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png");
	//uri.set_location("https://www.youtube.com");

	uri.add_query_param("dfsad", 25.5f);

	kyhttp::HttpClientOption option;
	//option.m_auto_redirect = TRUE;

	kyhttp::WebProxy proxy;

	//proxy.m_hostname = "85.208.107.192";
	//proxy.m_port = 1337;

	kyhttp::HttpCookie cookie;

	kyhttp::HttpClient* client = new kyhttp::HttpClient();
	client->SetConfigunation(option);
	client->SettingProxy(proxy);
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

	////========== 2. GET test ============
	//client->SettingProxy();
	//client->SettingCookie();


	//request->SetContent(content);

	kyhttp::HttpErrorCode err = client->Request(kyhttp::GET, uri, request);

	auto a = client->Response();
	kyhttp::HTTPCode code = a->GetStatusCode();

	delete client;
	delete request;


	//std::string data = "122324324532543655";

	//HttpBuffer* buff = new HttpBuffer();

	////buff->Reserve(100);
	//buff->append(data.c_str(), data.length());

	//std::wstring txt = buff->buffer();


	//std::cout << buff->buffer() << std::endl;


	getchar();
}