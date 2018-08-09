// SimCity4LotTeleporter.h: interface for the SimCity4LotTeleporter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMCITY4LOTTELEPORTER_H__EE46E844_7A18_428D_9A7A_74CA007D741A__INCLUDED_)
#define AFX_SIMCITY4LOTTELEPORTER_H__EE46E844_7A18_428D_9A7A_74CA007D741A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <assert.h>

// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>

using namespace std;

#include "MaxisTeleporter.h"

class SimCity4LotTeleporter : public MaxisTeleporter
{
public:
	SimCity4LotTeleporter();

	bool StartDownload( const string &scriptPath, const string &assetID );
	bool StartUpload( const string &scriptPath, const string &lotPath );
	static string Test( string serverName );
	static string TestDownload( string serverName );
	static string TestUpload( string serverName );

protected:
	virtual bool SetReqBodyWithAsset( const string &lotFilePath );
};


#endif // !defined(AFX_SIMCITY4LOTTELEPORTER_H__EE46E844_7A18_428D_9A7A_74CA007D741A__INCLUDED_)
