#include "file.h"
#include "soap/soapH.h" // obtain the generated stub 
#include "soap/DriveSoapBinding.nsmap" // obtain the generated XML namespace mapping table for the Quote service  

#include "trace.h"

typedef char *xsd__string; 

void lockFile(char* path) {

}

PFILE_INFO_NODE getFiles(char* path) {
	struct soap *soap = soap_new();
	struct _ns1__getFilesResponse response;
	struct _ns1__getFiles request;

	PFILE_INFO_NODE list = NULL;
	PFILE_INFO_NODE last = NULL;
	PFILE_INFO_NODE node = NULL;

	int size = 0;
	
	//strcpy(soap->endpoint,"http://zaz-hierroo.tb-solutions.com:8085/VDrive/services/Drive");
	//wctomb()
	printf("\n\n\ngetFiles: %s\n\n\n",path);
	request.path = path;
	
	if (soap_call___ns1__getFiles(soap,"http://zaz-hierroo.tb-solutions.com:8085/VDrive/services/Drive",NULL,&request,&response) == SOAP_OK) {	
		//printf("Current IBM Stock Quote = %d\n", response.__sizegetFilesReturn); 
//		TRACE(DEBUG,L"Total files: %d",response.__sizegetFilesReturn);

		TRACE(DEBUG,L"Get response");

		TRACE(DEBUG,L"Files in directory: %d",response.__sizegetFilesReturn);
		
		if (response.getFilesReturn != NULL) 
			TRACE(DEBUG,L"Response not NULL");
		else
			TRACE(DEBUG,L"Response NULL");

		//if (response.getFilesReturn != NULL) {	
		if (response.__sizegetFilesReturn > 0) {
			int i = 0;

			if (response.getFilesReturn[0].name != NULL) 
				TRACE(DEBUG,L"Response[0] not NULL");
			else
				TRACE(DEBUG,L"Response[0] NULL");


			for (i=0; i<response.__sizegetFilesReturn; i++) {					
				PFILE_INFO info = (PFILE_INFO) malloc(sizeof(FILE_INFO));
				TRACE(DEBUG,L"Get files");
				

				if (response.getFilesReturn[i].name == NULL) {
					TRACE(DEBUG,L"Name it's NULL. Neeeeeext");
					continue;
				}

				// String conversions
				size = mbstowcs(NULL,response.getFilesReturn[i].name,strlen(response.getFilesReturn[i].name));
				TRACE(DEBUG,L"Size: %d",size);
				info->name = (WCHAR*) malloc((size+1)*sizeof(WCHAR));
			
				size = mbstowcs(info->name,response.getFilesReturn[i].name,strlen(response.getFilesReturn[i].name));

				// Set node information
				info->name[size] = 0;
				TRACE(DEBUG,L"File: %s",info->name);

				info->id = response.getFilesReturn[i].id;
				info->size = response.getFilesReturn[i].size;
				info->attributes = response.getFilesReturn[i].attributes;

				// Add new node to list				
				node = (PFILE_INFO_NODE) malloc(sizeof(PFILE_INFO_NODE));
				memset(node,0,sizeof(PFILE_INFO_NODE));
				node->info = info;
				node->next = NULL;

				if (list == NULL) {					
					list = node;
					last = node;
				} else {
					last->next = node;
					last = node;
				}
			}
		}		
	} else {
		char buffer[1024];

		TRACE(DEBUG,L"no files");
		
		soap_sprint_fault(soap,buffer,1024);
		printf("%s",buffer);
	}
		
	return list;
}


PFILE_INFO_NODE getFile(char* path) {
	struct soap *soap = soap_new();
	struct _ns1__getFileResponse response;
	struct _ns1__getFile request;

	PFILE_INFO_NODE list = NULL;
	PFILE_INFO_NODE last = NULL;
	PFILE_INFO_NODE node = NULL;

	int size = 0;
	
	//strcpy(soap->endpoint,"http://zaz-hierroo.tb-solutions.com:8085/VDrive/services/Drive");
	//wctomb()
	printf("\n\n\ngetFile: %s\n\n\n",path);
	request.path = path;

	if (soap_call___ns1__getFile(soap,"http://zaz-hierroo.tb-solutions.com:8085/VDrive/services/Drive",NULL,&request,&response) == SOAP_OK) {	
		//printf("Current IBM Stock Quote = %d\n", response.__sizegetFilesReturn); 
		//TRACE(DEBUG,L"Total files: %d",response.__sizegetFileReturn);

		if (response.getFileReturn != NULL) {			
			PFILE_INFO info = (PFILE_INFO) malloc(sizeof(FILE_INFO));

			size = mbstowcs(NULL,response.getFileReturn->name,strlen(response.getFileReturn->name));
			TRACE(DEBUG,L"Size: %d",size);
			info->name = (WCHAR*) malloc((size+1)*sizeof(WCHAR));
			
			size = mbstowcs(info->name,response.getFileReturn->name,strlen(response.getFileReturn->name));

			info->name[size] = 0;
			TRACE(DEBUG,L"File: %s",info->name);

			info->id = response.getFileReturn->id;
			info->size = response.getFileReturn->size;
			info->attributes = response.getFileReturn->attributes;				

			node = (PFILE_INFO_NODE) malloc(sizeof(PFILE_INFO_NODE));
			memset(node,0,sizeof(PFILE_INFO_NODE));
			node->info = info;
//			node->next = NULL;
		}		
	} else {
		char buffer[1024];
		soap_sprint_fault(soap,buffer,1024);
		printf("%s",buffer);
	}
		
	return node;
}

FILE* getFileContent(char* path,char* buffer) {
	struct soap *soap = soap_new();
	struct _ns1__getFileContentResponse response;
	struct _ns1__getFileContent request;

	PFILE_INFO_NODE list = NULL;
	PFILE_INFO_NODE last = NULL;
	PFILE_INFO_NODE node = NULL;
	//xsd__string data;
	//char* data;

	int size = 0;
	FILE* out;
	
	//strcpy(soap->endpoint,"http://zaz-hierroo.tb-solutions.com:8085/VDrive/services/Drive");
	//wctomb()
	printf("\n\n\ngetFile: %s\n\n\n",path);
	request.path = path;

	if (soap_call___ns1__getFileContent(soap,"http://zaz-hierroo.tb-solutions.com:8085/VDrive/services/Drive",NULL,&request,&response) == SOAP_OK) {	

//		data = response.getFileContentReturn;
				//printf("Current IBM Stock Quote = %d\n", response.__sizegetFilesReturn); 
		//TRACE(DEBUG,L"Total files: %d",response.__sizegetFilesReturn);
		

		//if (response.getFileContentReturn != NULL) {
			TRACE(DEBUG,L"GET_FILE_CONTENT_SIZE %d",response.getFileContentReturn.__size);
			TRACE(DEBUG,L"GET_FILE_CONTENT_TYPE %s",response.getFileContentReturn.type);


			// to temp file
			out = fopen("c:\\temp\\TMP-DOKAN-FILE","wb+");
			if (out == NULL)
				TRACE(ERROR,L"Can't use temporary file!!!");


			fwrite(response.getFileContentReturn.__ptr,1,response.getFileContentReturn.__size,out);
			fflush(out);
						
			//data = malloc(response.getFileContentReturn.__size*sizeof(char));
			//memcpy(data,response.getFileContentReturn.__ptr,response.getFileContentReturn.__size);
			//memcpy(buffer,response.getFileContentReturn.__ptr,response.getFileContentReturn.__size);
			fseek(out,0L,SEEK_SET);

			return out;

//			int i = 0;
//			//for (i=0; i<response.__sizegetFilesReturn; i++) {	
//
//			//data = response.getFileContentReturn;
//			
//			return data;
		//}
//		
	} else {
		char buffer[1024];
		soap_sprint_fault(soap,buffer,1024);
		printf("%s",buffer);
	}
		
	return NULL;
}