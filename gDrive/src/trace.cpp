#include "trace.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define FATAL	0
#define ERROR	1
#define WARNING 2
#define INFO	3
#define DEBUG	4


static FILE* output = NULL;
wchar_t* levelMessage[] = { L"FATAL", L"ERROR", L"WARNING", L"INFO", L"DEBUG" };

void initTrace(wchar_t* path) {
	if (output == NULL)
		output = _wfopen(path,L"w");
}

//void trace(int level, const wchar_t* message) {
//	wchar_t buffer[16384];
//	
//	swprintf(buffer,L"%s\t%s\t\t%s","01/01/01",levelMessage[level],message);
//	
//	fwrite(buffer,1,sizeof(wchar_t)*wcslen(buffer),output);
//	fwrite("\n",1,sizeof(wchar_t),output);
//	fflush(output);
//}

void trace(int level, const wchar_t* message, ...) {
	wchar_t buffer[16384];
	wchar_t buffer2[16384];
	char charBuffer[16384];
	int size;

	va_list arguments;
	va_start(arguments,message);

	vwprintf(message,arguments);
	vswprintf(buffer, 16384, message, arguments);	

	va_end(arguments);

	swprintf(buffer2,L"%s\t%s\t\t%s",L"01/01/01",levelMessage[level],buffer);
	size = wcstombs(&(charBuffer[0]),buffer2,wcslen(buffer2));
	
	fwrite(charBuffer,1,size,output);
	fwrite("\n",1,1,output);
	fflush(output);
}


//char* UnicodeToAnsi(wchar_t* s) {
//	if (s==NULL) return NULL;
//	
//	int cw=lstrlenW(s);
//	
//	if (cw==0) {
//		CHAR *psz=new CHAR[1];
//		*psz='\0';
//		return psz;
//	}
//	int cc=WideCharToMultiByte(CP_ACP,0,s,cw,NULL,0,NULL,NULL);
//	
//	if (cc==0) return NULL;
//
//	CHAR *psz=new CHAR[cc+1];
//	cc=WideCharToMultiByte(CP_ACP,0,s,cw,psz,cc,NULL,NULL);
//	if (cc==0) {
//		delete[] psz;
//		return NULL;
//	}
//
//	psz[cc]='\0';
//	return psz;
//}
