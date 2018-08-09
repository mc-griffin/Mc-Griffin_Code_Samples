// FileUtils.h
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEUTILS_H__CA34FB1E_2871_4373_B84E_48D3E2622B99__INCLUDED_)
#define AFX_FILEUTILS_H__CA34FB1E_2871_4373_B84E_48D3E2622B99__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <windows.h>

// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>
#include <vector>

using namespace std;


extern bool FileExists( const string &path );
extern bool DirExists( const string &dirPath );
extern DWORD GetTheFileSize( const string &path );
extern bool DeleteTheFile( const string &path );
extern bool SplitFilePath( const string &filePath,
					string *dir, string *fileNameSansExt, string *fileExt );
extern bool HasFileExtension( const string &fileName );
extern string GetFileExtension( const string &fileName );
extern bool HasEndingBackSlash( const string &fileName );
extern string MakeWinPath( const string &unixPath );
extern string MakeUnixPath( const string &winPath );

extern DWORD GetWildCardFileNameSearch( const string &wildCard,
					const string &dirPath, vector<string> *fileNames );

// a general utility function that brings up a Save As dialog box
extern string SaveAsSelectFilePath( const string &filePath, const string &title,
										HINSTANCE inst, HWND parent );

// delete a file from the browser cache
bool DeleteFileFromCache( LPCSTR url );

// If you can't open the file (probably because an application has it
// open), then make a copy in the temp directory, and open that.
HANDLE OpenTheFile( const string &filePath, string *tempFilePath );


#endif // !defined(AFX_FILEUTILS_H__CA34FB1E_2871_4373_B84E_48D3E2622B99__INCLUDED_)
