#ifndef __TRACE_H__
#define __TRACE_H__

#include <wchar.h>

#define FATAL	0
#define T_ERROR	1
#define WARNING 2
#define INFO	3
#define DEBUG	4

void initTrace(wchar_t* path);
//void trace(int level, const wchar_t* message);
void trace(int level, const wchar_t* message, ...);


#define TRACE trace

#endif