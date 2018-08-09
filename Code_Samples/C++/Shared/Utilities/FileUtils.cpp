// FileUtils.cpp
//
//////////////////////////////////////////////////////////////////////

#include "FileUtils.h"
#include <assert.h>
#include <wininet.h>


// improve SplitFilePath( filePath, drive, dir, fileNameSansExt, fileExt )
bool SplitFilePath( const string &filePath,
		string *dir, string *fileNameSansExt, string *fileExt )
{
	assert( dir || fileNameSansExt || fileExt );

	if ( filePath.empty())
	{
		assert(false);
		return false;
	}

	int rightBackSlash = filePath.rfind( "\\" );
	if ( rightBackSlash == string::npos ) { assert(false); return false; }

	if ( dir )
	{
		*dir = filePath.substr( 0, rightBackSlash );
		assert( !dir->empty() && !HasEndingBackSlash( *dir ));
	}

	DWORD fileNameStarts = rightBackSlash + 1;
	if ( filePath.length() <= fileNameStarts ) { assert(false); return false; }

	string fileName = filePath.substr( fileNameStarts );
	assert( !fileName.empty());

	int extensionStarts = fileName.rfind( "." );

	if ( fileNameSansExt )
	{
		if ( extensionStarts < 0 )
			extensionStarts = fileName.size();
		*fileNameSansExt = fileName.substr( 0, extensionStarts );
		assert( !fileNameSansExt->empty());
	}

	if ( fileExt )
	{
		if ( extensionStarts < 0 )
			*fileExt = "";
		else
			*fileExt = fileName.substr( extensionStarts + 1 );
	}

	return true;
}


// if fileName has pattern *.+ then true, false otherwise
// ('*'=any number of characters, '+' is at least one character)
// Hence fileName must have at least 2 characters ('.' followed by at least one).
bool HasFileExtension( const string &fileName )
{
	if ( fileName.length() < 2 ) return false;
	DWORD dot = fileName.rfind( "." );
	if ( dot == string::npos ) return false;
	DWORD fileExtStarts = dot + 1;
	bool wellFormed = fileExtStarts < fileName.length();
	return wellFormed;
}


string GetFileExtension( const string &fileName )
{
	if ( !HasFileExtension( fileName ))
		return "";
	DWORD dot = fileName.rfind( "." );
	if ( dot == string::npos ) return false;
	DWORD fileExtStarts = dot + 1;
	return fileName.substr( fileExtStarts );
}


bool HasEndingBackSlash( const string &fileName )
{
	if ( fileName.empty()) return false;
	return fileName.at( fileName.length() - 1 ) == '\\';
}


bool FileExists( const string &path )
{
	assert( !path.empty() && path.length() < MAX_PATH );
	DWORD fileAttributes = GetFileAttributes( path.c_str());
	return fileAttributes != 0xFFFFFFFF;
}


DWORD GetTheFileSize( const string &path )
{
    HANDLE hFile = CreateFile( path.c_str(), GENERIC_READ, 0, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) { assert(false); return 0; }

	DWORD fileSize = GetFileSize( hFile, NULL );
	CloseHandle( hFile );

	return fileSize;
}


bool DeleteTheFile( const string &path )
{
	// don't bother to check path
	BOOL del = DeleteFile( path.c_str()); assert( del );
	return BOOL( del ) == TRUE;
}


bool DirExists( const string &path )
{
	DWORD a = GetFileAttributes( path.c_str());
	if ( a == 0xFFFFFFFF ) return false;
	if (( a & FILE_ATTRIBUTE_DIRECTORY ) != FILE_ATTRIBUTE_DIRECTORY )
		return false;
	return true;
}


string MakeWinPath( const string &unixPath )
{
	if ( unixPath.empty()) return unixPath;
	
	string winPath;
	for ( int i = 0; i < unixPath.size(); ++i )
	{
		const char c = unixPath.at( i );
		if ( c == '/' )
			winPath.append( 1, '\\' );
		else
			winPath.append( 1, c );
	}
	return winPath;
}


string MakeUnixPath( const string &winPath )
{
	if ( winPath.empty()) return winPath;

	string unixPath;
	for ( int i = 0; i < winPath.size(); ++i )
	{
		const char c = winPath.at( i );
		if ( c == '\\' )
			unixPath.append( 1, '/' );
		else
			unixPath.append( 1, c );
	}
	return unixPath;
}


DWORD GetWildCardFileNameSearch( const string &wildCard,
					const string &dirPath, vector<string> *filePaths )
{
	if ( wildCard.empty() || dirPath.empty() || filePaths == NULL )
		{ assert(false); return 0; }

	if ( !DirExists( dirPath ))
		{ assert(false); return 0; }

	string searchOnThis = dirPath;
	if ( !HasEndingBackSlash( searchOnThis ))
		searchOnThis += "\\";
	searchOnThis += wildCard;

	HANDLE search = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA fd = {0};

	search = FindFirstFile( searchOnThis.c_str(), &fd );
	if ( search == INVALID_HANDLE_VALUE )
		return 0;

	DWORD count = 0;
	do
	{
		if (( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
		{
			string filePath = dirPath;
			if ( !HasEndingBackSlash( filePath ))
				filePath += "\\";
			filePath += fd.cFileName;

			filePaths->push_back( fd.cFileName );
			++count;
			assert( FileExists( filePath ));
		}
	}
	while ( FindNextFile( search, &fd ) == TRUE );
	FindClose( search );

	return count;
}


// Put up a Save As dialog box and return the path of the selected file.
// File extension filter is derived from the defaultFilePath along with *.*.
string SaveAsSelectFilePath( const string &defaultFilePath, const string &title,
								HINSTANCE inst, HWND parent )
{
	const WORD maxTitleLen = 128; // 128 by fiat

	if ( inst == NULL || title.empty() || maxTitleLen < title.length())
		{ assert(false); return ""; }

	string dir;
	string fileName;
	string fileExt;
	if ( !defaultFilePath.empty())
		SplitFilePath( defaultFilePath, &dir, &fileName, &fileExt );

	// set filter
	char filter[32] = "";
	{
		int i = 0;
		if ( !fileExt.empty())
		{
			strcpy( filter, fileExt.c_str());
			i = fileExt.length();
			strcpy( filter + i, " (*." );
			i += 4;
			strcpy( filter + i, fileExt.c_str());
			i += fileExt.length();
			filter[ i++ ] = ')';			
			filter[ i++ ] = '\0';
			strcpy( filter + i, "*." );
			i += 2;
			strcpy( filter + i, fileExt.c_str());
			i += fileExt.length();
			filter[ i++ ] = '\0';
		}
		// "All (*.*)\0*.*\0"
		strcpy( filter + i, "All (*.*)" );
		i += 9;
		filter[ i++ ] = '\0';
		strcpy( filter + i, "*.*" );
		i += 3;
		filter[ i++ ] = '\0';
		filter[ i++ ] = '\0';			
	}

	char selectedFile[MAX_PATH + 1] = "";
	fileName.copy( selectedFile, MAX_PATH );
	if ( !fileExt.empty())
	{
		strcat( selectedFile, "." );
		strcat( selectedFile, fileExt.c_str());
	}

	char initialDir[MAX_PATH + 1] = "";
	dir.copy( initialDir, MAX_PATH );

	OPENFILENAME openFileName = {0};

	openFileName.lStructSize       = sizeof(OPENFILENAME);
    openFileName.hwndOwner         = parent;
    openFileName.hInstance         = inst;

    openFileName.lpstrFilter       = filter;
    openFileName.lpstrCustomFilter = NULL;
    openFileName.nMaxCustFilter    = 0;
    openFileName.nFilterIndex      = 1;

    openFileName.lpstrFile         = selectedFile;
    openFileName.nMaxFile          = MAX_PATH;
    openFileName.lpstrFileTitle    = NULL;
    openFileName.nMaxFileTitle     = 0;

    openFileName.lpstrInitialDir   = initialDir;

    openFileName.lpstrTitle        = title.c_str();

    openFileName.nFileOffset       = 0;
    openFileName.nFileExtension    = 0;
    openFileName.lpstrDefExt       = NULL;

	openFileName.lpfnHook 		   = NULL;
    openFileName.lCustData         = NULL;

	openFileName.lpTemplateName    = NULL;

    openFileName.Flags             = OFN_OVERWRITEPROMPT
										| OFN_EXPLORER | OFN_PATHMUSTEXIST
											| DS_CENTER; // this does not do any good?

	BOOL getFile = GetSaveFileName( &openFileName );
	if ( !getFile )
		return "";

	return selectedFile;
}


bool DeleteFileFromCache( LPCSTR url )
{
	// browser cache is case sensitive
	BOOL outcome = FALSE;
	try
	{
		outcome = DeleteUrlCacheEntry( url );
	}
	catch (...)
	{
		outcome = FALSE;
	}
#ifdef _DEBUG
	assert( strlen(url) < MAX_PATH );
	if ( !outcome )
	{
		DWORD lastError = GetLastError();
		LPSTR errorPhrase = NULL;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
						| FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
						lastError,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &errorPhrase, 0, NULL );

		if ( errorPhrase )
		{
			// examine errorPhrase to see what error is
			HLOCAL hl = LocalFree( errorPhrase ); assert( hl == 0 );
		}
	}
#endif
	return outcome == TRUE;
}

// This method tries to open filePath.  It if succeeds, it returns the HANDLE
// and an empty tempFilePath.  If filePath can't open, this method tries to
// make a copy in the user's temp directory, and tries to open that copy.
// If it succeeds, it returns a HANDLE and the temp file's path in tempFilePath.
// Note: it seems that if filePath is open (via CreateFile()), then this method
// does not seem to succeed.
// throws
HANDLE OpenTheFile( const string &filePath, string *tempFilePath )
{
	if ( filePath.empty() || tempFilePath == NULL || !tempFilePath->empty())
		{ assert(false); return INVALID_HANDLE_VALUE; }

	if ( !FileExists( filePath ))
	{
		assert(false);
		return INVALID_HANDLE_VALUE;
	}

		// set hFile
	HANDLE hFile = CreateFile( filePath.c_str(), GENERIC_READ, 0, NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile != INVALID_HANDLE_VALUE )
		return hFile;

	// LOG( "Unable to open file \"" + filePath + "\" in FormMimeFile::OpenTheFile" );

	char tempDir[MAX_PATH] = "";
	DWORD len = GetTempPath( MAX_PATH, tempDir );
	if ( len == 0 )
		{ assert(false); return INVALID_HANDLE_VALUE; }

	string tempPath = tempDir;
	if ( !HasEndingBackSlash( tempPath ))
		tempPath += "\\";

	DWORD nameStarts = filePath.rfind( "\\" );
	if ( nameStarts == string::npos )
		{ assert(false); return INVALID_HANDLE_VALUE; }
	++nameStarts;

	tempPath += filePath.substr( nameStarts );

	if ( tempPath == filePath )
		tempPath += "_";

	assert( tempPath.size() < MAX_PATH );
	assert( !FileExists( tempPath ));

	BOOL copied = CopyFile( filePath.c_str(), tempPath.c_str(), TRUE );
	if ( copied )
	{
		//LOG( "RequestBody::OpenTheFile: CopyFile succeeded in creating \""
		//		+ tempPath + "\"" );
	}
	else
	{
		DWORD err = GetLastError();
		// file is open but not-read-only => ERROR_SHARING_VIOLATION
		// file is open and read-only => ERROR_ACCESS_DENIED
		assert( err == ERROR_SHARING_VIOLATION || err == ERROR_ACCESS_DENIED );
		// for logging side effects
		//LOG( "CopyFile(" + tempPath + ") failed in RequestBody::OpenTheFile" );
		assert(false);
		return INVALID_HANDLE_VALUE;
	}

	assert( hFile == INVALID_HANDLE_VALUE && FileExists( tempPath ));

	// It seems that CopyFile() followed by a CreateFile() leads to errors,
	// but if we pause before CreateFile(), then there is no error.  Go figure.

	int attempt = 0;
	while ( hFile == INVALID_HANDLE_VALUE && attempt < 3 )
	{
		hFile = CreateFile( tempPath.c_str(), GENERIC_READ, 0, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL );
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			//LOG( "Unable to open file \"" + tempPath + "\" in RequestBody::OpenTheFile, "
			//		"sleep a bit" );
			Sleep( 100 );
		}
		++attempt;
	}

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		//LOG( "Give up on open file \"" + tempPath + "\" in RequestBody::OpenTheFile" );
		assert(false);
		return INVALID_HANDLE_VALUE;
	}

	*tempFilePath = tempPath;

	return hFile;
}

