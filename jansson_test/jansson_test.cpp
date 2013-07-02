// jansson_test.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"

#include "jansson.h"
//#include "comm.h"
#include "mycomm.h"
#include "StringConversions.h"
#include "Preferences.h"

#include <stdio.h>
#include <iostream>

#include <map>
#include <string>
#include <ctime>

#include "ctree.h"

using namespace std;

int ae_load_file_to_memory(const char *filename, char **result) 
{ 
	int size = 0;
	FILE *f = fopen(filename, "rb");
	if (f == NULL) 
	{ 
		*result = NULL;
		return -1; // -1 means file opening fail 
	} 
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*result = (char *)malloc(size+1);
	if (size != fread(*result, sizeof(char), size, f)) 
	{ 
		free(*result);
		return -2; // -2 means file reading fail 
	} 
	fclose(f);
	(*result)[size] = 0;
	return size;
}

int testJSON() {
	char *content; 
	int size;
	size = ae_load_file_to_memory("E:\\git\\jansson_test\\outputMessage.log", &content);

	json_error_t error;
	json_t* root = json_loads(content,0, &error);

	if(!root)
	{
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		return 1;
	}

	json_t* items = json_object_get(root,"items");
	
	if(!json_is_array(items))
	{
		fprintf(stderr, "error: items is not an array\n");
	    return 1;
	}
	
	for(int i = 0; i < json_array_size(items); i++)
	{
		json_t* data = json_array_get(items, i);
	    json_t* title = json_object_get(data, "title");
		if(!json_is_string(title))	{
			
		} else {
			const char* messageText = json_string_value(title);
		}
	}

	return 0;
}

PFILE_INFO_NODE findNodeInList(wstring nodeName, PFILE_INFO_NODE list) {
	PFILE_INFO_NODE node = list;

	while (node != NULL) {
		if ( !wcscmp(nodeName.c_str(), node->info->name.c_str()) ) {
			return node;
		}

		node = node->next;
	}
}

PFILE_INFO_NODE findFile(PAUTH_DATA authData, wstring filePath) {
	int level = getFolderLevels(filePath);

	if (level == 0) {
		wstring file = getFolder(filePath, 0);	
		if ( !wcscmp(file.c_str(),L"") ) {
			PFILE_INFO info = new FILE_INFO();
			info->id = L"\\";
			info->name = L"root";
			info->size = -1;
			info->attributes = ATT_DIRECTORY;

			PFILE_INFO_NODE infoNode = new FILE_INFO_NODE();
			infoNode->info = info;

			return infoNode;
		} else {
			PFILE_INFO_NODE list = listRoot(authData);
			return findNodeInList(file, list);
		}
	} else {
		PFILE_INFO_NODE list = listRoot(authData);
		PFILE_INFO_NODE first = findNodeInList(getFolder(filePath,0), list);

		PFILE_INFO_NODE current = first;

		for (int i=1; i<=level; i++) {
			PFILE_INFO_NODE nodes = listFolder(authData, current->info->id);
			current = findNodeInList(getFolder(filePath,i),nodes);
		}

		return current;
	}

	return NULL;
}

void listFolder(PAUTH_DATA authData, wstring id, bool recursive, int level) {
	PFILE_INFO_NODE folderList = listFolder(authData, id);
			
	PFILE_INFO_NODE current = folderList;
	while (current != NULL) {	
		for (int i=0; i<level; i++) cout << "\t";

		cout << "=";		

		if (current->info->attributes == ATT_DIRECTORY) {
			cout << "DIR => (" << wide2char(current->info->id) << ") - " << wide2char(current->info->name) << endl;

			if (recursive) {
				listFolder(authData, current->info->id, recursive, level+1);
			}
		} else {
			PGFILE_INFO info = (PGFILE_INFO) current->info;

			cout << wide2char(current->info->name) << ": " << current->info->size << " bytes -> " << wide2char(info->downloadUrl) << endl;
		}

		current = current->next;
	}
}

void listRoots(PAUTH_DATA authData, bool recursive) {
	PFILE_INFO_NODE ret = listRoot(authData);
	PFILE_INFO_NODE current = ret;

	while (current != NULL) {
		if (current->info->attributes == ATT_DIRECTORY) {
			cout << "DIR => (" << wide2char(current->info->id) << ") - " << wide2char(current->info->name) << endl;			

			if (recursive) {
				wstring fId = current->info->id;
				listFolder(authData, fId, recursive, 1);
			}
		} else {
			PGFILE_INFO info = (PGFILE_INFO) current->info;

			cout << wide2char(current->info->name) << ": " << current->info->size << " bytes -> " << wide2char(info->downloadUrl) << endl;
		}

		current = current->next;
	}
}

void fileInfo() {

}

PAUTH_DATA registerApp() {
	string code;

	cout << "Registering application..." << endl;
	system("explorer \"register.html\"");
	cout << "Insert auth code here: ";
	cin >> code;

	wstring wcode = char2wide(code.c_str());
	cout << "Code: " << code;
	return registerApp(wcode);
}

time_t parseTime(std::wstring& date) {
	int day,month,year,hour,minute,second;
	time_t rawTime;
	time(&rawTime);
	tm* tm = localtime(&rawTime);

	sscanf(wide2char(date),"%d/%d/%d - %d:%d:%d", &month, &day, &year, &hour, &minute, &second);

	tm->tm_mon = month - 1;
	tm->tm_year = year + 100;
	tm->tm_mday = day;
	
	tm->tm_hour = hour;
	tm->tm_min = minute;
	tm->tm_sec = second;
		
	tm->tm_isdst = -1;

	time_t requestTime = mktime(tm);
	return requestTime;
}

bool checkValidCredentials(std::wstring authRequest, long expires) {
	
	time_t rawTime;
	time(&rawTime);
	tm* tm = localtime(&rawTime);
	

	//strptime(authRequest,"%x - %X",&tm);
	int day,month,year,hour,minute,second;
	sscanf(wide2char(authRequest),"%d/%d/%d - %d:%d:%d", &month, &day, &year, &hour, &minute, &second);
	tm->tm_mon = month - 1;
	tm->tm_year = year + 100;
	tm->tm_mday = day;
	
	tm->tm_hour = hour;
	tm->tm_min = minute;
	tm->tm_sec = second;
		
	tm->tm_isdst = -1;

	time_t requestTime = mktime(tm);
	

	time_t currentTime;
	time(&currentTime);
	gmtime(&currentTime);

	if (difftime(currentTime,requestTime) > expires) {
		return false;
	}
	
	return true;	
}

PAUTH_DATA loadData() {
	Preferences preferences;
	std::map<std::wstring, std::wstring>* data = preferences.loadData();

	std::wstring accessToken = (*data)[L"ACCESS_TOKEN"];
	std::wstring refreshToken = (*data)[L"REFRESH_TOKEN"];
	std::wstring tokenType = (*data)[L"TOKEN_TYPE"];
	std::wstring expires = (*data)[L"EXPIRES"];
	std::wstring authRequest = (*data)[L"AUTH_REQUEST"];	
	
	int iExpires = atoi(wide2char(expires.c_str()));	

	PGAUTH_DATA authData = new GAUTH_DATA();
	authData->accessToken = accessToken;
	authData->refreshToken = refreshToken;
	authData->tokenType = tokenType;
	authData->expires = iExpires;
	authData->requestTime = parseTime(authRequest);

	if (!checkValidCredentials(authRequest,iExpires)) {
		// renew credentials
		PGAUTH_DATA newAuthData = (PGAUTH_DATA) renewAuth(refreshToken);

		// save new data....
		std::map<std::wstring, std::wstring> data;
		char buffer[100];
		_itoa_s(newAuthData->expires,buffer,100,10);
	
		data[L"ACCESS_TOKEN"] = newAuthData->accessToken;	
		data[L"REFRESH_TOKEN"] = newAuthData->refreshToken;
		data[L"TOKEN_TYPE"] = newAuthData->tokenType;

		_itoa_s(newAuthData->expires,buffer,100,10);
		data[L"EXPIRES"] = char2wide(buffer);

		tm* tm = gmtime(&(newAuthData->requestTime));
		strftime(buffer, 100, "%x - %X", tm);
		data[L"AUTH_REQUEST"] = char2wide(buffer);	

		preferences.saveData(data);
		return newAuthData;
	}

	return authData;
}

PFILE_INFO_NODE findNode(CTree<PFILE_INFO_NODE> tree, wstring fileName) {
	std::vector<CTree<PFILE_INFO_NODE>*>::iterator it;

	for (it = tree.children.begin(); it!= tree.children.end(); ++it) {
		if ((*it)->item->info->attributes == ATT_DIRECTORY) {
			return (*it)->item;
		}
	}	
	return tree.children[0]->item;	
}

void updateTree(CTree<PFILE_INFO_NODE> tree) {
	
	
}


int testGetNextFolder() {
	wstring result;

	cout << "Testing " << "\\" << endl;
	result = parsePath(L"\\");
	cout << "---------------------------" << endl;

	cout << "Testing " << "\\" << endl;
	result = parsePath(L"\\unaCarpeta");
	cout << "---------------------------" << endl;

	cout << "Testing " << "\\algo\\mas" << endl;
	result = parsePath(L"\\algo\\mas");
	cout << "---------------------------" << endl;

	cout << "Testing " << "\\segundo\\nivel\\omas" << endl;
	result = parsePath(L"\\segundo\\nivel\\omas");
	cout << "---------------------------" << endl;

	return 0;
}

int testGetFile(PAUTH_DATA authData) {	
	cout << "Testing \\ ..." << endl;
	PFILE_INFO_NODE node = findFile(authData, L"\\");
	if (node) {
		if (node->info->attributes == ATT_DIRECTORY) {
			cout << "DIR ID: " << wide2char(node->info->id) << endl;
		} else {
			cout << "NODE ID: " << wide2char(node->info->id) << endl;
		}
	}

	cout << "Testing \\Carpeta de prueba ..." << endl;
	node = findFile(authData, L"\\Carpeta de prueba");
	if (node) {
		if (node->info->attributes == ATT_DIRECTORY) {
			cout << "DIR ID: " << wide2char(node->info->id) << endl;
		} else {
			cout << "NODE ID: " << wide2char(node->info->id) << endl;
		}
	}
	
	cout << "Testing \\Carpeta de prueba\\Carpeta INSIDE ..." << endl;
	node = findFile(authData, L"\\Carpeta de prueba\\Carpeta INSIDE");
	if (node) {
		if (node->info->attributes == ATT_DIRECTORY) {
			cout << "DIR ID: " << wide2char(node->info->id) << endl;
		} else {
			cout << "NODE ID: " << wide2char(node->info->id) << endl;
		}
	}

	cout << "Testing \\Carpeta de prueba\\TEST ..." << endl;
	node = findFile(authData, L"\\Carpeta de prueba\\TEST");
	if (node) {
		if (node->info->attributes == ATT_DIRECTORY) {
			cout << "DIR ID: " << wide2char(node->info->id) << endl;
		} else {
			cout << "NODE ID: " << wide2char(node->info->id) << endl;
		}
	}
	
	cout << "Testing \\CLIENT_SIU_PAU.zip ..." << endl;
	node = findFile(authData, L"\\CLIENT_SIU_PAU.zip");
	if (node) {
		if (node->info->attributes == ATT_DIRECTORY) {
			cout << "DIR ID: " << wide2char(node->info->id) << endl;
		} else {
			cout << "NODE ID: " << wide2char(node->info->id) << endl;
		}
	}

	return 0;
}

void testDownloadFile(PAUTH_DATA authData) {
	PFILE_INFO_NODE node = findFile(authData, L"\\FACE_-_Manual_de_usuario_portal_de_proveedores_v1.2.pdf");
	FILE* output = getFile(authData, node);
}

int _tmain(int argc, _TCHAR* argv[]) {
	PAUTH_DATA authData = loadData();
	//listRoots(authData, true);

	//testGetNextFolder();
	//testGetPath(authData);

	testDownloadFile(authData);
}

