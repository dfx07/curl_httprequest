#include <iostream>
#include <string>
#include <cstdarg>
#include "utils.h"
#include "httprequest.h"
#include <curl/curl.h>


#define CURL_STATICLIB
#pragma comment (lib, "libcurl_debug.lib")

#pragma comment (lib, "Normaliz.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "advapi32.lib")


/* Auxiliary function that waits on the socket. */
static int wait_on_socket(curl_socket_t sockfd, int for_recv, long timeout_ms)
{
    struct timeval tv;
    fd_set infd, outfd, errfd;
    int res;

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    FD_ZERO(&infd);
    FD_ZERO(&outfd);
    FD_ZERO(&errfd);

    FD_SET(sockfd, &errfd); /* always check for error */

    if (for_recv) {
        FD_SET(sockfd, &infd);
    }
    else {
        FD_SET(sockfd, &outfd);
    }

    /* select() returns the number of signalled sockets or -1 */
    res = select((int)sockfd + 1, &infd, &outfd, &errfd, &tv);
    return res;
}


void curl_easy_setno_block(CURL* curl, bool nonblock)
{
    u_long;

}

typedef int (*FN_CALLBACK)(int code, LPVOID lParam);

class Socket
{
public:
    Socket()
    {
        m_curl = curl_easy_init();
        _bforce_stop = false;
    }

    ~Socket()
    {
        free();
    }
private:
    void free()
    {
        curl_easy_cleanup(m_curl);
    }

    // block : waiting for data
    // non block : missing  -> realtime
    void set_block(bool block)
    {
        if (m_curl && _sock)
        {
            u_long iMode = block ? 0 : 1;
            ioctlsocket(_sock, FIONBIO, &iMode);
            _block = block;
        }
    }

    // return -1 : sock error, 0 timeout , 1 :ok
    int wait(const int brecv, long timeout_ms)
    {
        struct timeval tv;
        fd_set infd, outfd, errfd;

        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        FD_ZERO(&infd);
        FD_ZERO(&outfd);
        FD_ZERO(&errfd);

        FD_SET(_sock, &errfd); /* always check for error */

        if (brecv) {
            FD_SET(_sock, &infd);
        }
        else {
            FD_SET(_sock, &outfd);
        }

        /* select() returns the number of signalled sockets or -1 */
        int res = ::select((int)_sock + 1, &infd, &outfd, &errfd, &tv);
        if (res > 0)
            return 1;
        else
            return res;
    }

    //@ return : 1: ok | 0 :timeout | -1: sockerror | -2: user force stop
    int select(const int brecv, long timeout_ms, FN_CALLBACK pfuncallback = NULL)
    {
        int iresult = 0;
        int iunit_incr_time = 200;

        int ielapse_time_total = 0;
        int ielapse_time_callback = 300;
        int icallback_time = 300;

        while (ielapse_time_total <= timeout_ms)
        {
            if (_bforce_stop)
            {
                iresult = -2;
                break;
            }

            iresult = this->wait(brecv, iunit_incr_time);
            if (iresult == 1) // is ok
                break;

            ielapse_time_callback += iunit_incr_time;
            ielapse_time_total += iunit_incr_time;

            if (icallback_time > 0 && ielapse_time_callback >= icallback_time)
            {
                if (pfuncallback)
                    pfuncallback(brecv, NULL);
                ielapse_time_callback = 0;
            }
        }
        return iresult;
    }


    virtual int _connect(std::string url, std::string host, int port, FN_CALLBACK callback_connectok = NULL)
    {
        curl_easy_setopt(m_curl, CURLOPT_CONNECT_ONLY, 1L);
        if (!url.empty())  // use url
        {
            curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
        }
        else // use host and port
        {
            curl_easy_setopt(m_curl, CURLOPT_URL, host.c_str());
            curl_easy_setopt(m_curl, CURLOPT_PORT, (long)port);

            curl_easy_setopt(m_curl, CURLOPT_MAX_RECV_SPEED_LARGE, 2000);
        }

        if (m_connect_timeout > 0)
            curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT_MS, m_connect_timeout);

        // try connect n time
        int nconnect = m_retry_connect <= 0 ? 1 : m_retry_connect;
        CURLcode res;
        for (int itry = 0; itry < nconnect; itry++)
        {
            res = curl_easy_perform(m_curl);

            if (res != CURLE_OK)
                printf("Error: %s\n", curl_easy_strerror(res));

            // get base socket
            if (res == CURLE_OK)
                res = curl_easy_getinfo(m_curl, CURLINFO_ACTIVESOCKET, &_sock);

            if (res == CURLE_OK)
                break;

            Sleep(1000);
            printf("try connect : %i\n", itry);
        }

        // is connect ok
        if (res == CURLE_OK)
        {

            set_block(true);

            if (callback_connectok)
                callback_connectok(res, NULL);
            return 1;
        }
        return 0;
    }
public:
    // @return 0 : ok | 1 : false
    virtual int connect(std::string host, int port, FN_CALLBACK callback_connectok)
    {
        return _connect(std::string(), host, port, callback_connectok);
    }
    virtual int connect(std::string url, FN_CALLBACK callback_connectok)
    {
        return _connect(url, "", -1, callback_connectok);
    }
    virtual int send(const char* pdata, const int& ndatasize, DWORD timeout, FN_CALLBACK pfnCallback = NULL)
    {
        if (!m_curl || !_sock)
            return 0;

        CURLcode res;
        size_t nsent_total = 0;
        do {
            /* Warning: This example program may loop indefinitely.
             * A production-quality program must define a timeout and exit this loop
             * as soon as the timeout has expired. */
            size_t nsent;
            bool bretry_once = true;
            do {
                nsent = 0;
                res = curl_easy_send(m_curl, pdata + nsent_total,
                    ndatasize - nsent_total, &nsent);
                nsent_total += nsent;

                if (res == CURLE_AGAIN && this->select(0, timeout, pfnCallback) < 0
                    /*!wait_on_socket(_sock, 0, timeout)*/)
                {
                    nsent_total -= nsent;

                    // the first time send failed so try once
                    if (bretry_once)
                    {
                        bretry_once = false;
                        continue;
                    }
                    break;
                }
            } while (res == CURLE_AGAIN);

            if (res != CURLE_OK) {
                printf("Error: %s\n", curl_easy_strerror(res));
                break;
            }

            printf("Sent %lu bytes.\n", (unsigned long)nsent);
        } while (nsent_total < ndatasize);

        return nsent_total;
    }

    virtual int recv(DWORD timeout, FN_CALLBACK pfnCallback = NULL)
    {
        if (!m_curl || !_sock)
            return 0;

        std::string data;
        char buf[512];

        for (;;) {
            /* Warning: This example program may loop indefinitely (see above). */
            CURLcode res;
            size_t nread = 0;
            do {
                nread = 0;
                res = curl_easy_recv(m_curl, buf, sizeof(buf), &nread);

                //if (_block && nread <= 0)
                //    break;

                // if it fails because the packet hasn't arrived yet and it's in the queue
                if ((res == CURLE_AGAIN || res == CURLE_SEND_ERROR))
                {
                    if (this->select(1, timeout, pfnCallback) > 0)
                        continue;
                }
                break;
            } while (true);

            if (nread > 0)
            {
                data.append(buf, nread);
                printf("Received %lu bytes.\n", (unsigned long)nread);
            }
            //exit if no bytes are received
            if (nread == 0)
                break;
            if (res != CURLE_OK) {
                printf("Error: %s\n", curl_easy_strerror(res));
                break;
            }
        }

        return data.length();
    }

private:
    void reset_buffer()
    {
        _buf_header.clear();
        _buf_body.clear();
    }
    static size_t write_header_callback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    static size_t write_body_callback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

private:

    void header_format_append(std::string& str, const char* format, ...)
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
        str.append(buf);
    }

    //void header_format_append(std::string& str, const char* strap)
    //{
    //    // append buff to output
    //    str.append(strap);
    //}

    std::string header_format_appendex(const char* format, ...)
    {
        const int nbufsize = 1024;
        char buf[nbufsize];
        memset(buf, 0, nbufsize);

        // list paramater write file follow format
        va_list args;
        va_start(args, format);
        vsnprintf(buf, nbufsize, format, args);
        va_end(args);

        return std::string(buf);
    }
private:

    /* =============================================================================================
    * Example : POST /upload.php HTTP/1.1
    * User-Agent: PostmanRuntime/7.29.2
    * Accept: /*
    * Postman-Token: 8b1b5444-788b-43d7-ac33-1f209b343e5c
    * Host: 127.0.0.1:8080
    * Accept-Encoding: gzip, deflate, br
    * Connection: keep-alive
    * Content-Type: multipart/form-data; boundary=--------------------------918649584285083799389692
    * Content-Length: 13515
    *=============================================================================================*/
    virtual std::string pack_header_multipart(const std::string& host_name,      //
        const std::string& request_param,  //
        const std::string& content_length)  //
    {
        std::string header;
        //header_format_append(header, "POST %s HTTP/1.1\r\n", request_param.c_str());
        header_format_append(header, "Host: %s\r\n", host_name.c_str());
        header_format_append(header, "Accept: */*\r\n");
        //header_format_append(header, "Content-Type: multipart/form-data;");
        header_format_append(header, "Pragma: no-cache\r\n");
        header_format_append(header, "Cache-Control: no-cache\r\n");
        header_format_append(header, "Accept-Encoding: gzip, deflate, br");
        //header_format_append(header, "\r\n");


        //header_format_append("POST %s%s HTTP/1.1", (m_request_param[0] != '/' ? "/" : ""), m_request_param.c_str());
        //header_format_append("Host: %s", m_host.c_str());
        //header_format_append("Content-type: %s", get_string_content_type(m_content_type).c_str());
        //if (!m_boundary.empty()) header_format_append("; boundary=----------------------------%s", m_boundary.c_str());
        //header_format_append("Connection: Keep-Alive");
        //header_format_append("User-Agent: Brinicle");
        //header_format_append("Accept: %s", m_accept_encoding.empty() ? "*/*" : m_accept_encoding.c_str());
        //header_format_append("Pragma: no-cache");
        //header_format_append("Cache-Control: no-cache");
        return header;
    }

    virtual void replace_data_multipart(std::string& data, const std::string search, const std::string replace)
    {
        size_t pos = 0;
        while ((pos = data.find(search, pos)) != std::string::npos) {
            data.replace(pos, search.length(), replace);
            pos += replace.length();
        }
    }

    /* =============================================================================================
    * Example :
    *----------------------------918649584285083799389692
    * Content-Disposition: form-data; name="fileToUpload"; filename="simpson.png"
    * <simpson.png>
    * ----------------------------918649584285083799389692--
    *=============================================================================================*/
    virtual std::string pack_body_multipart(const std::string& name_param,     //
        const std::string& file_name,      //
        const char* data,                  //
        const int& nsize)                  //
    {
        std::string body;
        // body content format
        std::string str_start_body = "";
        std::string str_end_body = "";
        header_format_append(str_start_body, "----------------------------%s\r\n", _boundary.c_str());
        header_format_append(str_start_body, "Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n", name_param.c_str(), file_name.c_str());
        header_format_append(str_start_body, "Content-Type: multipart/form-data\r\n");
        header_format_append(str_end_body, "\r\n----------------------------%s--\r\n", _boundary.c_str());

        header_format_append(body, "%s", str_start_body.c_str());
        body.append(data, nsize);
        header_format_append(body, "%s", str_end_body.c_str());

        return body;
    }

    int update_header(const std::string header)
    {
        struct curl_slist* headers = NULL;
        curl_slist_append(headers, header.c_str());
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);
        this->reset_buffer();
        //curl_slist_free_all(headers);

        return 1;
    }
public:
    virtual int connect_upload_file(const std::string& url, const char* buf, const int& nsize)
    {

        //return 1;
        curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_curl, CURLOPT_POST, 1);
        if (m_connect_timeout > 0)
            curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT_MS, m_connect_timeout);

        std::string header = pack_header_multipart("127.0.0.1:8080", "/upload.php", "13515");


        /* Create the form */
        auto form = curl_mime_init(m_curl);

        /* Fill in the file upload field */
        auto field = curl_mime_addpart(form);
        curl_mime_type(field, "multipart/form-data");
        curl_mime_name(field, "fileToUpload");
        curl_mime_filedata(field, "simpson.png");
        curl_mime_data(field, buf, nsize);

        //std::string body   = pack_body_multipart("fileToUpload", "simpson.png", buf, nsize);
        replace_data_multipart(header, "%s", "13515");

        if (header.empty()) return -1;

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Accept: thuong");
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(m_curl, CURLOPT_MIMEPOST, form);
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

        this->reset_buffer();

        auto res = curl_easy_perform(m_curl);
        curl_slist_free_all(headers);
        curl_mime_free(form);
        if (res != CURLE_OK)
        {
            printf("Error: %s\n", curl_easy_strerror(res));
            return -1;
        }



    }

    virtual int connect_send_request(const std::string& url, const char* header, const int nheader)
    {
        curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, &Socket::write_header_callback);
        curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &_buf_header);
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &Socket::write_body_callback);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &_buf_body);

        if (m_connect_timeout > 0)
            curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT_MS, m_connect_timeout);

        // setup information header
        struct curl_slist* headers = NULL;
        std::string temp(header, nheader);
        curl_slist_append(headers, temp.c_str());
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);

        this->reset_buffer();
        auto res = curl_easy_perform(m_curl);


        curl_slist_free_all(headers);
        res = curl_easy_getinfo(m_curl, CURLINFO_ACTIVESOCKET, &_sock);
        if (res != CURLE_OK)
        {
            printf("Error: %s\n", curl_easy_strerror(res));
            return -1;
        }

        // revert to normal state orther socket
        curl_easy_setopt(m_curl, CURLOPT_CONNECT_ONLY, 1L);
        res = curl_easy_perform(m_curl);
        curl_easy_getinfo(m_curl, CURLINFO_ACTIVESOCKET, &_sock);

        return res;
    }

    std::string get_header_data()
    {
        return _buf_header;
    }

    std::string get_body_data()
    {
        return _buf_body;
    }

private:
    CURL* m_curl;
    long    m_connect_timeout = 1000; // ms
    int     m_retry_connect = 5;

private:
    std::string   _buf_header;
    std::string   _buf_body;
    std::string   _boundary = "918649584285083799389692";
private: // origin
    curl_socket_t _sock;
    bool          _block;

    bool          _bforce_stop;
};

class SSLSocket
{

};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static size_t WriteHeaderCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


bool check_end_recv(char* buff, const int& nsize)
{
    char recv_end[] = "\r\n\r\n";
    int  n_rev_end = sizeof(recv_end) - 1;
    if (nsize < n_rev_end)
        return false;

    return !memcmp(buff + (nsize - n_rev_end), recv_end, n_rev_end) ? true : false;
}

void pack_header_normal()
{

}
void pack_header_upload()
{

}

void pack_header_upload_multipart()
{

}


void pack_header()
{
    struct curl_slist* chunk = NULL;
    chunk = curl_slist_append(chunk, "Accept:");
    chunk = curl_slist_append(chunk, "Another: yes");
    chunk = curl_slist_append(chunk, "Host: example.com");
    chunk = curl_slist_append(chunk, "X-silly-header;");
}

void unpack_header()
{

}

int main1(void)
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    std::string header;

    curl = curl_easy_init();
    //if (curl) {
    //    curl_easy_setopt(curl, CURLOPT_URL, "http://wonderfulmajesticslowmorning.neverssl.com/online/");
    //    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    //    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
    //    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    //    //curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 100);
    //    //curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 100);
    //    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    //    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &WriteHeaderCallback);
    //    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header);
    //    res = curl_easy_perform(curl);

    //    std::cout << readBuffer << std::endl;
    //}

    const char* request = "GET / HTTP/1.1\r\nHost: vnexpress.net\r\n\r\n";
    size_t request_len = strlen(request);

    curl_socket_t sockfd;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://vnexpress.net/");
        curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            printf("Error: %s\n", curl_easy_strerror(res));
            return 1;
        }

        /* Extract the socket from the curl handle - we will need it for
        waiting. */
        res = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &sockfd);

        if (res != CURLE_OK) {
            printf("Error: %s\n", curl_easy_strerror(res));
            return 1;
        }

        size_t nsent_total = 0;
        do {
            /* Warning: This example program may loop indefinitely.
             * A production-quality program must define a timeout and exit this loop
             * as soon as the timeout has expired. */
            size_t nsent;
            do {
                nsent = 0;
                res = curl_easy_send(curl, request + nsent_total,
                    request_len - nsent_total, &nsent);
                nsent_total += nsent;

                if (res == CURLE_AGAIN && !wait_on_socket(sockfd, 0, 6000L)) {
                    printf("Error: timeout.\n");
                    return 1;
                }
            } while (res == CURLE_AGAIN);

            if (res != CURLE_OK) {
                printf("Error: %s\n", curl_easy_strerror(res));
                return 1;
            }

            printf("Sent %lu bytes.\n", (unsigned long)nsent);
        } while (nsent_total < request_len);
        printf("Error: %s\n", curl_easy_strerror(res));
        //
        std::string data;
        bool brecv_end = false;
        int n = 0;
        for (;;) {
            /* Warning: This example program may loop indefinitely (see above). */
            char buf[1024];
            size_t nread;
            do {
                nread = 0;
                res = curl_easy_recv(curl, buf, sizeof(buf), &nread);

                if (res == CURLE_OK && nread < sizeof(buf))
                {
                    brecv_end = true;
                    break;
                }
                if (res == CURLE_AGAIN && !wait_on_socket(sockfd, 1, 6000L)) {

                    curl_easy_cleanup(curl);
                    printf("Error: timeout.\n");
                    return 1;

                }
                n++;
            } while (res == CURLE_AGAIN);

            if (res != CURLE_OK) {
                printf("Error: %s\n", curl_easy_strerror(res));
                break;
            }

            if (nread == 0) {
                /* end of the response */
                break;
            }

            data.append(buf, nread);

            printf("Received %lu bytes.\n", (unsigned long)nread);

            if (brecv_end)
                break;
        }

        getchar();
        int a = 10;
    }

    curl_easy_cleanup(curl);

    return 0;
}

int callback_retryconnect(int code, LPVOID lParam)
{
    std::cout << "call back" << std::endl;

    return 1;
}

int callback_recv_try(int code, LPVOID lParam)
{
    std::cout << "callback_recv_try" << std::endl;
    return 1;
}



int main2()
{
    Socket sock;
    //std::cout <<"connect status : "<< sock.connect("https://vnexpress.net/", callback_retryconnect) << std::endl;
    //std::cout <<"connect status : "<< sock.connect("https://dantri.com.vn/", 80, callback_retryconnect) << std::endl;
    //std::cout <<"connect status : "<< sock.connect("127.0.0.1", 8080, callback_retryconnect) << std::endl;

    //const char* request = "GET / HTTP/1.1\r\nHost: vnexpress.net\r\n\r\n";
    //const char* request = "GET / HTTP/1.1\r\nHost: dantri.com.vn\r\n\r\n";
    //size_t request_len = strlen(request);

    //std::cout << "send data : " << sock.send(request, request_len, 500) << std::endl;


    //std::cout << "recv data :" << sock.recv(1000, callback_recv_try) << std::endl;


    //std::cout << sock.connect_send_request("https://vnexpress.net/", request, request_len);

    //std::string he = sock.get_header_data();
    //std::string bd = sock.get_body_data();


    //sock.send(request, request_len, 500);

    //std::cout << sock.recv(1000, callback_recv_try) << std::endl;


    //std::string nameparam = "fileToUpload";
    //std::string filename = "simpson.png";

    void* data;
    int nsize = read_data_file(L"simpson.png", &data);
    //sock.connect_upload_file("http://127.0.0.1:8080/upload.php", (const char*)data, nsize);
    //delete[] data;

    curl_header_info_ptr header = std::make_shared<curl_header_info>();

    header->set_host("127.0.0.1:8081");
    header->set_content_type(content_type::form_data);
    //header->set_request_param("/upload.php");

    //curl_multipart_info_ptr post = std::make_shared<curl_multipart_info>();
    //post->set_name("fileToUpload");
    //post->set_value_name("simpson.png");
    //post->set_data(data, nsize);

    //curl_urlencode_info_ptr post = std::make_shared< curl_urlencode_info>();

    //post->add_data("thuong", 123.34);
    //post->add_data("thuong", 123);
    //post->add_data("thuong", true);

    HTTPRequestOption option;
    option.bshow_request = false;
    option.dconnect_timeout = 1;
    option.itry_connect = 5;
    option.dlimit_upload = 20;
    option.dlimit_download = 20;

    CurlHTTPRequest request(option);

    //HTTPRequestCode code = request.post("http://127.0.0.1:8080/upload.php", header.get(), post.get());
    HTTPRequestCode code = request.get("http://127.0.0.1:80", header.get());


    std::string respheader = request.get_header_response();

    std::cout << respheader.c_str() << std::endl;

    delete[] data;

    getchar();
    return 1;
}