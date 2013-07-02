#ifndef __C_TREE_ITEM_H__
#define __C_TREE_ITEM_H__

#include <vector>
#include <string>

template<class CItem> class CTree {
public:
	CItem item;

	std::vector<CTree*> children;
};

std::wstring getFolder(const std::wstring fileName, int level);
int getFolderLevels(const std::wstring fileName);
std::wstring parsePath(const std::wstring fileName);

#endif // __C_TREE_ITEM_H__
