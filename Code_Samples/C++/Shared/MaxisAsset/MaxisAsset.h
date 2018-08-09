// MaxisAsset.h: interface for the MaxisAsset class.
//
// Design Notes
//
// MaxisAsset is an abstract class and the class that subclasses
// MaxisAsset must override SetNameValue() and AllNeededFilesExist().
// Further, _sectionNames should be set in the constructor of the subclass.
//
// I ASSUME that no value will appear in different sections.  In other
// words, I assume that this code will not encounter an INI file like:
//		[Section1]
//		value="x"
//		[Section2]
//		value="x"
// because "value" appears in Section1 and Section2.
//
// MaxisAsset uses the Template pattern from the Gang of Four, meaning
// that SelectIniFile() calls the overriden SetNameValue().
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAXISASSET_H__3F60E8DD_4368_423E_9F1D_EDFE78F8AC62__INCLUDED_)
#define AFX_MAXISASSET_H__3F60E8DD_4368_423E_9F1D_EDFE78F8AC62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <windows.h>

// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>
#include <vector>
using namespace std;


class MaxisAsset
{
protected:
	string _assetType; // for example, LOT or NPC

	// .ini file
	string _iniFilePath;
	string _fileNameSansExt;
	string _dirPath;
	// _assetFileName is the file name from _iniFilePath, but with _fileExt as a file extension
	string _assetFileName;

public:
	MaxisAsset( const string &fileExt, const string &assetType );
	const string _fileExt; // for example, "dtl" or "cse"
	// section names in file with .INI structure, for example, "DowntownLot"
	vector<string> _sectionNames;

	// not virtual, so that this method can be called by a subclass
	bool IsSet() const;

	// .ini file
	bool SelectIniFile( const string &filePath );

	// data validation
	virtual bool AllNeededFilesExist() const = 0;
	static bool IsValidIff( const string &filePath );

	const string &GetFilePath() const;
	const string &GetAssetFileName() const;
	string GetAssetFilePath() const;

	string GetAssetType() const;

protected:
	// used by SetFromIniFile()
	virtual bool SetNameValue( LPCSTR name, LPCSTR value ) = 0;

public:
	// .SRF interface
	// Note: a MaxisAsset that uses .SRF files can use this interface,
	// but a MaxisAsset that doesn't use .SRF files does NOT need to use it.
	virtual bool SelectSrf( const string &srfPath );
	bool IsSrfSelected() const;
	static bool HasSrfFileExt( const string &fileName );
};

#endif // !defined(AFX_MAXISASSET_H__3F60E8DD_4368_423E_9F1D_EDFE78F8AC62__INCLUDED_)
