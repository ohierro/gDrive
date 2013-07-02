#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN             // Excluir material rara vez utilizado de encabezados de Windows

// Archivos de encabezado de Windows:
#include <windows.h>

#include "StringConversions.h"

char * 
wide2char(const wstring _wide_str)
{
  int size = wcstombs(0, _wide_str.c_str(), 0) + 1;
  if (!size) return 0;

  char *char_str = new char[size];
  if (!char_str) return 0;

  WideCharToMultiByte(CP_ACP, 0, _wide_str.c_str(), -1, char_str, size, 0, 0);
  char_str[size-1] = 0; // make sure that string is properly ended
  return char_str;
}

wstring
char2wide(const char *_char_str)
{
  int size = mbstowcs(0, _char_str, 0) + 1;
  if (!size) return 0;

  wchar_t *wide_str = new wchar_t[size];
  if (!wide_str) return 0;

  MultiByteToWideChar(CP_ACP, 0, _char_str, -1, wide_str, size);
  wide_str[size-1] = 0; // make sure that string is properly ended
  return wide_str;
}

wstring replace(const wstring& orig,const wstring& fnd, const wstring& repl)
{
    wstring ret = orig;
    size_t pos = 0;

    while(true)
    {
        pos = ret.find(fnd,pos);
        if(pos == wstring::npos)  // no more instances found
            break;

        ret.replace(pos,fnd.size(),repl);  // replace old string with new string
        pos += repl.size();
    }

    return ret;
}