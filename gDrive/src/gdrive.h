#ifndef __G_DRIVE_H__
#define __G_DRIVE_H__

#include "comm.h"
#include "ctree.h"

#include <string>

PFILE_INFO_NODE findNode(CTree<PFILE_INFO_NODE> tree, std::wstring fileName);

PFILE_INFO_NODE findFile(PAUTH_DATA authData, std::wstring folderPath);

PAUTH_DATA loadData();	

#endif //__G_DRIVE_H__