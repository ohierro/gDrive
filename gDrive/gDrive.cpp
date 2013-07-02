#ifndef __G_DRIVE_H__
#define __G_DRIVE_H__

PAUTH_DATA registerApp() {
	string code;

	cout << "Registering application..." << endl;
	system("explorer \"register.html\"");
	cout << "Insert auth code here: ";
	cin >> code;

	wstring wcode = char2wide(code.c_str());
	cout << "Code: " << code;
	return registerApp(wcode);
}

time_t parseTime(std::wstring& date) {
	int day,month,year,hour,minute,second;
	time_t rawTime;
	time(&rawTime);
	tm* tm = localtime(&rawTime);

	sscanf(wide2char(date),"%d/%d/%d - %d:%d:%d", &month, &day, &year, &hour, &minute, &second);

	tm->tm_mon = month - 1;
	tm->tm_year = year + 100;
	tm->tm_mday = day;
	
	tm->tm_hour = hour;
	tm->tm_min = minute;
	tm->tm_sec = second;
		
	tm->tm_isdst = -1;

	time_t requestTime = mktime(tm);
	return requestTime;
}

bool checkValidCredentials(std::wstring authRequest, long expires) {
	
	time_t rawTime;
	time(&rawTime);
	tm* tm = localtime(&rawTime);
	

	//strptime(authRequest,"%x - %X",&tm);
	int day,month,year,hour,minute,second;
	sscanf(wide2char(authRequest),"%d/%d/%d - %d:%d:%d", &month, &day, &year, &hour, &minute, &second);
	tm->tm_mon = month - 1;
	tm->tm_year = year + 100;
	tm->tm_mday = day;
	
	tm->tm_hour = hour;
	tm->tm_min = minute;
	tm->tm_sec = second;
		
	tm->tm_isdst = -1;

	time_t requestTime = mktime(tm);
	

	time_t currentTime;
	gmtime(&currentTime);

	if (difftime(currentTime,requestTime) > expires) {
		return false;
	}
	
	return true;	
}

PAUTH_DATA loadData() {
	Preferences preferences;
	std::map<std::wstring, std::wstring>* data = preferences.loadData();

	std::wstring accessToken = (*data)[L"ACCESS_TOKEN"];
	std::wstring refreshToken = (*data)[L"REFRESH_TOKEN"];
	std::wstring tokenType = (*data)[L"TOKEN_TYPE"];
	std::wstring expires = (*data)[L"EXPIRES"];
	std::wstring authRequest = (*data)[L"AUTH_REQUEST"];	
	
	int iExpires = atoi(wide2char(expires.c_str()));	

	PAUTH_DATA authData = new AUTH_DATA();
	authData->accessToken = accessToken;
	authData->refreshToken = refreshToken;
	authData->tokenType = tokenType;
	authData->expires = iExpires;
	authData->requestTime = parseTime(authRequest);

	if (!checkValidCredentials(authRequest,iExpires)) {
		// renew credentials
		PAUTH_DATA newAuthData = renewAuth(refreshToken);

		// save new data....
		std::map<std::wstring, std::wstring> data;
		char buffer[100];
		_itoa_s(newAuthData->expires,buffer,100,10);
	
		data[L"ACCESS_TOKEN"] = newAuthData->accessToken;	
		data[L"REFRESH_TOKEN"] = newAuthData->refreshToken;
		data[L"TOKEN_TYPE"] = newAuthData->tokenType;

		_itoa_s(newAuthData->expires,buffer,100,10);
		data[L"EXPIRES"] = char2wide(buffer);

		tm* tm = gmtime(&(newAuthData->requestTime));
		strftime(buffer, 100, "%x - %X", tm);
		data[L"AUTH_REQUEST"] = char2wide(buffer);	

		preferences.saveData(data);
		return newAuthData;
	}

	return authData;
}

#endif // __G_DRIVE_H__