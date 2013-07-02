#ifndef __COMM_H__
#define __COMM_H__

// El siguiente bloque ifdef muestra la forma estándar de crear macros que facilitan 
// la exportación de archivos DLL. Todos los archivos de este archivo DLL se compilan con el símbolo COMM_EXPORTS
// definido en la línea de comandos. Este símbolo no se debe definir en ningún proyecto
// que utilice este archivo DLL. De este modo, otros proyectos cuyos archivos de código fuente incluyan el archivo 
// interpretan que las funciones COMM_API se importan de un archivo DLL, mientras que este archivo DLL interpreta los símbolos
// definidos en esta macro como si fueran exportados.
#ifdef COMM_EXPORTS
#define COMM_API __declspec(dllexport)
#else
#define COMM_API __declspec(dllimport)
#endif

#include <windows.h>
#include <string>
#include <time.h>

/* API */
// CONSTANTS
#define ATT_DIRECTORY 0x0002

// TYPES
// FILE_INFO STRUCTURE
typedef struct file_info {
	std::wstring id;
	std::wstring name;
	long size;
	int attributes;
} FILE_INFO, *PFILE_INFO;

typedef struct file_info_node {
	PFILE_INFO info;
	file_info_node* next;
} FILE_INFO_NODE, *PFILE_INFO_NODE;

// AUTH_DATA
typedef struct auth_data {
} AUTH_DATA, *PAUTH_DATA;

// METHODS
COMM_API PFILE_INFO_NODE init(PAUTH_DATA authData, WCHAR* configPath);

COMM_API PFILE_INFO_NODE listRoot(PAUTH_DATA authData);

COMM_API PFILE_INFO_NODE listFolder(PAUTH_DATA authData, std::wstring& name);

//COMM_API FILE* getFile(PAUTH_DATA authData, std::wstring& file);

COMM_API FILE* getFile(PAUTH_DATA authData, PFILE_INFO_NODE file);

COMM_API PAUTH_DATA registerApp(std::wstring& code);

COMM_API PAUTH_DATA renewAuth(std::wstring& refreshCode);

#endif // __COMM_H__