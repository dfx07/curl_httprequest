#include "kyhttp_curl.h"



int main()
{
	kyhttp::RequestUri uri;
	//uri.uri = "https://webhook.site/304543bb-1f4e-4406-bd0c-3350941545f8";
	uri.uri = "https://www.google.com.sg/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png";
	//uri.uri = "http://192.168.111.247:8080/auth/login";

	kyhttp::HttpClient* client = new kyhttp::CurlHttpClient();

	std::string data = "3242352352352623";

	kyhttp::CurlHttpRequest* request = new kyhttp::CurlHttpRequest();

	//1. post example test
	//kyhttp::CurlHttpMultipartContent* content = new kyhttp::CurlHttpMultipartContent();
	//content->AddPartKeyValue("key1", "123445");
	//content->AddPartKeyValue("key2", "123445");
	//content->AddPartFile("key3", "simpson", data.c_str(), data.length());

	//kyhttp::CurlHttpUrlEncodedContent* content = new kyhttp::CurlHttpUrlEncodedContent();
	//content->AddKeyValue("12", 1000);
	//content->AddKeyValue("thuong", "ngo vanw thuong");

	//2. get example test

	client->Get(uri, NULL);


	//request->SetAccept("xyz");

	//request->SetContent(content);

	//client->Post(uri, request);

	//delete content;
	delete client;
	delete request;

	getchar();
}