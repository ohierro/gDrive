#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include <vector>
#include <curl/curl.h>
#include <string>

using namespace std;

struct MemoryStruct {
	int responseCode;
	char *memory;
	size_t size;
};

struct HttpResponse {
	int code;
	MemoryStruct data;
};


class HttpClient {

public:
	void init();	
	/* HEADERS */
	void addHeader(const WCHAR* header, const WCHAR* value);	

	/* DATA */
	void addData(const wstring& name, const wstring& value);
	void clearData();

	/* NETWORK OPERATIONS */
	HttpResponse* request(const wstring& url);
	HttpResponse* post(const wstring& url);

private:
	std::vector<std::wstring> headers;
	std::vector<std::pair<std::wstring, std::wstring>> formData;
	CURL *curl;
};

#endif

