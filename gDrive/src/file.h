#ifndef  __FILE_H__
#define __FILE_H__

#define ATT_DIRECTORY 0x0002

#include <windows.h>

typedef struct FILE_INFO {
	long id;
	WCHAR* name;
	long size;
	int attributes;
} FILE_INFO, *PFILE_INFO;


typedef struct FILE_INFO_NODE {
	PFILE_INFO info;
	struct FILE_INFO_NODE *next;
} FILE_INFO_NODE, *PFILE_INFO_NODE;


PFILE_INFO_NODE getFiles(char* path);


#endif


