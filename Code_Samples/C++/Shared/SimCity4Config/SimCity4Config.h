// SimCity4Config.h: interface for the SimCity4Config class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMCITYCONFIG_H__6FCE6FDC_4A96_4121_B1CA_02719D1BA5B2__INCLUDED_)
#define AFX_SIMCITYCONFIG_H__6FCE6FDC_4A96_4121_B1CA_02719D1BA5B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>
using namespace std;


class SimCity4Config // SimCity 4
{
protected:
	// HKEY_LOCAL_MACHINE\SOFTWARE\Electronic Arts\Maxis\SimCity 4 contains Install Dir
	string _installPath;
	// something like C:\Documents and Settings\apai\My Documents\SimCity4\Regions\Downloads\	
	string _downloadPath;
	// Serial Number
	string _serialNumber;
	// album path
	string _albumPath;
	// region path
	string _regionPath;
	// plugin path
	string _pluginPath;
	// Language
	string _language;

	// what happens in the Registry when a higher version than 1.0 is installed?
	// left out of _description until better understanding
	// HKEY_LOCAL_MACHINE\SOFTWARE\Electronic Arts\Maxis\SimCity 4\1.0
	string _version;

	string _description;

	bool _isRushHourThere;
	bool _isDeluxeThere;

public:
	SimCity4Config() : _isRushHourThere(false), _isDeluxeThere(false) {}

	bool Initialize( const string &eol = string());
	bool IsSet() const { return !_description.empty(); }

	string GetDescription() const { return _description; }

	// Rules for IsSimCity4There(), IsRushHourThere(), and IsDeluxeThere():
	// * IsSimCity4There() will be true if there is any SimCity4 installed.
	// * IsRushHourThere() and IsDeluxeThere() cannot both be true.
	// * If either IsRushHourThere() or IsDeluxeThere() are true, then
	//   IsSimCity4There() will also be true.

	bool IsSimCity4There() const { return !_serialNumber.empty(); }
	bool IsReasonableCharSet() const; // returns true if SKU is US or Euro (since these use mostly characters in the 0x00 to 0xff range, which will work best with our ascii databases)
	bool IsRushHourThere() const { return _isRushHourThere; }
	bool IsDeluxeThere() const { return _isDeluxeThere; }

	string GetInstallPath() const { return _installPath; }
	string GetDownloadPath() const { return _downloadPath; }
	string GetSerialNumber() const { return _serialNumber; }
	string GetRegionPath() const { return _regionPath; }
	string GetAlbumPath() const { return _albumPath; }
	string GetPluginPath() const { return _pluginPath; }
	string GetLanguage() const { return _language; }
};


#endif // !defined(AFX_SIMCITYCONFIG_H__6FCE6FDC_4A96_4121_B1CA_02719D1BA5B2__INCLUDED_)
