#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include <map>
#include <string>

class Preferences {

public:
	void saveData(std::map<std::wstring, std::wstring>& data);
	std::map<std::wstring, std::wstring>* loadData();
};


#endif  //__PREFERENCES_H__