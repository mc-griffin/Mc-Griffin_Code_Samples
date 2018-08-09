// MaxisAssetBrowser.h: interface for the MaxisAssetBrowser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAXISASSETBROWSER_H__2E9FD6D2_352F_4C41_B6C6_9DD3E388A68F__INCLUDED_)
#define AFX_MAXISASSETBROWSER_H__2E9FD6D2_352F_4C41_B6C6_9DD3E388A68F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "MaxisFileBrowser.h"


template <class T>
class MaxisAssetBrowser // : public MaxisFileBrowser
{
protected:
	MaxisFileBrowser _browser;
	SelectionList _assets;

public:
	bool SelectDir( const string &dir );

	DWORD GetDirCount() const;
	string GetFirstDir();
	string GetNextDir();

	DWORD GetFileCount() const;
	string GetFirstFile();
	string GetNextFile();
};

#endif // !defined(AFX_MAXISASSETBROWSER_H__2E9FD6D2_352F_4C41_B6C6_9DD3E388A68F__INCLUDED_)
