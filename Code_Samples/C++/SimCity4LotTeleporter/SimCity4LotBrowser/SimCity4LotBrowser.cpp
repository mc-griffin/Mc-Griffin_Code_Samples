// SimCity4LotBrowser.cpp: implementation of the SimCity4LotBrowser class.
//
//////////////////////////////////////////////////////////////////////

#include "SimCity4LotBrowser.h"
#include "FileUtils.h"


bool SimCity4LotBrowser::SelectDir( const string &dirIn, const string &filter,
									list<string> *files )
{
	if ( files == NULL ) { assert(false); return false; }

	if ( !IsValidFilter( filter ))
		{ assert(false); return false; }

	const bool isFilterOpen = filter.empty() || filter == "*";

	string dir = dirIn;

	if ( dir.empty() || _dirs._maxNameLen < dir.length())
		{ assert(false); return false; }

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

	// fill files
	do
	{
		if (( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
		{
			if ( isFilterOpen )
				files->push_back( fd.cFileName );
			else
			{
				// get the last occurrence of period
				char *dot = strrchr( fd.cFileName, '.' );
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
					files->push_back( fd.cFileName );
			}
		}
	}
	while ( ::FindNextFile( hSearch, &fd ) == TRUE );
	::FindClose( hSearch );

	files->sort();

	return true;
}


bool SimCity4LotBrowser::SelectDir( const string &dirIn )
{
	bool noErrors = true;

	if ( dirIn.empty() || MAX_PATH < dirIn.length() || !DirExists( dirIn ))
		{ assert(false); return false; }

	_dirs.MakeEmpty();
	_files.MakeEmpty();

	// fill _dirs
	{
		string dir = dirIn;

		if ( !DirExists( dir ) || dir[ dir.length()-1 ] == '\\' )
			{ assert(false); return false; }

		dir += "\\";
		dir += "*.*";

		list<string> dirs;

		WIN32_FIND_DATA	fd;
		HANDLE hSearch = ::FindFirstFile( dir.c_str(), &fd );

		if ( hSearch == INVALID_HANDLE_VALUE )
		{
			assert( GetLastError() == ERROR_FILE_NOT_FOUND );
			SetLastError(0);
			return false;
		}

		do
		{
			if (( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY )
				dirs.push_back( fd.cFileName );
		}
		while ( ::FindNextFile( hSearch, &fd ) == TRUE );
		::FindClose( hSearch );

		dirs.sort();

		list<string>::iterator it = dirs.begin();
		while ( it != dirs.end())
		{
			_dirs.Append( *it );
			++it;
		}
	}

	// fill _files
	{
		list<string> files;
		if (! SelectDir( dirIn, "SC4Lot", &files ))
			noErrors = false;

		list<string>::iterator it = files.begin();
		while ( it != files.end())
		{
			_files.Append( *it );
			++it;
		}
	}

	_selectedDir = dirIn;

	return noErrors;
}


DWORD SimCity4LotBrowser::GetDirCount() const
{
	return _dirs.GetSize();
}


string SimCity4LotBrowser::GetFirstDir()
{
	bool select = _dirs.SelectFirst();
	if ( !select )
	{
		//assert(false);
		return "";
	}

	return _dirs.GetSelected();
}


string SimCity4LotBrowser::GetNextDir()
{
	bool select = _dirs.SelectNext();
	if ( !select )
	{
		assert( 0 < _dirs.GetSize());
		return "";
	}

	return _dirs.GetSelected();
}


DWORD SimCity4LotBrowser::GetFileCount() const
{
	return _files.GetSize();
}


string SimCity4LotBrowser::GetFirstFile()
{
	bool select = _files.SelectFirst();
	if ( !select )
		return "";

	return _files.GetSelected();
}


string SimCity4LotBrowser::GetNextFile()
{
	bool select = _files.SelectNext();
	if ( !select )
	{
		assert( 0 < _files.GetSize());
		return "";
	}

	return _files.GetSelected();
}


// borrowed from MaxisAssetBrowser, because its such a pain to link to that project
bool SimCity4LotBrowser::IsValidFilter( const string &filter )
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

