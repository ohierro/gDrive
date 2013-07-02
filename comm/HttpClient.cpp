#include "stdafx.h"

#include <stdlib.h>

#include "HttpClient.h"
#include "StringConversions.h"

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);


void HttpClient::init() {
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
}
	
void HttpClient::addHeader(const WCHAR* header, const WCHAR* value) {
	std::wstring val = header;
	val += L": ";
	val += value;

	headers.push_back(val);
}

void HttpClient::addData(const wstring& name, const wstring& value) {
	std::pair<std::wstring,std::wstring> data;
	data.first = name;
	data.second = value;

	formData.push_back(data);
}

void HttpClient::clearData() {
	formData.clear();
}

HttpResponse* HttpClient::post(const wstring& url) {
	CURLcode res;	

	struct MemoryStruct chunk; 
	chunk.memory = (char*) malloc(1);  /* will be grown as needed by the realloc above */ 
	chunk.size = 0;    /* no data at this point */

	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, wide2char(url.c_str()));		
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_POST, 1L);

		/// HEADERS
		struct curl_slist *headerList = NULL;

		for(std::vector<std::wstring>::iterator it = headers.begin(); 
			it != headers.end();
			it++)
		{
			char* header;			
			header = wide2char(it->c_str());

			headerList = curl_slist_append(headerList, header);
		}

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

		/* send all data to this function  */ 
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 
		/* we pass our 'chunk' struct to the callback function */ 
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
 
		/* some servers don't like requests that are made without a user-agent
		field, so we provide one */ 
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		
		struct curl_httppost* post = NULL;
		struct curl_httppost* last = NULL;

		
		wstring params;
		for(std::vector<std::pair<std::wstring, std::wstring>>::iterator it = formData.begin(); 
			it != formData.end();
			it++) {			
				//curl_formadd(&post, &last, CURLFORM_COPYNAME, wide2char(it->first.c_str()), CURLFORM_COPYCONTENTS, wide2char(it->second.c_str()), CURLFORM_END);
				params += it->first;
				params += L"=";
				params += it->second;
				params += L"&";
		}

		params = params.substr(0,params.length()-1);
					
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, wide2char(params.c_str()));		 
		//curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		} else {
			// chunk has data!!
		}

		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}
 
	curl_global_cleanup();
	HttpResponse* response = new HttpResponse();
	response->data = chunk;
	response->code = res;

	return response;
}
	
HttpResponse* HttpClient::request(const wstring& url) {
	CURLcode res;	

	struct MemoryStruct chunk; 
	chunk.memory = (char*) malloc(1);  /* will be grown as needed by the realloc above */ 
	chunk.size = 0;    /* no data at this point */
	
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, wide2char(url.c_str()));
		
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		/// HEADERS
		struct curl_slist *headerList = NULL;
 
		//headers = curl_slist_append(headers, "Authorization: OAuth ya29.AHES6ZSbVSgkJ89BmU0UlkqYGUtD979p35N1w8HD7JFm6w");
		//headers = curl_slist_append(headers, "Content-type: application/json");
		for(std::vector<std::wstring>::iterator it = headers.begin(); 
			it != headers.end();
			it++)
		{
			char* header;			
			//header = wide2char(&(*it->c_str()));
			header = wide2char(it->c_str());

			headerList = curl_slist_append(headerList, header);
		}

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
		/// HEADERS

		/* send all data to this function  */ 
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 
		/* we pass our 'chunk' struct to the callback function */ 
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
 
		/* some servers don't like requests that are made without a user-agent
		field, so we provide one */ 
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		
		 /* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		} else {
			// chunk has data!!
		}

		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}
 
	curl_global_cleanup();
	HttpResponse* response = new HttpResponse();
	response->data = chunk;
	response->code = res;

	return response;
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}