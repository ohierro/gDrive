#include "ctree.h"

#include "StringConversions.h"
#include <iostream>

using namespace std;

wstring getFolder(const wstring fileName, int level) {
	wstring path = fileName;

	if (path[0] == '\\') {
		path = fileName.substr(1,fileName.length()-1);
	} 

	if (level > 0 ) {
		std::size_t pos;

		while (level > 0) {
			pos = path.find('\\');
			path = path.substr(pos+1, path.length()-pos-1);
			level--;
		}

		pos = path.find('\\');
		if (pos != std::string::npos) {
			return path.substr(0,pos);
		} else {
			return path;
		}
	} else {
		std::size_t pos = path.find('\\');
		return path.substr(0,pos);
	}
}

int getFolderLevels(const wstring fileName) {
	wstring path = fileName;

	if (path[0] == '\\') {
		path = fileName.substr(1,fileName.length()-1);
	} 

	size_t pos = path.find('\\');
	int levels = 0;

	while (pos != std::string::npos) {		
		path = path.substr(pos+1, path.length()-pos-1);
		pos = path.find('\\');
		levels++;
	}

	return levels;
}

wstring parsePath(const std::wstring fileName) {
	int levels = getFolderLevels(fileName);
	cout << wide2char(fileName) << " has " << levels << " levels.." << endl;

	for (int i=0; i<=levels; i++) {
		wstring folder = getFolder(fileName,i);
		cout << "Level " << i << ": " << wide2char(folder) << endl;
	}

	return L"error";
}