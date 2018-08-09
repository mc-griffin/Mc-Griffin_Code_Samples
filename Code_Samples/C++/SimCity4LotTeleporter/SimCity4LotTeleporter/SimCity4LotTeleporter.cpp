// SimCity4LotTeleporter.cpp: implementation of the SimCity4LotTeleporter class.
//
//////////////////////////////////////////////////////////////////////
#include "SimCity4LotTeleporter.h"
#include "FileUtils.h"
#include "HttpUri.h"
#include "SimCity4Config.h"


static const string s_title = "Maxis SimCity4 City Teleporter";
static const string s_boundary = "------M-A-X-I-S--S-I-M-C-I-T-Y-";
static const DWORD s_maxUploadSize = 52428800; // this is arbitrary

SimCity4LotTeleporter::SimCity4LotTeleporter()
	: MaxisTeleporter( s_title, s_boundary, s_maxUploadSize, "" )
{}


bool SimCity4LotTeleporter::StartDownload( const string &scriptPath, const string &assetID )
{
	LOG( "SimCity4LotTeleporter::StartDownload(" + scriptPath + "," + assetID + ")" );

	if ( scriptPath.empty() || assetID.empty() )
		{ assert(false); return false; }

	// set url, for example:
	// /scape_dev/teleport/down/dl_manager.php?assetID=123&login_key=3ba24b49f9a29a735c35e3bd614f74aa&transaction_id=123456789ABC
	HttpUri uri;
	uri.SetUrl( scriptPath );
	uri.AddModifier( string( "assetID=" + assetID ));
	string loginKey = GetLoginKey();
	if ( loginKey.empty())
	{
		Exception e(
			"No login key found in SimCity4LotTeleporter::StartDownload",
			"You must be logged in as a product-registered user "
				"to use the exchange." );
		e.TellUser( _reqResp._title );
		return false;
	}
	uri.AddModifier( loginKey );
	if ( SetTransactionId())
		uri.AddModifier( GetTransactionIdWithName());

	return MaxisTeleporter::StartDownload( uri.GetString(), HttpReqResp::HRR_DUAL,
								HttpReqResp::HRR_WININET, true );
}

bool SimCity4LotTeleporter::StartUpload( const string &scriptPath, const string& lotPath )
{
	LOG( string( "SimCity4LotTeleporter::StartUpload(" )
		+ scriptPath + "," + lotPath + ")" );

	if ( scriptPath.empty() || lotPath.empty())
		{ assert(false); return false; }

	string url = scriptPath;
	string loginKey = GetLoginKey();
	if ( loginKey.empty())
	{
		Exception e(
			"No login key found in SimCity4LotTeleporter::StartDownload",
			"You must be logged in as a product-registered user "
				"to use SimCityScape." );
		e.TellUser( _reqResp._title );
		return false;
	}
	url += "?";
	url += loginKey;

	if ( SetTransactionId())
		url += ( "&" + GetTransactionIdWithName());

	return MaxisTeleporter::StartUpload( url, lotPath, HttpReqResp::HRR_DUAL,
						HttpReqResp::HRR_WININET, MaxisTeleporter::MT_OLD_BODY );
}

bool SimCity4LotTeleporter::SetReqBodyWithAsset( const string &lotFilePath )
{
	LOG( "SimCity4LotTeleporter::SetReqBodyWithAsset(" + lotFilePath + ")" );

	if ( lotFilePath.empty()) { assert(false); return false; }

	string dir;
	string lotFileNameSansExt;
	if ( !SplitFilePath( lotFilePath, &dir, &lotFileNameSansExt, NULL ))
		{ assert(false); return false; }

	const string lot = dir + "\\" + lotFileNameSansExt + ".SC4Lot";

	// APP now we now put thumbnail and xml in temp directory
	char* tmpPath = new char[_MAX_PATH]; 
	int tmpPathSize = GetTempPath(_MAX_PATH, tmpPath);
	if (tmpPathSize > _MAX_PATH) {
		assert(false);
		return false; 
	}
	const string thumb = string(tmpPath) + lotFileNameSansExt + ".png";
	const string xml = string(tmpPath) + lotFileNameSansExt + ".xml";
	delete[] tmpPath;

	// make sure all files exist
	try
	{
		if (! FileExists(lot) )
			throw Exception(
				"SimCity4LotTeleporter::SetReqBodyWithAsset: lot doesn't exist",
				lot );
		if (! FileExists(thumb) )
			throw Exception(
				"SimCity4LotTeleporter::SetReqBodyWithAsset: thumbnail doesn't exist",
				thumb );
		if (! FileExists(xml) )
			throw Exception(
				"SimCity4LotTeleporter::SetReqBodyWithAsset: xml doesn't exist",
				xml );
		
	}
	catch ( const Exception &e )
	{
		e.TellUser( _reqResp._title );
		return false;
	}

	// fill body
	PostRequest *post = static_cast<PostRequest *>( _reqResp._request );
	RequestBody &body = post->_body;

	// append "asset_type"
	body.Append( "asset_type", "LOT");

	// append "count_files"
	string countFiles = "3"; 
	body.Append( "count_files", countFiles );

	body.AppendFile( "sc4lot", lot );
	body.AppendFile( "thumb", thumb );
	body.AppendFile( "xml", xml );

	body.AppendActionUpload();

	return true;
}


string SimCity4LotTeleporter::Test( string serverName )
{
	Logger::Start( "SOFTWARE\\Maxis\\SimCity4", "SimCity4LotTeleporter.txt" );

	string log = "Test SimCity4LotTeleporter\n";

	for ( int i = 0; i < 1; ++i )
	{
		log += TestUpload( serverName );
		log += TestDownload( serverName );
	}

	return log;
}

// APP todo this depends on server
string SimCity4LotTeleporter::TestDownload( string serverName )
{
	SimCity4Config sc4Config;
	sc4Config.Initialize();
	string sc4Dir = sc4Config.GetInstallPath();

	SimCity4LotTeleporter t;

	string log;

	Server server;
	if ( !server.SetNameAddress( serverName ))
		log += "Server::Initialize() failed\n";

	if ( !t.Initialize( &server ))
		log += "Initialize() failed\n";

	// example: GET /scape/teleport/down/dl_manager.php?regionID=17&tileNo=26&turn=-1&login_key=d2eecc8c0086d0eb7895da075744bacc&transaction_id=123456789ABC
	string scriptPath = "/scape/teleport/down/dl_manager.php";
	string assetId = "17";
	bool download = t.StartDownload( scriptPath, assetId );
	if ( !download )
		log += "StartDownload() failed\n";

	while ( t.GetPercentComplete() != "100" ) 
		Sleep( 500 );

	SfpSilentlyOverWrite silentlyOverWrite;
	string outcomeCode = t.FinishTransport( sc4Dir, silentlyOverWrite );
	if ( outcomeCode.empty())
		log += "FinishTransport() failed\n";

	if ( !t.WasSuccessful())
		log += "WasSuccessful() returned false\n";

	string filePath;
	{
		// set filePath from Content-Disposition header
		{
			string fileInfo =
				t._reqResp._response._headers.FindOnly( "Content-Disposition" );
			int nameStarts = fileInfo.find( "filename=" );
			nameStarts += strlen("filename=");
			int i = nameStarts + 1; // why + 1?
			while ( i < fileInfo.size())
			{
				char c = fileInfo.at( i );
				if ( c == ';' )
					break;
				++i;
			}
			filePath = sc4Dir;
			filePath += t._relativeImportPath;
			if ( fileInfo.empty())
				filePath.erase();
			else

				filePath += fileInfo.substr( nameStarts, i - nameStarts );
		}

	}
	if ( FileExists( filePath ))
	{
		BOOL del = DeleteFile( filePath.c_str()); assert( del );
	}
	else
		log += "SaveBodyAsFile() failed\n";

	return log;
}


string SimCity4LotTeleporter::TestUpload( string serverName )
{
	SimCity4LotTeleporter t;

	string log;
	
	Server server;
	if ( !server.SetNameAddress( serverName ))
		log += "Server::Initialize() failed\n";

	if ( !t.Initialize( &server ))
		log += "Initialize() failed\n";
	
	string scriptPath = "/teleport/unleashed/LOT/up/ul_manager.php";
	string filePath = "C:\\Documents and Settings\\gmcclellan\\My Documents\\src\\Depot\\PlayerXProjects\\WebDevelopment\\ClientTech\\Teleporters\\UnleashedTeleporter\\SimCity4LotTeleporter\\SimCity4LotTeleporterTest\\Test Data\\L70.srf";
	bool upload = t.StartUpload( scriptPath, filePath );
	if ( upload )
		while ( t.GetPercentComplete() != "100" ) // bound the number of iterations?
			Sleep( 500 );
	else
		log += "StartUpload() failed\n";

	string outcomeCode = t.FinishUpload();
	if ( outcomeCode.empty())
		log += "FinishUpload() failed\n";
	string uploadPhrase = t.GetOutcomePhrase();
	if ( uploadPhrase.empty())
		log += "GetOutcomePhrase() failed\n";

	if ( !t.WasSuccessful())
		log += "WasSuccessful() returned false\n";

	if ( t.GetAccount().empty() || t.GetCode().empty())
		log += "GetAccount() or GetCode() failed\n";

	return log;
}




