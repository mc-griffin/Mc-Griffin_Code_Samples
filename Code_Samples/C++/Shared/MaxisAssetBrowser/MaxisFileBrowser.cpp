// MaxisFileBrowser.cpp: implementation of the MaxisFileBrowser class.
//
//////////////////////////////////////////////////////////////////////

#include "MaxisFileBrowser.h"
#include "FileUtils.h"
#include <list>


bool MaxisFileBrowser::IsValidFilter( const string &filter )
{
	// Ensure that filter is a file extension with an optional '*' as the
	// last character.  The '.' that signifies the beginning of a file
	// extension is not represented.
	// An empty filter is equivalent to "*".
	if ( !filter.empty())
	{
		for ( int i = 0; i < filter.size(); ++i )
			if ( !isalnum( filter.at( i )))
				if ( filter.at( i ) == '*' )
				{
					if ( i != filter.size() - 1 )
					{
						// without the try-catch BoundsChecker reports serious error
						try { assert(false); } catch (...) {}
						return false;
					}
				}
				else
					return false;
	}

	return true;
}


bool MaxisFileBrowser::SelectDir( const string &dirIn, const string &filter /*=string()*/)
{
	if ( !IsValidFilter( filter ))
		{ assert(false); return false; }

	const bool isFilterOpen = filter.empty() || filter == "*";

	if ( dirIn.empty() || _dirs._maxNameLen < dirIn.length())
		{ assert(false); return false; }

	string dir = MakeWinPath( dirIn );
	if ( dir.empty()) { assert(false); return false; }

	if ( !DirExists( dir ) || dir[ dir.length()-1 ] == '\\' )
		{ assert(false); return false; }

	dir += "\\";
	dir += "*.*";

	WIN32_FIND_DATA	fd;
	HANDLE hSearch = ::FindFirstFile( dir.c_str(), &fd );

	if ( hSearch == INVALID_HANDLE_VALUE )
	{
		assert( GetLastError() == ERROR_FILE_NOT_FOUND );
		SetLastError(0);
		return false;
	}

	// set dirs and files
	list< string > dirs;
	list< string > files;
	do
	{
		if (( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY )
			dirs.push_back( fd.cFileName );
		else if ( isFilterOpen )
			files.push_back( fd.cFileName );
		else
		{
			char *dot = strstr( fd.cFileName, "." );
			// cFileName has no file extension and hence doesn't match the filter
			if ( dot == NULL )
				continue;
			WORD extOffset = dot - fd.cFileName + 1;

			// does cFileName (case insensitive) match filter?
			bool match = true;
			for ( int i = 0; i < filter.size(); ++i )
				if ( filter.at( i ) == '*' ) // match with '*'
					break;
				else if ( toupper( filter.at( i ))
						!= toupper(( fd.cFileName + extOffset )[ i ]))
				{
					match = false;
					break;
				}
	
			if ( match )
				files.push_back( fd.cFileName );
		}
	}
	while ( ::FindNextFile( hSearch, &fd ) == TRUE );
	::FindClose( hSearch );

	dirs.sort();
	files.sort();

	// from here on, we start changing members, no errors should be possible after this point

	_dirs.MakeEmpty();
	_files.MakeEmpty();

	// _dirs <- dirs
	//_files <- files
	{
		list< string >::iterator it = dirs.begin();
		while ( it != dirs.end())
		{
			_dirs.Append( *it );
			++it;
		}
		it = files.begin();
		while ( it != files.end())
		{
			_files.Append( *it );
			++it;
		}
	}

	return true;
}


DWORD MaxisFileBrowser::GetDirCount() const
{
	return _dirs.GetSize();
}

string MaxisFileBrowser::GetFirstDir()
{
	bool select = _dirs.SelectFirst();
	if ( !select )
	{
		//assert(false);
		return "";
	}

	return _dirs.GetSelected();
}


string MaxisFileBrowser::GetNextDir()
{
	bool select = _dirs.SelectNext();
	if ( !select )
	{
		assert( 0 < _dirs.GetSize());
		return "";
	}

	return _dirs.GetSelected();
}


DWORD MaxisFileBrowser::GetFilesCount() const
{
	return _files.GetSize();
}


string MaxisFileBrowser::GetFirstFile()
{
	bool select = _files.SelectFirst();
	if ( !select )
		return "";

	return _files.GetSelected();
}


string MaxisFileBrowser::GetNextFile()
{
	bool select = _files.SelectNext();
	if ( !select )
	{
		assert( 0 < _files.GetSize());
		return "";
	}

	return _files.GetSelected();
}


string MaxisFileBrowser::ConvertFileNameToSrf( const string &filePath )
{
	if ( filePath.empty()) { assert(false); return ""; }

	string dir;
	string fileNameSansExt;

	if ( !SplitFilePath( filePath, &dir, &fileNameSansExt, NULL ))
		{ assert(false); return ""; }

	if ( dir.empty() || fileNameSansExt.empty())
		{ assert(false); return ""; }

	return dir + "\\" + fileNameSansExt + ".srf";
}

// begin remove
bool MaxisFileBrowser::DirExists( const string &pathIn )
{
	string path = MakeWinPath( pathIn );
	DWORD a = GetFileAttributes( path.c_str());
	if ( a == 0xFFFFFFFF ) return false;
	if (( a & FILE_ATTRIBUTE_DIRECTORY ) != FILE_ATTRIBUTE_DIRECTORY )
		return false;
	return true;
}


bool MaxisFileBrowser::FileExists( const string &filePathIn )
{
	string filePath = MakeWinPath( filePathIn );
	DWORD fileAttributes = GetFileAttributes( filePath.c_str());
	return fileAttributes != 0xFFFFFFFF;
}


string MaxisFileBrowser::MakeWinPath( const string &unixPath )
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


string MaxisFileBrowser::MakeUnixPath( const string &winPath )
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
// end remove

bool MaxisFileBrowser::Test()
{
	printf( "Test MaxisFileBrowser\n" );
	bool passed = true;

	if ( !DirExists( "\\Windows" ) && !DirExists( "\\WinNt" ))
	{
		passed = false;
		printf( "DirExists() failed\n" );
	}

	if ( !FileExists( "\\autoexec.bat" ))
	{
		passed = false;
		printf( "FileExists() failed\n" );
	}

	if ( MakeWinPath( "/\\" ) != "\\\\" )
	{
		passed = false;
		printf( "MakeWinPath() failed\n" );
	}

	if ( MakeUnixPath( "/\\" ) != "//" )
	{
		passed = false;
		printf( "MakeUnixPath() failed\n" );
	}

	if ( !IsValidFilter( "" ) || !IsValidFilter( "*" )
			|| IsValidFilter( "." ) || IsValidFilter( "*b" )
				|| !IsValidFilter( "a*" )  || !IsValidFilter( "xyz" ))
	{
		passed = false;
		printf( "IsValidFilter() failed\n" );
	}

	MaxisFileBrowser b;

	if ( !b.GetFirstDir().empty() || !b.GetFirstFile().empty()
			|| !b.GetNextDir().empty() || !b.GetNextFile().empty())
	{
		printf( "GetFirst/NextDir/File() failed\n" );
		passed = false;
	}

	char dirThatShouldNotExist[] = "dir that should not exist";
	if ( DirExists( dirThatShouldNotExist ) || b.SelectDir( dirThatShouldNotExist ))
	{
		printf( "DirExists() or SelectDir() failed\n" );
		passed = false;
	}

	char dir[ MAX_PATH + 1 ] = "";
	DWORD dirLen = GetCurrentDirectory( MAX_PATH, dir );
	assert( 0 < dirLen && 0 < strlen( dir ));

	if ( !DirExists( dir ) || !FileExists( string( dir ) + "\\MaxisAssetBrowserTest.dsp" ))
	{
		printf( "DirExists() or FileExists() failed\n" );
		passed = false;
	}

	if ( !b.SelectDir( dir, "dsp" ))
	{
		printf( "SelectDir() failed\n" );
		passed = false;
	}

	if ( b.GetFirstFile() != "MaxisAssetBrowserTest.dsp" ) // case sensitivity problem?
	{
		printf( "GetFirstFile() failed\n" );
		passed = false;
	}

	// I assume only one dsw file in this directory
	if ( !b.GetNextFile().empty())
	{
		printf( "GetNextFile() failed\n" );
		passed = false;
	}

	return passed;
}


