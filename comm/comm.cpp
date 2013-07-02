// comm.cpp: define las funciones exportadas de la aplicación DLL.
//

#include "stdafx.h"
#include "comm.h"
#include "mycomm.h"

#include "HttpClient.h"
#include "ResponseParser.h"
#include "StringConversions.h"

// Authentication data
PGAUTH_DATA credentials = NULL;

COMM_API PFILE_INFO_NODE init(WCHAR* configPath) {
	return NULL;
}


COMM_API PFILE_INFO_NODE listRoot(PAUTH_DATA authData) {
	HttpClient client;
	client.init();

	wstring authHeader = L"Bearer ";
	authHeader += ((PGAUTH_DATA)authData)->accessToken;

	client.addHeader(L"Authorization",authHeader.c_str());

	HttpResponse* response = client.request(LIST_FILES);

	ResponseParser parser;
	PFILE_INFO_NODE nodeList = parser.parseListRoot(response->data.memory);
		
	return nodeList;
}

COMM_API PFILE_INFO_NODE listFolder(PAUTH_DATA authData, std::wstring& name) {
	HttpClient client;
	client.init();

	wstring authHeader = L"OAuth ";
	authHeader += ((PGAUTH_DATA)authData)->accessToken;

	client.addHeader(L"Authorization",authHeader.c_str());

	wstring url = replace(LIST_CHILDREN,L"{folderId}",name);

	HttpResponse* response = client.request(url);

	ResponseParser parser;
	PFILE_INFO_NODE nodeList = parser.parseListFolder(response->data.memory);
		
	return nodeList;
}

COMM_API FILE* getFile(PAUTH_DATA authData, PFILE_INFO_NODE file) {
	HttpClient client;

	client.init();

	wstring authHeader = L"Bearer ";
	authHeader += ((PGAUTH_DATA)authData)->accessToken;

	client.addHeader(L"Authorization",authHeader.c_str());
	
	PGFILE_INFO info = (PGFILE_INFO) file->info;

	HttpResponse* res = NULL;

	if (!info->downloadUrl.empty()) {
		res = client.request(info->downloadUrl);
	} else {
		res = client.request(info->exportUrls[L"application/pdf"]);
	}

	FILE* output = fopen("c:\\tmpOutput.tmp","wb");

	fwrite(res->data.memory,1,res->data.size,output);

	return output;
}

COMM_API PAUTH_DATA registerApp(std::wstring& code) {
	HttpClient client;

	client.init();
	
	client.addData(L"code",code);
	client.addData(L"client_id",L"474401029373-7d5bv3o7mrhl9pvmbs5m1d7ucq85t6sd.apps.googleusercontent.com");
	client.addData(L"client_secret",L"GPY4-M7rwLzcTD_A0HjXb1WL");
	client.addData(L"redirect_uri",L"urn:ietf:wg:oauth:2.0:oob");
	client.addData(L"grant_type",L"authorization_code");

	HttpResponse* res = client.post(L"https://accounts.google.com/o/oauth2/token");

	ResponseParser parser;
	credentials = parser.parseAuthData(res->data.memory);
	time(&(credentials->requestTime));

	return credentials;
}

COMM_API PAUTH_DATA renewAuth(std::wstring& code) {
	HttpClient client;

	client.init();
	
	client.addData(L"refresh_token",code);
	client.addData(L"client_id",L"474401029373-7d5bv3o7mrhl9pvmbs5m1d7ucq85t6sd.apps.googleusercontent.com");
	client.addData(L"client_secret",L"GPY4-M7rwLzcTD_A0HjXb1WL");
	//client.addData(L"redirect_uri",L"urn:ietf:wg:oauth:2.0:oob");
	client.addData(L"grant_type",L"refresh_token");

	HttpResponse* res = client.post(L"https://accounts.google.com/o/oauth2/token");
	//HttpResponse* res = client.post(L"http://10.50.102.213:4444/o/oauth2/token");

	ResponseParser parser;
	credentials = parser.parseAuthData(res->data.memory);
	credentials->refreshToken = code; // PARSER REFRESH-TOKEN BUG!!
	time(&(credentials->requestTime));

	return credentials;
}



