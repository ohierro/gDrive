#include "Preferences.h"

#include <stdio.h>
#include <iostream>
#include "sqlite3.h"
#include "StringConversions.h"


static int callback(void *dataMap, int argc, char **argv, char **azColName)
{
	//NotUsed = 0;
	int i;
	std::map<std::wstring, std::wstring>* data = (std::map<std::wstring, std::wstring>*) dataMap;
	for (i = 0; i < argc; i++)
	{
		//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}

	std::pair<std::wstring, std::wstring> item;
	item.first = char2wide(argv[0]);
	item.second = char2wide(argv[1]);

	data->insert(item);
	printf("\n");
	return 0;
}

void Preferences::saveData(std::map<std::wstring, std::wstring>& data) {
	sqlite3 *db;
	char *zErrMsg = 0;

	int rc = sqlite3_open("preferences.db", &db);

	if( rc ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		//return(1);   
	}
	
	sqlite3_exec(db, "DELETE FROM PREFERENCES;", NULL, NULL, &zErrMsg);

	std::map<std::wstring, std::wstring>::iterator it;
	for (it = data.begin(); it != data.end(); ++it) {
		std::string sql = "INSERT INTO PREFERENCES(NAME, VALUE) values ('";
		sql += wide2char(it->first.c_str());
		sql += "','";
		sql += wide2char(it->second.c_str());
		sql += "');";

		sqlite3_exec(db, sql.c_str(), &callback, &data, &zErrMsg);
	}
	
	sqlite3_close(db);
}

std::map<std::wstring, std::wstring>* Preferences::loadData() {
	sqlite3 *db;
	char *zErrMsg = 0;

	int rc = sqlite3_open("preferences.db", &db);

	if( rc ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		//return(1);   
	}
	
	std::map<std::wstring, std::wstring>* data = new std::map<std::wstring, std::wstring>();
	sqlite3_exec(db, "SELECT * FROM PREFERENCES;", &callback, data, &zErrMsg);

	std::map<std::wstring, std::wstring>::iterator it;
	for (it = data->begin(); it != data->end(); ++it) {
		cout << wide2char(it->first.c_str()) << ": " << wide2char(it->second.c_str()) << endl;
	}
	
	sqlite3_close(db);
	return data;
}