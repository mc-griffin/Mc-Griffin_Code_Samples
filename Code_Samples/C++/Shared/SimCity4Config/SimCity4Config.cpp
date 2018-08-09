// SimCity4Config.cpp: implementation of the SimCity4Config class.
//
//////////////////////////////////////////////////////////////////////

#include "SimCity4Config.h"
#include "FileUtils.h"
#include "Utilities.h"
#include <windows.h>
#include <shlobj.h>
#include <assert.h>


bool SimCity4Config::Initialize( const string &endOfLine /*=string()*/)
{
	string eol = "\r\n";
	if ( !endOfLine.empty())
		eol = endOfLine;

	// HKEY_LOCAL_MACHINE\SOFTWARE\Maxis\SimCity 4
	HKEY installKey = NULL;
	// HKEY_LOCAL_MACHINE\SOFTWARE\Maxis\SimCity 4\1.00
	HKEY version1Key = NULL;

	try
	{
		// Set installKey
		long result = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Maxis\\SimCity 4",
			0, KEY_QUERY_VALUE, &installKey );
		if ( result != ERROR_SUCCESS )
		{
			throw "SimCity 4 is not installed on this machine.";
		}

		const DWORD simCityPathBufLen = MAX_PATH;
		char simCityPathBuf[ simCityPathBufLen + 1 ] = "";
		const DWORD serialBufLen = MAX_PATH;
		char serialBuf[ serialBufLen + 1 ] = "";

		// InstalledPath
		{
			DWORD simCityPathLen = simCityPathBufLen;
			DWORD type = REG_SZ;
			result = RegQueryValueEx( installKey, "Install Dir", 0, &type,
				(unsigned char *) simCityPathBuf, &simCityPathLen );
			if ( result != ERROR_SUCCESS )
			{
				throw "The SimCity 4 installation on this "
					"machine is mangled.  Can't get install path.";
			}
			assert( simCityPathLen != 0 && strlen( simCityPathBuf ) + 1 == simCityPathLen );
		}

		// assuming Product GUID = Serial # (but has { } around it)
		{
			DWORD serialLen = serialBufLen;
			DWORD type = REG_SZ;
			result = RegQueryValueEx( installKey, "Product GUID", 0, &type,
				(unsigned char *) serialBuf, &serialLen );
			if ( result != ERROR_SUCCESS )
			{
				throw "The SimCity 4 installation on this "
					"machine is mangled.  Can't get serial number.";
			}
			assert( serialLen != 0 && strlen( serialBuf ) + 1 == serialLen );
		}

		// Language
		{
			char languageBuf[ MAX_PATH + 1 ] = "";
			DWORD languageBufLen = MAX_PATH;
			DWORD type = REG_SZ;
			result = RegQueryValueEx( installKey, "Language", 0, &type,
				(unsigned char *) languageBuf, &languageBufLen );
			if ( result == ERROR_SUCCESS )
				_language = languageBuf;
		}

		// set _isRushHourThere and _isDeluxeThere
		{
			// Moe says:
			//	Deluxe:  Has EP1 subkey and value IsDeluxe = 1 in SimCity 4 key
			//	Rush Hour: Has EP1 subkey and no value IsDeluxe
			//	Base product:  No EP1 subkey, no IsDeluxe value in SimCity 4 key.
			// If IsDeluxe is there, I assume Deluxe is installed and don't check for EP1.

			DWORD deluxe = 0;
			DWORD deluxeSize = sizeof(DWORD);
			DWORD type = REG_DWORD;
			result = RegQueryValueEx( installKey, "IsDeluxe", 0, &type,
				(unsigned char *) &deluxe, &deluxeSize );
			if ( result == ERROR_SUCCESS )
			{
				// I don't check the value of deluxe because Moe says:
				// "IsDeluxe will never be set to anything other than 1.
				// It will either be set to 1, or it wont exist at all."
				_isDeluxeThere = true;
			}
			else
			{
				// if deluxe is not there, check for rush hour
				HKEY rushHourKey = NULL;
				long result = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
					"SOFTWARE\\Maxis\\SimCity 4\\EP1",
					0, KEY_QUERY_VALUE, &rushHourKey );
				if ( result == ERROR_SUCCESS )
				{
					_isRushHourThere = true;
					LONG regClose = RegCloseKey( rushHourKey );
					assert( regClose == ERROR_SUCCESS );
				}
			}
			
			// _isDeluxeThere and _isRushHourThere cannot both be true
			assert( !_isDeluxeThere && !_isRushHourThere ||
				( _isDeluxeThere != _isRushHourThere ));
		}

		// Set version1Key
		result = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Maxis\\SimCity 4\\1.0",
			0, KEY_QUERY_VALUE, &version1Key );
		if ( result == ERROR_SUCCESS )
			_version = "1.0";
		else
			assert(false);

		// release resources
		{
			LONG regClose = ERROR_SUCCESS;
			if ( installKey != NULL )
			{
				regClose = RegCloseKey( installKey );
				assert( regClose == ERROR_SUCCESS );
				installKey = NULL;
			}

			if ( version1Key != NULL )
			{
				regClose = RegCloseKey( version1Key );
				assert( regClose == ERROR_SUCCESS );
				version1Key = NULL;
			}
		}

		_installPath = simCityPathBuf;
		_serialNumber = serialBuf;

		if ( HasEndingBackSlash( _installPath ))
		{
			_installPath.resize( _installPath.size() - 1 );
			assert( !HasEndingBackSlash( _installPath ));
		}

		// ------------- APP code to get download directory -------------
		// Taken from SC4000 code.
		//
		//By default, use the game data directory. If the system has use-specific
		//directory support, then let that override the data directory usage.
   
		//We use the SHGetSpecialFolderLocation function under Windows.
		//CSIDL_APPDATA is only available with v4.71 and later versions of shell32.dll.
		//According to Microsoft documentation, you need "IE 4.0 Integrated Shell" or 
		//later. This means that WinME, Win2000, WinXP and later will have this functionality
		//by default. But Win95, Win98, and WinNT will only have it if the user installed 
		//IE 4 or something else that enabled this functionality. In this case, we simply
		//will not use this user directory information.
		//
		//10/16/02 - ENANALE - changed to CSIDL_PERSONAL ("My Documents") folder
		//on request of CQC and approval by Billy.  The Application Data folder
		//is harder to find (and hidden by default on some systems.)
		char        szPath[MAX_PATH];
		string		downloadDir;
		LPMALLOC      pMalloc    = NULL;
		LPITEMIDLIST  pItemDList = NULL; 

		if(::SHGetSpecialFolderLocation((HWND)NULL, CSIDL_PERSONAL, &pItemDList) == NOERROR){
		 if(::SHGetPathFromIDList(pItemDList, szPath)){
			assert(szPath[0]);
			if(szPath[0]){
			   downloadDir = szPath;
			   if(! HasEndingBackSlash(downloadDir) )
				  downloadDir += "\\";
			}
		 }
		 if(::SHGetMalloc(&pMalloc) == NOERROR){
			pMalloc->Free(pItemDList);
			pMalloc->Release();
		 }
		}
 
		if(downloadDir.length()){
		  //Now we have a string that is something like:
		  //   "C:\Documents and Settings\PPedrian\My Documents\"
		  _downloadPath = downloadDir + "SimCity 4\\Regions\\Downloads";
		  _albumPath = downloadDir + "SimCity 4\\Albums";
		  _regionPath = downloadDir + "SimCity 4\\Regions";
		  _pluginPath = downloadDir + "SimCity 4\\Plugins";
		}
		else
		{ 
			throw "Unable to obtain My Documents folder."; 
		}

		assert( _description.empty()); // any error should have returned

		_description = "The SimCity 4 version ";
		_description += ( _version.empty()) ? "unknown" : _version;
		_description += " is installed at:";
		_description += eol;
		_description += ( _installPath.empty()) ? "not found" : _installPath;
		_description += eol;
		_description += "The serial number is ";
		_description += ( _serialNumber.empty()) ? "not found" : _serialNumber;
		_description += ( "." + eol + eol );
		_description += "The download path is ";
		_description += ( _downloadPath.empty()) ? "not found" : _downloadPath;
		_description += ( "." + eol + eol );
	}
	catch ( char const *err )
	{
		_description = err;
		_description += eol;

		// release resources
		LONG regClose = ERROR_SUCCESS;
		if ( installKey != NULL )
		{
			regClose = RegCloseKey( installKey );
			assert( regClose == ERROR_SUCCESS );
			installKey = NULL;
		}

		if ( version1Key != NULL )
		{
			regClose = RegCloseKey( version1Key );
			assert( regClose == ERROR_SUCCESS );
			version1Key = NULL;
		}

		return false;
	}
	catch (...)
	{
		_description = "Unknown exception" + eol;

		// release resources
		LONG regClose = ERROR_SUCCESS;
		if ( installKey != NULL )
		{
			regClose = RegCloseKey( installKey );
			assert( regClose == ERROR_SUCCESS );
			installKey = NULL;
		}

		if ( version1Key != NULL )
		{
			regClose = RegCloseKey( version1Key );
			assert( regClose == ERROR_SUCCESS );
			version1Key = NULL;
		}

		return false;
	}

	return IsSet();
}

// All SimCity skus use unicode; our DB uses ascii. However, according to PaulP:
// 
// Most "Western European" characters are supported by the Latin 1 character set, 
// which uses 0x0000 - 0x00FF. And Unicode maps its own 0x0000 - 0x00FF to the 
// Latin 1 character set. So the result is that most text supported by the game can 
// be stored in non-Unicode (assuming you can store unsigned bytes) if it is one of 
// the following languages:
//	English
//	UK English
//	French
//	German
//	Spanish
//	Italian
//	Norwegian
//	Finnish
//	Dutch
//	Danish
//	Swedish
//	Portuguese
// The full list of allowable values for the Language reg keys comes from the EA
// Installer, and was sent to me by Ed Nanale
bool SimCity4Config::IsReasonableCharSet() const
{
	return (_language == "English US" ||
			_language == "English UK" ||
			_language == "French" ||
			_language == "German" ||
			_language == "Spanish" ||
			_language == "Italian" || 
			_language == "Norwegian" ||
			_language == "Finnish" ||
			_language == "Dutch" ||
			_language == "Danish" ||
			_language == "Swedish" ||
			_language == "PortBrzl");
}