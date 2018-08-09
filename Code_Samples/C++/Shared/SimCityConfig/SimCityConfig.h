// SimCityConfig.h: interface for the SimCityConfig class.
//
// * Only Unlimited assets can be uploaded.
// * SimCity 3000 Unlimited cities are fully backwards-compatible with the
//   original SimCity 3000.
// * Buildings and Scenarios are specific to Unlimited.
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


class SimCityConfig // SimCity 3000 Unlimited
{
protected:
	// HKEY_LOCAL_MACHINE\SOFTWARE\Electronic Arts\Maxis\SimCity 3000 Unlimited contains
	// InstalledPath, always ends in "\Apps"
	string _path;
	string _pathNoApps; // _path - "\Apps"
	// Serial Number
	string _serialNumber;
	// PubToolLastPath
	string _pubToolLastPath;
	// SKU
	string _sku;
	// Country
	string _country;
	// Language
	string _language;
	// ScenarioCreator
	string _scenarioCreator;

	// what happens in the Registry when a higher version than 1.00.000 is installed?
	// left out of _description until better understanding
	// HKEY_LOCAL_MACHINE\SOFTWARE\Electronic Arts\Maxis\SimCity 3000 Unlimited\1.00.000
	string _version;

	string _description;

public:
	bool Initialize( const string &eol = string());
	bool IsSet() const { return !_description.empty(); }

	string GetDescription() const { return _description; }

	bool IsSimCityThere() const { return !_serialNumber.empty(); }

	string GetPath() const { return _path; }
	string GetPathNoApps() const { return _pathNoApps; }
	string GetSerialNumber() const { return _serialNumber; }
	string GetPubToolLastPath() const { return _pubToolLastPath; }
};


#endif // !defined(AFX_SIMCITYCONFIG_H__6FCE6FDC_4A96_4121_B1CA_02719D1BA5B2__INCLUDED_)
