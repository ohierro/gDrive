#ifndef __STRING_CONVERSIONS_H__
#define __STRING_CONVERSIONS_H__

//void wide2char(CONST WCHAR* wide, char** result);
//BOOL char2wide(CONST CHAR *in_Src, WCHAR *out_Dst, INT in_MaxLen);
#include <string>
using namespace std;

char *
wide2char(const wstring _wide_str);

wstring
char2wide(const char *_char_str);

wstring replace(const wstring& orig,const wstring& fnd, const wstring& repl);

#endif