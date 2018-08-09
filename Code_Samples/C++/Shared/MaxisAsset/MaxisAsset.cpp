// MaxisAsset.cpp: implementation of the MaxisAsset class.
//
//////////////////////////////////////////////////////////////////////

#include "MaxisAsset.h"
#include "FileUtils.h"
#include "Utilities.h"
#include <assert.h>


MaxisAsset::MaxisAsset( const string &fileExt, const string &assetType )
	: _fileExt( fileExt )
{
	if ( assetType.length() == 3 )
	{
		assert( ToUpper( assetType ) == assetType );
		_assetType = assetType;
	}
	else
		assert(false);

	assert( !_fileExt.empty() && _fileExt.find( "." ) == string::npos );
}


string MaxisAsset::GetAssetType() const
{
	if ( _assetType.length() != 3 ) { assert(false); return ""; }
	return _assetType;
}


bool MaxisAsset::IsSet() const
{
	if ( _sectionNames.empty())
		return false;
	if ( _fileExt.empty() || _fileExt.find( "." ) != string::npos )
		return false;
	if ( _assetType.length() != 3 )
		return false;
	if ( _iniFilePath.empty())
		return false;
	return true;
}


bool MaxisAsset::IsValidIff( const string &filePath )
{
	if ( filePath.empty()) { assert(false); return false; }

	static const char dtlSig[] =
		"IFF FILE 2.5:TYPE FOLLOWED BY SIZE";
	static const int dtlSigLen = sizeof( dtlSig ) - 1; // - 1 for trailing zero
	assert( strlen( dtlSig ) == dtlSigLen );

	HANDLE hFile = CreateFile( filePath.c_str(), GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		assert(false);
		return false;
	}

	DWORD bytes = GetFileSize( hFile, NULL );
	if ( bytes <= dtlSigLen )
	{
		assert(false);
		return false;
	}

	// does it look like a valid DTL?
	char buf[ dtlSigLen + 1 ] = "";
	DWORD bytesRead = 0;
	BOOL readit = ReadFile( hFile, (void *) buf, dtlSigLen, &bytesRead, NULL );
	if ( !readit || bytesRead < dtlSigLen )
	{
		assert(false);
		return false;
	}
	assert( buf[ bytesRead ] == '\0' );
	buf[ bytesRead ] = '\0';

	BOOL close = CloseHandle( hFile ); assert( close );

	return strncmp( dtlSig, buf, dtlSigLen ) == 0;
}


bool MaxisAsset::SelectIniFile( const string &filePath )
{
	// Note:
	// 1. If there are double quotation marks around the name of a
	// name-value pair, for example "name with quotes"="yes",
	// those double quotation marks will show up in the name
	// when read by GetPrivateProfileString().
	// 2. If the srf file contains unrecognized names, this function fails.

	if ( filePath.empty() || HasEndingBackSlash( filePath ) || !FileExists( filePath ))
	{
		assert(false);
		return false;
	}

	// _sectionNames and _fileExt should be set before calling this method
	if ( _sectionNames.empty() || _fileExt.empty()) { assert(false); return false; }

	bool errorOccurred = false;

	// Fill namesList with a list of names represented in the sections in srfPath.
	for ( DWORD sec = 0; sec < _sectionNames.size(); ++sec )
	{
		const string sectionName = _sectionNames[sec];

		vector<string> namesList;
		GetNamesInSection( filePath, sectionName, &namesList );

		// For each name in namesList, find its associated value in srfPath.

		for ( int i = 0; i < namesList.size(); ++i )
		{
			const DWORD bufSize = 512;
			char value[ bufSize ] = "";
			const char defaultValue[] = "";

			string name = namesList[i];

			DWORD r = GetPrivateProfileString( sectionName.c_str(), name.c_str(), defaultValue,
						value, bufSize - 1, filePath.c_str());
			if ( r < 1 )
			{
				assert(false);
				continue;
			}
			assert( r < bufSize );

			// HERE IS WHERE I ASSUME that all values are unique.
			bool set = SetNameValue( name.c_str(), value );
			if ( !set )
			{
				assert(false);
				errorOccurred = true;
			}
		}

	}
	
	if ( errorOccurred )
		return false;

	_iniFilePath = filePath;

	bool split = SplitFilePath( filePath, &_dirPath, &_fileNameSansExt, NULL );
	if ( !split ) { assert(false); return false; }
	
	_assetFileName = _fileNameSansExt + "." + _fileExt;

	return !_iniFilePath.empty() && !_assetFileName.empty() && !_fileNameSansExt.empty()
				&& !_dirPath.empty();
}


// .SRF Interface

bool MaxisAsset::SelectSrf( const string &srfPath )
{
	if ( !HasSrfFileExt( srfPath ))
		{ assert(false); return false; }
	
	return MaxisAsset::SelectIniFile( srfPath );
}


bool MaxisAsset::IsSrfSelected() const
{
	return IsSet();
}


bool MaxisAsset::HasSrfFileExt( const string &fileName )
{
	if ( !HasFileExtension( fileName ))
		return false;
	string fileExt = ".srf";
	// create an all lower case version of fileName
	string lowerCaseFileName;
	{
		char buf[2] = { '\0', '\0' };
		for ( DWORD i = 0; i < fileName.length(); ++i )
		{
			char c = fileName.at( i );
			buf[0] = isupper( c ) ? tolower( c ) : c;
			lowerCaseFileName += buf;
		}
	}

	DWORD fileExtStart = lowerCaseFileName.rfind( fileExt );
	assert( fileExtStart != string::npos );

	return fileExtStart == lowerCaseFileName.length() - fileExt.length();
}


const string &MaxisAsset::GetFilePath() const
{
	assert( IsSrfSelected());
	return _iniFilePath;
}

const string &MaxisAsset::GetAssetFileName() const
{
	assert( IsSrfSelected());
	return _assetFileName;
}

string MaxisAsset::GetAssetFilePath() const
{
	if ( !IsSrfSelected()) { assert(false); return ""; }
	string path = _dirPath + "\\" + _assetFileName;
	return path;
}

