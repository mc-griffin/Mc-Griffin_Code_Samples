// SimCity4LotBrowser.h: interface for the SimCity4LotBrowser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMCITY4LOTBROWSER_H__A42AAAEF_D766_4D50_AFC3_57EFF665C2F5__INCLUDED_)
#define AFX_SIMCITY4LOTBROWSER_H__A42AAAEF_D766_4D50_AFC3_57EFF665C2F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SelectionList.h"
#include <list>


class SimCity4LotBrowser  
{
protected:
	SelectionList _dirs;
	SelectionList _files;
	string _selectedDir;

public:
	bool SelectDir( const string &dir );
	string GetSelectedDir() const { return _selectedDir; }

	DWORD GetDirCount() const;
	string GetFirstDir();
	string GetNextDir();

	DWORD GetFileCount() const;
	string GetFirstFile();
	string GetNextFile();

	static bool IsValidFilter( const string &filter );

protected:
	bool SelectDir( const string &dir, const string &filter, list<string> *files );
};

#endif // !defined(AFX_SIMCITY4LOTBROWSER_H__A42AAAEF_D766_4D50_AFC3_57EFF665C2F5__INCLUDED_)
