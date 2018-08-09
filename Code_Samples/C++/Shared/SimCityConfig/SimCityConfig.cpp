// SimCityConfig.cpp: implementation of the SimCityConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "SimCityConfig.h"
#include "FileUtils.h"
#include "Utilities.h"
#include <windows.h>
#include <assert.h>


bool SimCityConfig::Initialize( const string &endOfLine /*=string()*/)
{
	string eol = "\r\n";
	if ( !endOfLine.empty())
		eol = endOfLine;

	// HKEY_LOCAL_MACHINE\SOFTWARE\Electronic Arts\Maxis\SimCity 3000 Unlimited
	HKEY installKey = NULL;
	// HKEY_LOCAL_MACHINE\SOFTWARE\Electronic Arts\Maxis\SimCity 3000 Unlimited\1.00.000
	HKEY version1Key = NULL;

	try
	{
		// Set installKey
		long result = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Electronic Arts\\Maxis\\SimCity 3000 Unlimited",
			0, KEY_QUERY_VALUE, &installKey );
		if ( result != ERROR_SUCCESS )
		{
			throw "SimCity 3000 Unlimited is not installed on this machine.";
		}

		const DWORD simCityPathBufLen = MAX_PATH;
		char simCityPathBuf[ simCityPathBufLen + 1 ] = "";
		const DWORD serialBufLen = MAX_PATH;
		char serialBuf[ serialBufLen + 1 ] = "";
		const DWORD pubToolLastPathBufLen = MAX_PATH;
		char pubToolLastPathBuf[ pubToolLastPathBufLen ] = "";

		// InstalledPath
		{
			DWORD simCityPathLen = simCityPathBufLen;
			DWORD type = REG_SZ;
			result = RegQueryValueEx( installKey, "InstalledPath", 0, &type,
				(unsigned char *) simCityPathBuf, &simCityPathLen );
			if ( result != ERROR_SUCCESS )
			{
				throw "The SimCity 3000 Unlimited installation on this "
					"machine is mangled.  Can't get install path.";
			}
			assert( simCityPathLen != 0 && strlen( simCityPathBuf ) + 1 == simCityPathLen );
		}

		// Serial Number
		{
			DWORD serialLen = serialBufLen;
			DWORD type = REG_SZ;
			result = RegQueryValueEx( installKey, "Serial Number", 0, &type,
				(unsigned char *) serialBuf, &serialLen );
			if ( result != ERROR_SUCCESS )
			{
				throw "The SimCity 3000 Unlimited installation on this "
					"machine is mangled.  Can't get serial number.";
			}
			assert( serialLen != 0 && strlen( serialBuf ) + 1 == serialLen );
		}

		// PubToolLastPath, may not be there?
		{
			DWORD pubLen = pubToolLastPathBufLen;
			DWORD type = REG_SZ;
			result = RegQueryValueEx( installKey, "PubToolLastPath", 0, &type,
				(unsigned char *) pubToolLastPathBuf, &pubLen );
			// I want to examine installations where this asserts!
			assert( result == ERROR_SUCCESS );
			assert( pubLen != 0 && strlen( pubToolLastPathBuf ) + 1 == pubLen );
		}

		// Set version1Key
		result = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Electronic Arts\\Maxis\\SimCity 3000 Unlimited\\1.00.000",
			0, KEY_QUERY_VALUE, &version1Key );
		if ( result == ERROR_SUCCESS )
			_version = "1.00.000";
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

		_path = simCityPathBuf;
		_serialNumber = serialBuf;
		_pubToolLastPath = pubToolLastPathBuf;

		if ( HasEndingBackSlash( _path ))
		{
			// I'd like to know if this ever happens
			assert(false);
			_path.resize( _path.size() - 1 );
			assert( !HasEndingBackSlash( _path ));
		}

		// set _pathNoApps
		{
			DWORD apps = _path.rfind( "\\Apps" );
			if ( apps == string::npos )
				assert(false);
			else
				_pathNoApps = _path.substr( 0, apps );
		}

		// does PubToolLastPath always end with a "\"?
		assert( _pubToolLastPath.empty()
				|| _pubToolLastPath[ _pubToolLastPath.size() - 1 ] == '\\' );

		assert( _description.empty()); // any error should have returned

		_description = "The SimCity 3000 Unlimited version ";
		_description += ( _version.empty()) ? "unknown" : _version;
		_description += " is installed at:";
		_description += eol;
		_description += ( _path.empty()) ? "not found" : _path;
		_description += eol;
		_description += "The serial number is ";
		_description += ( _serialNumber.empty()) ? "not found" : _serialNumber;
		_description += ( "." + eol );
		_description += "PubToolLastPath is ";
		_description += ( _pubToolLastPath.empty()) ? "not found" : _pubToolLastPath;
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


