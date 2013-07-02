#ifndef __RESPONSE_PARSER_H__
#define __RESPONSE_PARSER_H__

#include "mycomm.h"
#include "jansson.h"

class ResponseParser {
public:
	PFILE_INFO_NODE parseListRoot(char* json);
	PFILE_INFO_NODE parseListFolder(char* json);
	PGAUTH_DATA parseAuthData(char* json);

private:
	PFILE_INFO parseItem(json_t* item);
};

#endif