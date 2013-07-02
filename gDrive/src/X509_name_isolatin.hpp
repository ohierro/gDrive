#ifndef __X509_NAME_ISOLATIN__
#define __X509_NAME_ISOLATIN__

#ifdef _X509_ISO_LATIN_

#ifdef WIN32
#include <WINDOWS.H>
#endif
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#ifdef __cplusplus
extern "C" {
#endif

char *ISOLatin(void *pX509Name,char *szName,int iLen);

#ifdef __cplusplus
}
#endif

#define X509_NAME_oneline ISOLatin

#endif


#endif
