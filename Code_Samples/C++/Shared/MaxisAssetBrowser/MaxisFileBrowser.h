// MaxisFileBrowser.h: interface for the MaxisFileBrowser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAXISFILEBROWSER_H__96E1082E_2165_4D4B_BA29_76524D3E0236__INCLUDED_)
#define AFX_MAXISFILEBROWSER_H__96E1082E_2165_4D4B_BA29_76524D3E0236__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "SelectionList.h"


class MaxisFileBrowser  
{
	SelectionList _dirs;
	SelectionList _files;

public:
	bool SelectDir( const string &dir, const string &filter = string());

	DWORD GetDirCount() const;
	string GetFirstDir();
	string GetNextDir();

	DWORD GetFilesCount() const;
	string GetFirstFile();
	string GetNextFile();

	static string ConvertFileNameToSrf( const string &filePath );
// remove these, get from FileUtils
	static bool DirExists( const string &dirPath );
	static bool FileExists( const string &filePath );
	static string MakeWinPath( const string &unixPath );
	static string MakeUnixPath( const string &winPath );
// end remove
	static bool IsValidFilter( const string &filter );

	static bool Test();
};

#endif // !defined(AFX_MAXISFILEBROWSER_H__96E1082E_2165_4D4B_BA29_76524D3E0236__INCLUDED_)
