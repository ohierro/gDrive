#include "soap/soapH.h" // obtain the generated stub 
//#include "soap/DriveSoapBinding.nsmap" // obtain the generated XML namespace mapping table for the Quote service  
//#include "soap/stdsoap2.h"

int __cdecl main1() {

	struct soap *soap = soap_new();
	//struct_ns1__getFilesResponse response;
	struct _ns1__getFilesResponse response;
	struct _ns1__getFiles request;

		
	request.path= "/";
	
	if (soap_call___ns1__getFiles(soap,NULL,NULL,&request,&response) == SOAP_OK) {	
		printf("Current IBM Stock Quote = %d\n", response.__sizegetFilesReturn); 
		if (response.__sizegetFilesReturn > 0) {
			int i = 0;
			for (i=0; i<response.__sizegetFilesReturn; i++) {		
				if (response.getFilesReturn[i].attributes & 2)
					printf("D");
				else
					printf(" ");
				
				printf(" %s\n", response.getFilesReturn[i].name); 
			}
		}
		
	} else {
		char buffer[1024];
		soap_sprint_fault(soap,buffer,1024);
		printf("%s",buffer);
	}
		
	return 0;
}