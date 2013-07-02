#include "stdafx.h"

#include "ResponseParser.h"
#include "jansson.h"
#include "StringConversions.h"
#include "mycomm.h"

PFILE_INFO_NODE ResponseParser::parseListRoot(char* json) {
	json_error_t error;
	json_t* root = json_loads(json,0, &error);

	if(!root)
	{
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		//return 1;
	}

	json_t* items = json_object_get(root,"items");
	
	if(!json_is_array(items))
	{
		fprintf(stderr, "error: items is not an array\n");
	    //return 1;
	}

	PFILE_INFO_NODE start,current,end;	
	start = current = end = NULL;

	for(int i = 0; i < json_array_size(items); i++)
	{
		json_t* data = json_array_get(items, i);
		json_t* parents = json_object_get(data, "parents");
		PFILE_INFO_NODE node = NULL;
	
		if (parents != NULL) {
			json_t* parent = json_array_get(parents,0);

			json_t* is_root_json = json_object_get(parent,"isRoot");

			if ( is_root_json == json_true()) {				
				PFILE_INFO info = parseItem(data);
				if (info != NULL) {
					PFILE_INFO_NODE node = new FILE_INFO_NODE();
					node->info = info;
					node->next = NULL;

					if (start == NULL) {
						start = end = current = node;
					} else {
						current->next = node;
						end = node;

						current = current->next;
					}
				}
			}
		}
	}

	return start;
}

PFILE_INFO_NODE ResponseParser::parseListFolder(char* json) {

	json_error_t error;
	json_t* root = json_loads(json,0, &error);

	if(!root)
	{
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		//return 1;
	}

	json_t* items = json_object_get(root,"items");
	
	if(!json_is_array(items))
	{
		fprintf(stderr, "error: items is not an array\n");
	    //return 1;
	}

	PFILE_INFO_NODE start,current,end;	
	start = current = end = NULL;

	for(int i = 0; i < json_array_size(items); i++) {
		json_t* data = json_array_get(items, i);
		json_t* parents = json_object_get(data, "parents");

		PFILE_INFO_NODE node = NULL;
	
		if (parents != NULL) {			
			PFILE_INFO info = parseItem(data);

			if (info != NULL) {
				node = new FILE_INFO_NODE();
				node->info = info;
				node->next = NULL;

				if (start == NULL) {
					start = end = current = node;
				} else {
					current->next = node;
					end = node;

					current = current->next;
				}
			}
		}
	}

	return start;
}

PGAUTH_DATA ResponseParser::parseAuthData(char* json) {
	
	json_error_t error;
	json_t* root = json_loads(json,0, &error);

	if(!root)
	{
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		//return 1;
	}

	json_t* accessToken = json_object_get(root,"access_token");
	json_t* expires = json_object_get(root,"expires_in");
	json_t* tokenType = json_object_get(root,"token_type");
	json_t* refreshToken = json_object_get(root,"refresh_token");	

	PGAUTH_DATA data = new GAUTH_DATA();
	
	data->accessToken = char2wide(json_string_value(accessToken));
	data->expires = json_integer_value(expires);
	data->tokenType = char2wide(json_string_value(tokenType));
	if (json_string_value(refreshToken)) {
		data->refreshToken = char2wide(json_string_value(refreshToken));
	}

	return data;
}

PFILE_INFO ResponseParser::parseItem(json_t *item) {
	
    json_t* title = json_object_get(item, "title");
	json_t* mimeType = json_object_get(item, "mimeType");
	json_t* id = json_object_get(item, "id");
	json_t* fileSize = json_object_get(item, "fileSize");
	json_t* originalFileName = json_object_get(item,"originalFilename");
	json_t* downloadUrl = json_object_get(item,"downloadUrl");
	
	json_t* labels = json_object_get(item,"labels");
	json_t* trashed = json_object_get(labels,"trashed");

	if (json_is_true(trashed)) {
		return NULL;
	}

	json_t* parents = json_object_get(item, "parents");
	PFILE_INFO_NODE node = NULL;

	if (parents != NULL) {
		PGFILE_INFO info = new GFILE_INFO();

		info->id = char2wide(json_string_value(id));

		if (originalFileName != NULL) {
			info->name = char2wide(json_string_value(originalFileName));/*char2wide(json_string_value(title));*/
		} else {
			info->name = char2wide(json_string_value(title));
		}

		if (fileSize != NULL) {
			info->size = atoi(json_string_value(fileSize));
		} else {
			info->size = 0;
		}

		if (downloadUrl != NULL) {
			info->downloadUrl = char2wide(json_string_value(downloadUrl));
		} else {
			json_t* exportUrls = json_object_get(item,"exportLinks");
			
			void* iter = json_object_iter(exportUrls);

			while ( iter ) {
				wstring key = char2wide(json_object_iter_key(iter));
				wstring value = char2wide(json_string_value(json_object_iter_value(iter)));

				info->exportUrls[key] = value;
				iter = json_object_iter_next(exportUrls, iter);
			}			
		}

		if (strcmp(json_string_value(mimeType),"application/vnd.google-apps.folder") == 0) {
			// FOLDER
			info->attributes = ATT_DIRECTORY;
		} 
	
		return info;
	}

	return NULL;
}