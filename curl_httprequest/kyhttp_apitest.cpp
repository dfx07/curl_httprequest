#include "kyhttp_curl.h"


#define FOLDER_API_REQUEST_DATA   L"ksmart_api/request/"
#define FOLDER_API_RESPONSE_DATA  L"ksmart_api/response/"

void post_urlencoded_content_usefile(IN		const kyhttp::Uri&		uri,
									 IN		kyhttp::HttpClientPtr	client,
									 IN		kyhttp::HttpRequestPtr	request,
									 IN		const wchar_t*			send_data_file,
									 IN		const wchar_t*			rev_data_file)
{
	// read post data file
	void* data;
	std::wstring path_send_data(FOLDER_API_REQUEST_DATA);
	path_send_data.append(send_data_file);

	kyhttp::read_data_file(path_send_data.c_str(), &data);
	char*file_data  = static_cast<char*>(data);

	auto content = std::make_shared<kyhttp::HttpUrlEncodedContent>();
	content->AddKeyValue("kp", file_data);
	delete[] data;

	// save received data to file
	request->SetContent(content.get());

	kyhttp::HttpErrorCode err = client->Request(kyhttp::POST, uri, request.get());
	auto response = client->Response();

	std::wstring path_response_data(FOLDER_API_RESPONSE_DATA);
	path_response_data.append(rev_data_file);
	response->SaveToFile(path_response_data.c_str(), TRUE);
}

void get_file_to(IN		const kyhttp::Uri&		uri,
				 IN		kyhttp::HttpClientPtr	client,
				 IN		const wchar_t*			rev_data_file)
{
	kyhttp::HttpErrorCode err = client->Request(kyhttp::GET, uri, nullptr);
	auto response = client->Response();

	std::wstring path_response_data(FOLDER_API_RESPONSE_DATA);
	path_response_data.append(rev_data_file);
	response->SaveToFile(path_response_data.c_str(), FALSE);
}

void upload_file_to(IN		const kyhttp::Uri& uri,
				   IN		kyhttp::HttpClientPtr	client,
				   IN		const wchar_t* path_file_upload,
				   IN		const wchar_t* path_file_response)
{
	kyhttp::HttpRequestPtr request = std::make_shared<kyhttp::HttpRequest>();

	void* data;
	std::wstring path_send_data(FOLDER_API_REQUEST_DATA);
	path_send_data.append(path_file_upload);
	int file_size = kyhttp::read_data_file(path_send_data.c_str(), &data);
	char* file_data = static_cast<char*>(data);
	std::string content_param = "AgentId=\"492F183D-404E-4088-B49C-0A183F5ADA4E\"; AuthToken=\"12923\"; UserId=\"e\"; JobId=\"6226119\"; GroupSeq=\"0\"";

	kyhttp::HttpMultipartContentPtr content = std::make_shared<kyhttp::HttpMultipartContent>();
	
	content->AddPartFile("file", "job_kyhttp_test.kyjob", file_data, file_size,
						kyhttp::HttpContentType::application_octet_stream, content_param.c_str());

	//content->AddPartFile("file", "job_kyhttp_test.kyjob", file_data, file_size,
	//	kyhttp::Auto, NULL, content_param.c_str());

	request->SetContent(content.get());

	kyhttp::HttpErrorCode err = client->Request(kyhttp::POST, uri, request.get());
	auto response = client->Response();

	std::wstring path_response_data(FOLDER_API_RESPONSE_DATA);
	path_response_data.append(path_file_response);
	response->SaveToFile(path_response_data.c_str(), TRUE);
}

void Get_Test()
{
	kyhttp::Uri uri;
	uri.set_location("http://www.youtube.com/");

	kyhttp::HttpClientOption option;

	option.m_auto_redirect = TRUE;

	// use setup parameter
	kyhttp::HttpClientPtr  client = std::make_shared<kyhttp::HttpClient>();
	client->Configunation(option);

	kyhttp::HttpErrorCode err = client->Request(kyhttp::GET, uri, nullptr);
	auto response = client->Response();
}

void KSMARTLogin()
{
	kyhttp::Uri uri;
	uri.set_location("http://192.168.111.247:80/common");

	// use setup parameter
	kyhttp::HttpClientPtr  client  = std::make_shared<kyhttp::HttpClient>();
	kyhttp::HttpRequestPtr request = std::make_shared<kyhttp::HttpRequest>();
	
	post_urlencoded_content_usefile(uri, client, request, L"login.json", L"login_response.txt");

}

void UserAuthPlcy()
{
	kyhttp::Uri uri;
	uri.set_location("http://192.168.111.247:80/policy");

	// use setup parameter
	kyhttp::HttpClientPtr  client = std::make_shared<kyhttp::HttpClient>();
	kyhttp::HttpRequestPtr request = std::make_shared<kyhttp::HttpRequest>();

	post_urlencoded_content_usefile(uri, client, request, L"user_auth_plcy.json", L"user_auth_plcy_response.txt");
}


void KSMARTConfig()
{
	kyhttp::Uri uri;
	uri.set_location("http://192.168.111.247:80/policy");

	// use setup parameter
	kyhttp::HttpClientPtr  client = std::make_shared<kyhttp::HttpClient>();
	kyhttp::HttpRequestPtr request = std::make_shared<kyhttp::HttpRequest>();

	post_urlencoded_content_usefile(uri, client, request, L"ksmart_config.json", L"ksmart_config_response.txt");
}

void lib_part_allpart()
{
	kyhttp::Uri uri;
	uri.set_location("http://192.168.111.247:80/lib_part");

	kyhttp::HttpClientPtr  client = std::make_shared<kyhttp::HttpClient>();
	kyhttp::HttpRequestPtr request = std::make_shared<kyhttp::HttpRequest>();

	post_urlencoded_content_usefile(uri, client, request, L"lib_part_allpart.json", L"lib_part_allpart_response.txt");
}


void lib_pkg_allpkg()
{
	kyhttp::Uri uri;
	uri.set_location("http://192.168.111.247:80/lib_pkg");

	kyhttp::HttpClientPtr  client = std::make_shared<kyhttp::HttpClient>();
	kyhttp::HttpRequestPtr request = std::make_shared<kyhttp::HttpRequest>();

	post_urlencoded_content_usefile(uri, client, request, L"lib_pkg_allpkg.json", L"lib_pkg_allpkg_response.txt");
}


void job_act_list()
{
	kyhttp::Uri uri;
	uri.set_location("http://192.168.111.247:80/job");

	kyhttp::HttpClientPtr  client = std::make_shared<kyhttp::HttpClient>();
	kyhttp::HttpRequestPtr request = std::make_shared<kyhttp::HttpRequest>();

	post_urlencoded_content_usefile(uri, client, request, L"job_act_list.json", L"job_act_list_response.txt");
}


void load_condi()
{
	kyhttp::Uri uri;
	uri.set_location("http://192.168.111.247:80/common");

	kyhttp::HttpClientPtr  client = std::make_shared<kyhttp::HttpClient>();
	kyhttp::HttpRequestPtr request = std::make_shared<kyhttp::HttpRequest>();

	post_urlencoded_content_usefile(uri, client, request, L"load_condi.json", L"load_condi_response.txt");
}

void download_file()
{
	kyhttp::Uri uri;
	uri.set_location("http://192.168.111.247:80/data/_ASSETDATA_/E4E76581-E539-444F-8846-F3F76577F078/job/HUMAX_2ARRAY_LONGRUN/WholeBoard.bmp");

	kyhttp::HttpClientPtr  client = std::make_shared<kyhttp::HttpClient>();
	get_file_to(uri, client, L"download_file_response.bmp");
}

//Result : {"Result":{"Version":"1.0", "ResultCode" : "3001", "Message" : ""}}
void upload_file_multipart()
{
	kyhttp::Uri uri;
	uri.set_location("http://192.168.111.247:80/upload_job");
	//uri.set_location("http://webhook.site/304543bb-1f4e-4406-bd0c-3350941545f8");

	kyhttp::HttpClientPtr  client = std::make_shared<kyhttp::HttpClient>();

	kyhttp::HttpClientOption option;

	option.m_retry_connet = 3;
	client->Configunation(option);
	upload_file_to(uri, client, L"job_upload_test.kyjob", L"upload_file_multipart_response.txt");
	//upload_file_to(uri, client, L"job_kyhttp_test.kyjob", L"upload_file_multipart_response.txt");
}



int main()
{
	//0. get test
	//Get_Test();

	//1. login ksmart
	//KSMARTLogin();

	//2. UserAuthPlcy
	//UserAuthPlcy();

	//3. KSMARTConfig
	//KSMARTConfig();

	//4. lib_part_allpart
	//lib_part_allpart();

	//5. lib_pkg_allpkg
	//lib_pkg_allpkg();


	//6. job_act_list
	//job_act_list();


	//7. load_condi
	//load_condi();

	//8. download_file
	//download_file();

	//9. upload file
	upload_file_multipart();
	getchar();

	return 1;
}