  /*

Copyright (c) 2007, 2008 Hiroki Asakawa info@dokan-dev.net

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include "dokan.h"

#include "trace.h"
#include "comm.h"
#include "gDrive.h"
#include "ctree.h"

// SOAP CALL
//#include "soap/soapH.h" // obtain the generated stub 
//#include "soap/DriveSoapBinding.nsmap" // obtain the generated XML namespace mapping table for the Quote service  

BOOL g_UseStdErr;
BOOL g_DebugMode;
PAUTH_DATA authData;
CTree<PFILE_INFO_NODE> tree;

#define INVALID_SET_FILE_POINTER	0xFFFFFFFF



static void DbgPrint(LPCWSTR format, ...)
{
	if (g_DebugMode) {
		WCHAR buffer[512];
		va_list argp;
		va_start(argp, format);
		//vswprintf_s(buffer, sizeof(buffer)/sizeof(WCHAR), format, argp);
		vswprintf(buffer, 512, format,argp);
		va_end(argp);
		if (g_UseStdErr) {
			fwprintf(stderr, buffer);
		} else {
			OutputDebugStringW(buffer);
		}
	}
}

static WCHAR RootDirectory[MAX_PATH] = L"C:";

typedef struct file_position {
	FILE* handle;
	char* data;
	long length;
	long offset;	
} FILE_POSITION,*PFILE_POSITION;

static void
GetFilePath(
	PWCHAR	filePath,
	LPCWSTR FileName)
{
	RtlZeroMemory(filePath, MAX_PATH);
	wcsncpy(filePath, RootDirectory, wcslen(RootDirectory));
	wcsncat(filePath, FileName, wcslen(FileName));
}


#define MirrorCheckFlag(val, flag) if (val&flag) { DbgPrint(L"\t" L#flag L"\n"); }

static int
MirrorCreateFile(
	LPCWSTR					FileName,
	DWORD					AccessMode,
	DWORD					ShareMode,
	DWORD					CreationDisposition,
	DWORD					FlagsAndAttributes,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	WCHAR filePath[MAX_PATH];
	HANDLE handle;
	char* buffer;
	int error;
	PFILE_INFO info = NULL;

	TRACE(INFO,L"MirrorCreateFile %s\n", FileName);
	//TRACE(DEBUG,L"FileName: %s",FileName);
	
	TRACE(DEBUG,L"AccessMode: %d",AccessMode);
	TRACE(DEBUG,L"ShareMode: %d",ShareMode);
	TRACE(DEBUG,L"CreationDisposition: %d",CreationDisposition);
	TRACE(DEBUG,L"FlagsAndAttributes: %d",FlagsAndAttributes);		

	if (CreationDisposition == OPEN_EXISTING) {
		TRACE(INFO,L"OPEN_EXISTING\n");

		PFILE_INFO_NODE node = findFile(authData, FileName);
		if (node == NULL) {
			TRACE(INFO,L"FILE NOT FOUND!\n");
			SetLastError(ERROR_FILE_NOT_FOUND);
			DokanFileInfo->Context = (ULONG64)INVALID_HANDLE_VALUE;
			return ERROR_FILE_NOT_FOUND * -1;
		} else {
			TRACE(INFO,L"FILE FOUND!\n");
			DokanFileInfo->Context = (ULONG64)node;
		}		
	}

	return 0;
}


static int
MirrorCreateDirectory(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	WCHAR filePath[MAX_PATH];
	GetFilePath(filePath, FileName);

	TRACE(INFO,L"MirrorCreateDirectory: %s\n", FileName);	

	return 0;
}


static int
MirrorOpenDirectory(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	TRACE(INFO,L"MirrorOpenDirectory");
	TRACE(INFO,L"FileName: %s\n",FileName);

	if (!wcscmp(FileName,L"\\")) {
		TRACE(INFO,L"Request root directory\n");
		//PFILE_INFO_NODE roots = listRoot(authData);
		PFILE_INFO info = new FILE_INFO();
		info->id = L"\\";
		info->name = L"root";
		info->size = -1;
		info->attributes = -1;

		PFILE_INFO_NODE infoNode = new FILE_INFO_NODE();
		infoNode->info = info;

		DokanFileInfo->Context = (ULONG64)infoNode; 
		tree.item = infoNode;
	} else {
		TRACE(INFO,L"Request directory %s\n", FileName);
		
		/*PFILE_INFO_NODE node = findNode(tree, FileName);
		DokanFileInfo->Context = (ULONG64)node; 
		*/
		PFILE_INFO_NODE node = findFile(authData, FileName);
		DokanFileInfo->Context = (ULONG64)node; 
	}

	return 0;
}


static int
MirrorCloseFile(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	TRACE(INFO,L"MirrorCloseFile");
	TRACE(INFO,L"FileName: %s\n",FileName);
	
	return 0;
}


static int
MirrorCleanup(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	TRACE(INFO,L"MirrorCleanup");
	TRACE(INFO,L"FileName: %s\n",FileName);

	return 0;
}


static int
MirrorReadFile(
	LPCWSTR				FileName,
	LPVOID				Buffer,
	DWORD				BufferLength,
	LPDWORD				ReadLength,
	LONGLONG			Offset,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	TRACE(INFO,L"MirrorReadFile");
	TRACE(DEBUG,L"FileName: %s",FileName);
	/*
	TRACE(DEBUG,L"Handle: %d",handle);
	TRACE(DEBUG,L"Buffer length: %d",BufferLength);
	TRACE(DEBUG,L"Buffer offset: %d",Offset);	
	*/
	return 0;
}


static int
MirrorWriteFile(
	LPCWSTR		FileName,
	LPCVOID		Buffer,
	DWORD		NumberOfBytesToWrite,
	LPDWORD		NumberOfBytesWritten,
	LONGLONG			Offset,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	TRACE(INFO,L"MirrorWriteFile");
	TRACE(DEBUG,L"FileName: %s",FileName);

	return 0;
}


static int
MirrorFlushFileBuffers(
	LPCWSTR		FileName,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	TRACE(INFO,L"MirrorFlushFileBuffers");
	TRACE(DEBUG,L"FileName: %s",FileName);

	return 0;
}

/***********
TO A DIFFERENT FILE!!
*********** */
void TimetToFileTime(time_t t, LPFILETIME pft )
{
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD) ll;
    pft->dwHighDateTime = ll >>32;
}

static int
MirrorGetFileInformation(
	LPCWSTR							FileName,
	LPBY_HANDLE_FILE_INFORMATION	HandleFileInformation,
	PDOKAN_FILE_INFO				DokanFileInfo)
{
	TRACE(INFO,L"MirrorGetFileInformation: %s", FileName);

	PFILE_INFO_NODE node = findFile(authData,FileName);
	
	if (node->info->attributes == ATT_DIRECTORY) {
		HandleFileInformation->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	} else {
		HandleFileInformation->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	}

	time_t date;
	time(&date);
	FILETIME wTime;
	TimetToFileTime(date, &wTime);

	HandleFileInformation->ftCreationTime = wTime;
	HandleFileInformation->ftLastAccessTime = wTime;
	HandleFileInformation->ftLastWriteTime = wTime;
	HandleFileInformation->nFileSizeHigh = 0;
	HandleFileInformation->nFileSizeLow = node->info->size;

	return 0;
}


static int
MirrorFindFiles(
	LPCWSTR				FileName,
	PFillFindData		FillFindData, // function pointer
	PDOKAN_FILE_INFO	DokanFileInfo) {

	TRACE(INFO,L"MirrorFindFiles: %s", FileName);

	PFILE_INFO_NODE infoNode = (PFILE_INFO_NODE) DokanFileInfo->Context;
	WIN32_FIND_DATAW data;

	if ( !wcscmp(infoNode->info->id.c_str(),L"\\") ) {
		PFILE_INFO_NODE list = listRoot(authData);
		
		PFILE_INFO_NODE node = list; // start from the beginning

		while (node != NULL) {
			TRACE(DEBUG,L"Tenemos nodo!!!");		
			TRACE(DEBUG,L"Name: %s!!!",node->info->name);		

			//mbstowcs(data.cFileName,node->info->name,MB_CUR_MAX);
			//mbstowcs(data.cAlternateFileName,node->info->name,strlen(node->info->name));

			//data.cbFileName = 

			wcscpy(data.cFileName,node->info->name.c_str());

			TRACE(DEBUG,L"Inserting data: %s",data.cFileName);

			if (node->info->attributes == ATT_DIRECTORY)
				data.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
			else
				data.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

			data.nFileSizeHigh = 0;
			data.nFileSizeLow = node->info->size;

			wchar_t* buffer = new wchar_t[512];
			wcscpy(buffer, node->info->id.c_str());
			data.dwReserved1 = (DWORD) buffer;			
			FillFindData(&data,DokanFileInfo);

			node = node->next;		
			CTree<PFILE_INFO_NODE>* treeNode = new CTree<PFILE_INFO_NODE>();
			treeNode->item = node;
			tree.children.push_back(treeNode);

			//count++;
		} 
	} else {
			PFILE_INFO_NODE list = listFolder(authData, infoNode->info->id);
			PFILE_INFO_NODE node = list; // start from the beginning

			while (node != NULL) {
				TRACE(DEBUG,L"Tenemos nodo!!!");		
				TRACE(DEBUG,L"Name: %s!!!",node->info->name);		

				//mbstowcs(data.cFileName,node->info->name,MB_CUR_MAX);
				//mbstowcs(data.cAlternateFileName,node->info->name,strlen(node->info->name));

				//data.cbFileName = 

				wcscpy(data.cFileName,node->info->name.c_str());

				TRACE(DEBUG,L"Inserting data: %s",data.cFileName);

				if (node->info->attributes == ATT_DIRECTORY) {
					data.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
				} else {
					data.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
				}

				data.nFileSizeHigh = 0;
				data.nFileSizeLow = node->info->size;

				wchar_t* buffer = new wchar_t[512];
				wcscpy(buffer, node->info->id.c_str());
				data.dwReserved1 = (DWORD) buffer;			
				FillFindData(&data,DokanFileInfo);

				node = node->next;		
				CTree<PFILE_INFO_NODE>* treeNode = new CTree<PFILE_INFO_NODE>();
				treeNode->item = node;
				tree.children.push_back(treeNode);

				//count++;
			} 
	}

	return 0;
}


static int
MirrorDeleteFile(LPCWSTR FileName, PDOKAN_FILE_INFO	DokanFileInfo) {
	TRACE(INFO,L"MirrorDeleteFile: %s", FileName);

	return 0;
}


static int
MirrorDeleteDirectory(LPCWSTR FileName, PDOKAN_FILE_INFO DokanFileInfo) {
	TRACE(INFO,L"MirrorDeleteDirectory: %s", FileName);
	
	return 0;
}


static int
MirrorMoveFile(LPCWSTR FileName, LPCWSTR NewFileName, BOOL ReplaceIfExisting, PDOKAN_FILE_INFO DokanFileInfo) {

	TRACE(INFO,L"MirrorMoveFile: %s", FileName);

	return 0;
}


static int
MirrorLockFile(
	LPCWSTR				FileName,
	LONGLONG			ByteOffset,
	LONGLONG			Length,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	HANDLE	handle;
	LARGE_INTEGER offset;
	LARGE_INTEGER length;

	TRACE(INFO,L"LockFile %s",FileName);	

	return 0;
}


static int
MirrorSetEndOfFile(
	LPCWSTR				FileName,
	LONGLONG			ByteOffset,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR			filePath[MAX_PATH];
	HANDLE			handle;
	LARGE_INTEGER	offset;

	TRACE(INFO,L"MirrorSetEndOfFile %s",FileName);	
	return 0;
}


static int
MirrorSetFileAttributes(
	LPCWSTR				FileName,
	DWORD				FileAttributes,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	
	TRACE(INFO,L"MirrorSetFileAttributes %s",FileName);	

	return 0;
}


static int
MirrorSetFileTime(
	LPCWSTR				FileName,
	CONST FILETIME*		CreationTime,
	CONST FILETIME*		LastAccessTime,
	CONST FILETIME*		LastWriteTime,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	TRACE(INFO,L"MirrorSetFileTime %s",FileName);	

	return 0;
}



static int
MirrorUnlockFile(
	LPCWSTR				FileName,
	LONGLONG			ByteOffset,
	LONGLONG			Length,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	DbgPrint(L"UnlockFile %s\n", FileName);

	return 0;
}


static int
MirrorUnmount(
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	DbgPrint(L"Unmount\n");
	return 0;
}


static DOKAN_OPERATIONS
dokanOperations = {
	MirrorCreateFile,
	MirrorOpenDirectory,
	MirrorCreateDirectory,
	MirrorCleanup,
	MirrorCloseFile,
	MirrorReadFile,
	MirrorWriteFile,
	MirrorFlushFileBuffers,
	MirrorGetFileInformation,
	MirrorFindFiles,
	NULL, // FindFilesWithPattern
	MirrorSetFileAttributes,
	MirrorSetFileTime,
	MirrorDeleteFile,
	MirrorDeleteDirectory,
	MirrorMoveFile,
	MirrorSetEndOfFile,
	MirrorLockFile,
	MirrorUnlockFile,
	NULL, // GetDiskFreeSpace
	NULL, // GetVolumeInformation
	MirrorUnmount // Unmount
};



int __cdecl
main(ULONG argc, PCHAR argv[])
{
	int status;
	ULONG command;
	PDOKAN_OPTIONS dokanOptions = (PDOKAN_OPTIONS)malloc(sizeof(DOKAN_OPTIONS));

	initTrace(L"c:\\log.txt");

	if (argc < 5) {
		fprintf(stderr, "mirror.exe\n"
			"  /r RootDirectory (ex. /r c:\\test)\n"
			"  /l DriveLetter (ex. /l m)\n"
			"  /t ThreadCount (ex. /t 5)\n"
			"  /d (enable debug output)\n"
			"  /s (use stderr for output)");
		return -1;
	}

	g_DebugMode = FALSE;
	g_UseStdErr = FALSE;

	ZeroMemory(dokanOptions, sizeof(DOKAN_OPTIONS));
	dokanOptions->ThreadCount = 0; // use default

	for (command = 1; command < argc; command++) {
		switch (tolower(argv[command][1])) {
		case 'r':
			command++;
			mbstowcs(RootDirectory, argv[command], strlen(argv[command]));
			DbgPrint(L"RootDirectory: %ls\n", RootDirectory);
			break;
		case 'l':
			command++;
			dokanOptions->DriveLetter = argv[command][0];
			break;
		case 't':
			command++;
			dokanOptions->ThreadCount = (USHORT)atoi(argv[command]);
			break;
		case 'd':
			g_DebugMode = TRUE;
			break;
		case 's':
			g_UseStdErr = TRUE;
			break;
		default:
			fprintf(stderr, "unknown command: %s\n", argv[command]);
			return -1;
		}
	}

	dokanOptions->DebugMode = (UCHAR)g_DebugMode;
	dokanOptions->UseStdErr = (UCHAR)g_UseStdErr;
	dokanOptions->UseKeepAlive = 1;

	authData = loadData();

	status = DokanMain(dokanOptions, &dokanOperations);
	switch (status) {
		case DOKAN_SUCCESS:
			fprintf(stderr, "Success\n");
			break;
		case DOKAN_ERROR:
			fprintf(stderr, "Error\n");
			break;
		case DOKAN_DRIVE_LETTER_ERROR:
			fprintf(stderr, "Bad Drive letter\n");
			break;
		case DOKAN_DRIVER_INSTALL_ERROR:
			fprintf(stderr, "Can't install driver\n");
			break;
		case DOKAN_START_ERROR:
			fprintf(stderr, "Driver something wrong\n");
			break;
		case DOKAN_MOUNT_ERROR:
			fprintf(stderr, "Can't assign a drive letter\n");
			break;
		default:
			fprintf(stderr, "Unknown error: %d\n", status);
			break;
	}

	return 0;
}

