#ifndef __MY_COMM_H__
#define __MY_COMM_H__

#include "comm.h"
#include <map>

/* CUSTOM FILE INFO */
typedef struct gfile_info : file_info {	
	std::wstring downloadUrl;
	std::map<std::wstring, std::wstring> exportUrls;
} GFILE_INFO, *PGFILE_INFO;

typedef struct gauth_data : auth_data {
	std::wstring accessToken;	
	std::wstring tokenType;
	std::wstring refreshToken;
	time_t requestTime;
	long expires;
} GAUTH_DATA, *PGAUTH_DATA;


// GDRIVE OPERATIONS
#define		LIST_FILES		L"https://www.googleapis.com/drive/v2/files"
#define		LIST_CHILDREN	L"https://www.googleapis.com/drive/v2/files?q=%27{folderId}%27+in+parents"
#define		GET_FILE		L"https://www.googleapis.com/drive/v2/files/{fileId}"


#endif // __MY_COMM_H__