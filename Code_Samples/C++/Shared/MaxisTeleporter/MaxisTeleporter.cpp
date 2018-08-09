// MaxisTeleporter.cpp: implementation of the MaxisTeleporter class.
//
//////////////////////////////////////////////////////////////////////

#include "MaxisTeleporter.h"
#include "FileUtils.h"
#include "Utilities.h"
#include "HttpUri.h"
#include <time.h>


// _successfulUploadCode must match what the server sends
const char *MaxisTeleporter::_successfulUploadCode = "0";
// _successfulDownloadCode and _canceledDownloadCode are just used
// by the client but should not conflict with any code sent by the server.
const char *MaxisTeleporter::_successfulDownloadCode = "108";
const char *MaxisTeleporter::_canceledDownloadCode = "109";

const string s_transactionIdName = "transaction_id";


MaxisTeleporter::MaxisTeleporter( const string &title, const string &boundary,
						DWORD maxUploadSize, const string &relativeImportPath )
	: _reqResp( title, boundary )
	, _maxUploadSize( maxUploadSize )
	, _direction( MaxisTeleporter::MT_NULL )
	, _relativeImportPath( relativeImportPath )
	, _wasCanceled( false )
	, _server( NULL )
	, _base64DecodeResponseBody( false )
{}


void MaxisTeleporter::Reset()
{
	_reqResp.Reset();
	if ( !_attributes.empty())
		_attributes.erase( _attributes.begin());
	_direction = MT_NULL;
	_wasCanceled = false;
	_transactionId = "";
	_base64DecodeResponseBody = false;
}


bool MaxisTeleporter::Initialize( const Server *server )
{
	if ( server == NULL || !server->IsSet()) { assert(false); return false; }
	_server = server;
	return true;
}


bool MaxisTeleporter::StartDownload( const string &url,
						HttpReqResp::Threading thr,
							HttpReqResp::Transport transport /*=HRR_WININET*/,
								bool base64DecodeResponseBody /*=false*/)
{
	LOG( string( "MaxisTeleporter::StartDownload(" ) + url + ")" );

	if ( url.empty()) { assert(false); return false; }

	if ( !CreateDownloadRequest())
		return false;

	_base64DecodeResponseBody = base64DecodeResponseBody;

	_reqResp._request->SetUrl( url );

	// SetMinimalHeaders() requires that Generate() be called first
	_reqResp._request->Generate();

	DWORD count = _reqResp._request->SetMinimalHeaders();
	assert( 0 < count );

	bool cookie = _reqResp.IncludeCookie();
	assert( cookie );

	if ( !IsTransportReady())
		{ assert(false); return false; }

	short outcome = HRR_INVALID_STATE;
	try
	{
		outcome = _reqResp.SendReceive( thr, transport );
	}
	catch ( const Exception &e )
	{
		e.TellUser( _reqResp._title, GetTransactionId());
	}

	return outcome == HRR_OK;
}


bool MaxisTeleporter::StartUpload( const string &url, const string &filePathIn,
						HttpReqResp::Threading thr,
							HttpReqResp::Transport transport /*=HRR_WININET*/,
								BodyFormat bodyFormat /*=MT_FORM_MIME*/)
{
	LOG( string( "MaxisTeleporter::StartUpload(" ) + url + ")" );

	string filePath = MakeWinPath( filePathIn );

	short outcome = HRR_INVALID_STATE;
	try
	{
		if ( url.empty())
			throw Exception( "Input parameter empty in MaxisTeleporter::StartUpload",
									"Internal error 100." );

		if ( !CreateUploadRequest( bodyFormat == MT_OLD_BODY ))
			throw Exception( "CreateUploadRequest failed "
								"in MaxisTeleporter::StartUpload",
									"Internal error 101." );

		bool set = SetReqBodyWithAsset( filePath );
		assert(set);

		_reqResp._request->SetUrl( url );

		// Generate() must be called before SetMinimalHeaders() and GetBodyLength()
		_reqResp._request->Generate( bodyFormat == MT_FORM_MIME_BASE64 );

		DWORD count = _reqResp._request->SetMinimalHeaders();
		assert( 0 < count );

		DWORD bodySize =  _reqResp._request->GetBodyLength();
		if ( _maxUploadSize < bodySize )
		{
			string error = "The asset you are trying to upload is too large.\n";
			error += "The maximum upload size is ";
			char maxUpload[32] = "";
			sprintf( maxUpload, "%u", _maxUploadSize / 1024 );
			error += maxUpload;
			error += "K.";
			throw Exception( "MaxisTeleporter::StartUpload", error );
		}

		bool cookie = _reqResp.IncludeCookie();
		assert( cookie );

		if ( !_reqResp.IsTransportReady())
			throw Exception( "_reqResp.IsTransportReady() failed "
								"in MaxisTeleporter::StartUpload",
									"Internal error 103." );

		outcome = _reqResp.SendReceive( thr, transport );
	}
	catch ( const Exception &e )
	{
		e.TellUser( _reqResp._title, GetTransactionId());
	}

	return outcome == HRR_OK;
}


string MaxisTeleporter::FinishTransport( const string &installPath,
							SaveFilePolicy &policy )
{
	string log = "MaxisTeleporter::FinishTransport( ";
	log += ( installPath + " )" );
	LOG( log );

	try
	{
		if ( _direction != MT_UPLOAD && _direction != MT_DOWNLOAD )
			throw "invalid _direction";

		if ( !_reqResp._server->IsSet())
			throw "server not set";

		if ( !_reqResp.IsReceiveDone())
		{
			short wait = WaitForTransportEnd();
			if ( wait == HttpReqResp::HRR_TIMEOUT  || IsHrrError( wait ))
				throw "WaitForTransport returned HRR_TIMEOUT or error";
		}

		short percent = _reqResp.GetPercentReceive();
		if ( percent != 100 )
			throw "GetPercentReceive did not return 100";

		// write cookie
		DWORD cookies = _reqResp.SetCookies();
		assert( 0 <= cookies );
	}
	catch ( LPCSTR error )
	{
		if ( error == NULL )
			{ assert(false); return "-1"; }

		string msg = "Error in MaxisTeleporter::FinishTransport: ";
		msg += error;
		LOG( error );
		_code = "-2";
		return _code;
	}
	catch (...)
	{
		LOG( "non-Exception exception in MaxisTeleporter::FinishTransport" );
		assert(false);
		_code = "-3";
		return _code;
	}

	// download has no outcome code, so if we just downloaded, return success
	if ( _direction == MT_DOWNLOAD )
	{
		SaveOutcome saveBody = MT_ERROR;
		try
		{
			saveBody = SaveBodyAsFile( installPath, policy, _base64DecodeResponseBody );
			if ( saveBody == MT_ERROR )
			{
				LOG( "MaxisTeleporter::FinishTransport: SaveBodyAsFile failed" );
				char buf[128] = "";
				DWORD bufLen = min( 128, _reqResp._response.GetBodyLength());
				memcpy( buf, _reqResp._response.GetBody(), bufLen );
				string msg = "MaxisTeleporter::FinishTransport: first part of body: ";
				msg += buf;
				LOG( msg );
				assert(false);
				return "-4";
			}

			// set _code, see MaxisTeleporter1.cpp
			if ( saveBody == MT_SUCCESS )
				_code = _successfulDownloadCode;
			else if ( saveBody == MT_CANCEL )
				_code = _canceledDownloadCode;
			else
			{
				_code = "-1";
				assert(false);
			}
		}
		catch ( const Exception &e )
		{
			e.TellUser( _reqResp._title, GetTransactionId());
			_code = "-1";
			return _code;
		}

		return _code;
	}

	const string &body = _reqResp._response.GetBody();
	if ( body.empty())
		return "-5";

	string codeAccount = ExtractCodeAccount( body );
	if ( codeAccount.empty())
	{
		LOG( "MaxisTeleporter::FinishTransport: ExtractCodeAccount failed" );
		string bodyPart = body.substr( 0, 64 );
		string msg = "MaxisTeleporter::FinishTransport: first part of body: ";
		msg += bodyPart;
		LOG( msg );
		assert(false);
		return "-6";
	}

	LOG( "Extracted from body: " + codeAccount );

	// parse "code [account]" then set _code and _account
	if ( !ParseCodeAccount( codeAccount, &_code, &_account ))
		{ assert(false); return "-7"; }

	return _code;
}


string MaxisTeleporter::FinishUpload()
{
	assert( _direction == MT_UPLOAD );
	SfpNullPolicy null;
	return FinishTransport( "this shouldn't matter", null );
}


string MaxisTeleporter::GetOutcomePhrase() const
{
	return MaxisTeleporter1::GetOutcomePhrase( GetCode());
}


// only call this after FinishTransport() is called
bool MaxisTeleporter::WasSuccessful() const
{
	if ( !_reqResp.WasTransportSuccessful())
		{ assert(false); return false; }

	if ( _direction != MT_UPLOAD && _direction != MT_DOWNLOAD )
		{ assert(false); return false; }

	// _code being set assumes that FinishTransport() has been called
	if ( _direction == MT_UPLOAD && _code != _successfulUploadCode )
		return false;

	if ( _direction == MT_DOWNLOAD && _code != _successfulDownloadCode )
		return false;

	return true;
}


string MaxisTeleporter::GetAccount() const
{
	// for now, server only passes this info back for upload?
	assert( _direction == MT_UPLOAD );
	return _account;
}


string MaxisTeleporter::GetCode() const
{
	// for now, server only passes this info back for upload?
	assert( _direction == MT_UPLOAD );
	return _code;
}


string MaxisTeleporter::GetCookie() const
{
	if ( _server == NULL || !_server->IsSet()) { assert(false); return ""; }
	return _server->GetCookie();
}


bool MaxisTeleporter::CreateUploadRequest( bool useOldBody /*=false*/)
{
	if ( _server == NULL || !_server->IsSet()) { assert(false); return false; }

	_direction = MT_UPLOAD;
	return _reqResp.CreateRequest( _server, HttpReqResp::HRR_POST, useOldBody );
}


bool MaxisTeleporter::CreateDownloadRequest()
{
	if ( _server == NULL || !_server->IsSet()) { assert(false); return false; }

	_direction = MT_DOWNLOAD;
	return _reqResp.CreateRequest( _server, HttpReqResp::HRR_GET );
}


bool MaxisTeleporter::IsTransportReady() const
{
	return _reqResp.IsTransportReady();
}


DWORD MaxisTeleporter::SetCookies() const
{
	return _reqResp.SetCookies();
}


bool MaxisTeleporter::IncludeCookie()
{
	return _reqResp.IncludeCookie();
}


string MaxisTeleporter::GetLoginKey() const
{
	return GetLoginKeyFromCookie( GetCookie());
}


string MaxisTeleporter::GetLoginKeyFromCookie( const string &cookie )
{
	// set loginKey
	string loginKey;
	try
	{
		if ( cookie.empty())
		{
			// HEY!  This error message needs to progagate back to the user!
			// Use Exception::TellUser() for this error.
			throw "No Cookie!";
		}

		int mxlg = cookie.find( "MXLG=" );

		if ( mxlg == string::npos )
			throw "No MXLG= found in MaxisTeleporter::GetLoginKey()";

		int endMxlg = cookie.find( ";", mxlg + 1 );
		// endMxlg == string::npos => login key is at end of mxlg,

		mxlg += 5;
		if ( endMxlg == string::npos )
			loginKey = cookie.substr( mxlg, cookie.size() - mxlg );
		else
			loginKey = cookie.substr( mxlg, endMxlg - mxlg );
		if ( loginKey.empty())
			throw "No login key found in MaxisTeleporter::GetLoginKey()";
	}
	catch ( LPCSTR err )
	{
		string msg = "MaxisTeleporter::GetLoginKey: ";
		msg += err;
		LOG( msg );
		return "";
	}

	assert( !loginKey.empty());
	return string( "login_key=" ) + loginKey;
}


bool MaxisTeleporter::SetAttribute( const string &name, const string &value )
{
	NameValuePair p;
	bool set = p.Set( name, value );
	if ( !set ) return false;
	_attributes.push_back( p );
	return true;
}


string MaxisTeleporter::GetPercentComplete() const
{
	if ( _direction != MT_UPLOAD && _direction != MT_DOWNLOAD )
		{ assert(false); return ""; }

	short percent = 0;
	switch( _direction )
	{
		case MT_UPLOAD:
			percent = _reqResp.GetPercentSend();
			break;

		case MT_DOWNLOAD:
			percent = _reqResp.GetPercentReceive();
			break;

		default:
			assert(false);
			return "";
	}
	if ( percent < 0 || 100 < percent ) { assert(false); return ""; }

	char szPercent[4] = "";
	sprintf( szPercent, "%d", percent );

	return szPercent;
}


bool MaxisTeleporter::IsSendDone() const
{
	return _reqResp.IsSendDone();
}


bool MaxisTeleporter::IsReceiveDone() const
{
	return _reqResp.IsReceiveDone();
}


DWORD MaxisTeleporter::GetBytesSent() const
{
	return _reqResp.GetBytesSent();
}


DWORD MaxisTeleporter::GetBytesReceived() const
{
	return _reqResp.GetBytesReceived();
}


short MaxisTeleporter::WaitForTransportEnd()
{
	if ( _direction != MT_UPLOAD && _direction != MT_DOWNLOAD )
		{ assert(false); return HRR_INVALID_STATE; }

	if ( !_reqResp.IsSet())
		{ assert(false); return HRR_INVALID_STATE; }

	// set contentSize
	DWORD contentSize = 0; // in bytes
	switch( _direction )
	{
		case MT_UPLOAD:
			contentSize = _reqResp._request->GetBodyLength();
			break;

		case MT_DOWNLOAD:
		{
			string s = "MaxisTeleporter::WaitForTransportEnd: ";

			DWORD i = 0;
			// check for 5 seconds
			while ( i < 50 )
			{
				if ( _reqResp._response.GetExpectedContentLength( &contentSize ))
				{
					assert( _reqResp.IsHeadReceived());
					break;
				}

				if ( i == 0 )
					LOG( s + "sleep-check cycle for 5 seconds" );
				
				bool pump = HttpReqResp::PumpMessages( 100 );
				if ( !pump )
					return HttpReqResp::HRR_QUIT;

				++i;
			}

			if ( i < 50 )
				LOG( s + "got body length: " + NumberToString( contentSize ));
			else
			{
				LOG( s + "body length is unknown, set wait time to 60s" );
				return _reqResp.WaitForTransportEnd( 60000 );
			}
		}
			break;

		default:
			assert(false);
			return HRR_INVALID_STATE;
	}

	// what MSDN says about converting from double to DWORD:
	// "Convert to long; convert long to unsigned long"

	DWORD waitTime = (DWORD) HttpReqResp::WaitTime( contentSize ); // in milliseconds

	return _reqResp.WaitForTransportEnd( waitTime );
}


MaxisTeleporter::SaveOutcome MaxisTeleporter::SaveBodyAsFile(
				const string &installPath, SaveFilePolicy &policy,
				bool decodeBase64 /*=true*/)
{
	LOG( "MaxisTeleporter::SaveBodyAsFile");

	// set filePath from response headers
	string filePath;
	try
	{
		if ( _direction != MT_DOWNLOAD 
				|| !_reqResp.IsSendDone() || _reqResp.GetPercentReceive() != 100 )
			throw "Bad state";

		if ( installPath.empty() || MAX_PATH <= installPath.length())
			throw "Invalid installPath";

		assert( !HasEndingBackSlash( installPath ));

		string fileName = _reqResp._response._headers.GetDownloadedFileName();
		if ( fileName.empty()) // we check file length later in this function
			throw "No valid file name";

		filePath = installPath;
		if ( !_relativeImportPath.empty())
		{
			if ( !HasEndingBackSlash( filePath )
					&& _relativeImportPath.find( "\\" ) != 0 )
				filePath += "\\";
			filePath += _relativeImportPath;
		}

		if ( !HasEndingBackSlash( filePath ))
			filePath += "\\";

		filePath += fileName;
	}
	catch ( LPCSTR s )
	{
		string error = "MaxisTeleporter::SaveBodyAsFile: ";
		error += s;
		LOG( error );
		assert(false);
		return MT_ERROR;
	}
	catch ( const Exception &e )
	{
		e.TellUser( _reqResp._title, GetTransactionId());
		return MT_ERROR;
	}
	catch (...)
	{
		LOG( "Unexpected exception in MaxisTeleporter::SaveBodyAsFile" );
		assert(false);
		return MT_ERROR;
	}

	if ( filePath.empty() || MAX_PATH <= filePath.length())
	{
		LOG( "empty or illegal file name in MaxisTeleporter::SaveBodyAsFile" );
		assert(false);
		return MT_ERROR;
	}

	policy.SetFilePath( filePath );
	SaveFilePolicy::Outcome outcome = policy.Apply();
	if ( outcome == SaveFilePolicy::SFP_CANCEL )
	{
		_wasCanceled = true;
		return MT_CANCEL;
	}
	if ( outcome != SaveFilePolicy::SFP_PROCEED )
		{ assert(false); return MT_ERROR; }

	string selectedFilePath = policy.GetSelectedFilePath();
	if ( selectedFilePath.empty() || MAX_PATH <= selectedFilePath.length())
		{ assert(false); return MT_ERROR; }

	bool result = _reqResp._response.SaveBodyAsFile( selectedFilePath, decodeBase64 );
	if ( result == false )
		return MT_ERROR;

	return MT_SUCCESS;
}


bool MaxisTeleporter::WasCanceled() const
{
	return _wasCanceled;
}


string MaxisTeleporter::GenerateTransactionId()
{
	// generate a random hex value to distinguish the user from other users
	char randomNumberInHex[8] = "";
	srand((unsigned) time(NULL));
	// rand generates a number from [0..RAND_MAX-1], where RAND_MAX is
	// 2^15 = 32768 (stored in 2 bytes)
	int digits = sprintf( randomNumberInHex, "%04X", rand());
	if ( digits != 4 || randomNumberInHex[0] == '\0' )
		{ assert(false); return "-1"; }

	// generate UNIX timestamp
	time_t ltime = 0;
    /* Set time zone from TZ environment variable. If TZ is not set,
     * the operating system is queried to obtain the default value 
     * for the variable. 
     */
    _tzset();
    // Get UNIX-style time
    time( &ltime );
	char timeInHex[16] = "";
	digits = sprintf( timeInHex, "%08X", ltime );
	if ( digits != 8 || timeInHex[0] == '\0' )
		{ assert(false); return "-2"; }

	string transactionId = string( randomNumberInHex ) + string( timeInHex );
	return transactionId;
}


bool MaxisTeleporter::SetTransactionId()
{
	_transactionId = "";
	_transactionId = GenerateTransactionId();
	_reqResp.SetTransactionId( _transactionId );
	return _transactionId.size() == 12;
}


string MaxisTeleporter::GetTransactionId() const
{
	// assert( !_transactionId.empty());
	return _transactionId;
}


string MaxisTeleporter::GetTransactionIdWithName() const
{
	if ( _transactionId.empty()) { assert(false); return ""; }
	return string( "transaction_id=" ) + GetTransactionId();
}


// We must subclass MaxisTeleporter to override SetReqBodyWithAsset().

class MaxisTeleporterTest : public MaxisTeleporter
{
public:
	MaxisTeleporterTest( const string &t, const string &b, DWORD m, const string &p )
		: MaxisTeleporter( t, b, m, p )
	{}

	bool SetReqBodyWithAsset( const string &filePath );
};


string MaxisTeleporter::Test( bool testNetwork, string serverName, HINSTANCE inst )
{
	if ( !testNetwork )
		return "Test MaxisTeleporter requires network connection\n";

	string log = "Test MaxisTeleporter\n";

	for ( int i = 0; i < 10; ++i )
	{
		log += TestDownloadLot( serverName, inst );
		log += TestUploadLot( serverName );
	}

	string title = "MaxisTeleporter";
	string boundary = "----------M-A-X-I-S--T-E-L-E-P-O-R-T-E-R--";
	DWORD maxUploadSize = 1048576; // 1 MG

	MaxisTeleporter t( title, boundary, maxUploadSize, "" );

	// SetAttribute() and GetAttributes()
	{
		if ( !t.SetAttribute( "a", "b" ) || !t.SetAttribute( "c", "d" ))
			log += "SetAttribute() failed\n";

		const list<NameValuePair> &as = t.GetAttributes();
		if ( as.size() != 2 )
			log += "GetAttributes() failed\n";

		string s = as.begin()->Name();
		if ( s != "a" && s != "c" )
			log += "SetAttribute() or GetAttributes() failed\n";
	}

	Server server;
	if ( !server.SetNameAddress( serverName ))
		log += "Server::SetNameAddress() failed\n";

	if ( !t.Initialize( &server ))
		log += "Initialize() failed\n";

	// GetLoginKey & GetCookie
	{
		string loginKey = "login_key=";
		string cookie;
		{
			string cookieUrl = "http://";
			cookieUrl += serverName;
			char cookieData[512] = "";
			DWORD cookieSize = 511;
			InternetGetCookie( cookieUrl.c_str(), NULL, cookieData, &cookieSize );
			cookie = cookieData;
		}

		if ( cookie != t.GetCookie())
			log += "GetCookie() failed\n";

		int mxlg = cookie.find( "MXLG=" );
		int endMxlg = cookie.find( ";", mxlg + 1 );
		mxlg += 5;
		if ( endMxlg == string::npos )
			loginKey += cookie.substr( mxlg, cookie.size() - mxlg );
		else
			loginKey += cookie.substr( mxlg, endMxlg - mxlg );

		if ( loginKey != t.GetLoginKey())
			log += "GetLoginKey() failed\n";
	}

	log += TestUploadLotTwice( serverName );

	log += MaxisTeleporter1::Test();

	{
		MaxisTeleporter tele( title, boundary, maxUploadSize, "" );
		Server s;
		if ( tele.Initialize( &s ))
			log += "Initialize() failed\n";
	}

	// SfpFilePolicy and descendents
	{
		SfpConflictAlert::Test();
		bool not = SfpDoNotOverWrite::Test(); assert( not );
		SfpSaveAs::Test( inst );
	}

	return log;
}


string MaxisTeleporter::TestDownloadLot( string serverName, HINSTANCE inst )
{
	string title = "MaxisTeleporter";

	MaxisTeleporterTest t( title, "", 0, "Test Data" );

	string log;

	Server server;
	try
	{
		if ( !server.SetNameAddress( serverName ))
			log += "SetNameAddress() failed\n";

		if ( !t.Initialize( &server ))
			log += "Initialize() failed\n";
	}
	catch ( Exception &e )
	{
		e.TellUser( title, t.GetTransactionId());
		log += ( e.GetFriendlyPhrase() + "\n" );
	}
	catch (...)
	{
		log += "Unexpected exception from Server method\n";
	}

	if ( !t.SetTransactionId())
		log += "SetTransactionId() failed\n";

	string transId = t.GetTransactionId();
	if ( transId.empty() || transId == "-1" || transId == "-2" )
		log += "GetTransactionId() failed\n";

	HttpUri uri;
	uri.SetUrl( "/teleport/hotdate/dl_manager.php" );
	uri.AddModifier( "asset_id=1682" );
	uri.AddModifier( "asset_type=LOT" );
	uri.AddModifier( t.GetLoginKey());
	uri.AddModifier( t.GetTransactionIdWithName());

	if ( !t.StartDownload( uri.GetString(), HttpReqResp::HRR_DUAL ))
		log += "StartDownload() failed\n";

	string percent = t.GetPercentComplete();
	while (( percent.length() == 1 || percent.at(0) < '7' ) && percent.length() < 3 )
	{
		HttpReqResp::PumpMessages( 100 );

		// don't know what the answers should be, but let's cause some trouble
		t._reqResp.IsSendDone();
		t._reqResp.IsReceiveDone();
		t._reqResp.GetBytesSent();
		t._reqResp.GetBytesReceived();

		percent = t.GetPercentComplete();
	}

	short wait = t.WaitForTransportEnd();
	if ( wait != HttpReqResp::HRR_END )
		log += "WaitForTransportEnd() failed\n";

	if ( !t._reqResp.IsSendDone() || !t._reqResp.IsReceiveDone())
		log += "IsSendDone() or IsReceiveDone() failed\n";

	if ( t._reqResp.GetBytesSent() != 0 || t._reqResp.GetBytesReceived() == 0 )
		log += "GetBytesSent() or GetBytesReceived() failed\n";

//	if GETs are done quickly in succession, the web server apparently stops sending Set-Cookie:s
//	if ( t.SetCookies() == 0 )
//		log += "SetCookies() failed\n";

	char installPath[MAX_PATH+1] = "";
	GetCurrentDirectory( MAX_PATH, installPath );	

	//SfpSaveAs policy;
	//policy.SetInst( inst );
	//policy.SetTitle( "MaxisTeleporter::TestDownloadLot()" );
	//
	//SfpDoNotOverWrite policy;
	SfpSilentlyOverWrite policy;

	if ( t.SaveBodyAsFile( installPath, policy ))
	{
		string path = installPath;
		path += "\\";
		path += t._relativeImportPath;
		path += "\\";
		// path += t._reqResp._response.GetDownloadedFileName();
		// DeleteTheFile( path );
	}
	else
		log += "SaveBodyAsFile() failed\n";
/*
	DWORD bodyLength = 0;
	char currDir[MAX_PATH+1] = "";
	DWORD len = GetCurrentDirectory( MAX_PATH, currDir );
	assert( 0 < len && !HasEndingBackSlash( currDir ));
	string filePath = currDir;

	string outcomeCode = FinishTransport( filePath );
*/
	
	return log;
}


string MaxisTeleporter::TestUploadLot( string serverName )
{
	string title = "MaxisTeleporter";
	string boundary = "--------M-A-X-I-S--H-O-T--D-A-T-E--L-O-T---";
	DWORD maxUploadSize = 1048576; // 1 MG

	MaxisTeleporterTest t( title, boundary, maxUploadSize, "" );

	string log;

	Server server;
	try
	{
		if ( !server.SetNameAddress( serverName ))
			log += "SetNameAddress() failed\n";

		if ( !t.Initialize( &server ))
			log += "Initialize() failed\n";
	}
	catch ( Exception &e )
	{
		e.TellUser( title, t.GetTransactionId());
		log += ( e.GetFriendlyPhrase() + "\n" );
	}
	catch (...)
	{
		log += "Unexpected exception from Server method\n";
	}

	if ( !t.SetTransactionId())
		log += "SetTransactionId() failed\n";

	string transId = t.GetTransactionId();
	if ( transId.empty() || transId == "-1" || transId == "-2" )
		log += "GetTransactionId() failed\n";

	HttpUri uri;
	uri.SetUrl( "/teleport/hotdate/ul_manager.php" );
	uri.AddModifier( t.GetLoginKey());
	uri.AddModifier( t.GetTransactionIdWithName());

	string filePath;
	{
		char currDir[MAX_PATH+1] = "";
		DWORD len = GetCurrentDirectory( MAX_PATH, currDir );
		assert( 0 < len && !HasEndingBackSlash( currDir ));

		filePath = currDir;
 		filePath += "\\Test Data\\PostBody.txt";

		assert( FileExists( filePath ));
	}

	if ( !t.StartUpload( uri.GetString(), filePath, HttpReqResp::HRR_DUAL,
							HttpReqResp::HRR_WININET, MaxisTeleporter::MT_OLD_BODY ))
		log += "StartUpload() failed\n";

	string percent = t.GetPercentComplete();
	while (( percent.length() == 1 || percent.at(0) < '7' ) && percent.length() < 3 )
	{
		HttpReqResp::PumpMessages( 100 );

		// don't know what the answers should be, but let's cause some trouble
		t._reqResp.IsSendDone();
		t._reqResp.IsReceiveDone();
		t._reqResp.GetBytesSent();
		t._reqResp.GetBytesReceived();

		percent = t.GetPercentComplete();
	}

	string outcomeCode = t.FinishUpload();
	if ( outcomeCode != _successfulUploadCode )
		log += "FinishTransport() failed\n";

	if ( !t._reqResp.IsSendDone() || !t._reqResp.IsReceiveDone())
		log += "IsSendDone() or IsReceiveDone() failed\n";

	if ( t._reqResp.GetBytesSent() == 0 || t._reqResp.GetBytesReceived() == 0 )
		log += "GetBytesSent() or GetBytesReceived() failed\n";

	return log;
}


bool MaxisTeleporterTest::SetReqBodyWithAsset( const string &filePath )
{
	if ( filePath.empty() || MAX_PATH <= filePath.length())
		{ assert(false); return false; }

	HANDLE hFile = CreateFile( filePath.c_str(), GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL );
	assert( hFile != INVALID_HANDLE_VALUE );

	const DWORD bodyBufSize = 800000;
	char bodyBuf[ bodyBufSize ];

	DWORD bytesRead = 0;
	BOOL readed = ReadFile( hFile, bodyBuf, bodyBufSize,
								&bytesRead, NULL );
	assert( readed );

	DWORD endBoundaryLen = strlen( "\r\n--------M-A-X-I-S--H-O-T--D-A-T-E--L-O-T---" );
	DWORD endOfLot = bytesRead - endBoundaryLen;

	bodyBuf[ endOfLot ] = '\0';

	PostRequest *post = static_cast<PostRequest *>( _reqResp._request );
	post->_body.Set( bodyBuf );

assert(false);
// The previous Set() will cause a crash in StartUpload() because there
// StartUpload() calls Generate() and Generate() should never be called
// after a Set().

	BOOL closed = CloseHandle( hFile ); assert( closed );

	assert( endOfLot == _reqResp._request->GetBodyLength());
	return true;
}


string MaxisTeleporter::TestUploadLotTwice( string serverName )
{
	string title = "MaxisTeleporter";
	string boundary = "--------M-A-X-I-S--H-O-T--D-A-T-E--L-O-T---";
	DWORD maxUploadSize = 1048576; // 1 MG

	MaxisTeleporterTest t( title, boundary, maxUploadSize, "" );

	string log;

	Server server;
	if ( !server.SetNameAddress( serverName ))
		log += "SetNameAddress() failed\n";

	if ( !t.Initialize( &server ))
		log += "Initialize() failed\n";

	if ( !t.SetTransactionId())
		log += "SetTransactionId() failed\n";

	HttpUri uri;
	uri.SetUrl( "/teleport/hotdate/ul_manager.php" );
	uri.AddModifier( t.GetLoginKey());
	uri.AddModifier( t.GetTransactionIdWithName());

	string filePath;
	{
		char currDir[MAX_PATH+1] = "";
		DWORD len = GetCurrentDirectory( MAX_PATH, currDir );
		assert( 0 < len && !HasEndingBackSlash( currDir ));

		filePath = currDir;
 		filePath += "\\Test Data\\PostBody.txt";

		assert( FileExists( filePath ));
	}

	// do it again

	if ( !t.StartUpload( uri.GetString(), filePath, HttpReqResp::HRR_DUAL ))
		log += "StartUpload() failed\n";

	string percent = t.GetPercentComplete();
	while (( percent.length() == 1 || percent.at(0) < '7' ) && percent.length() < 3 )
	{
		HttpReqResp::PumpMessages( 100 );

		// don't know what the answers should be, but let's cause some trouble
		t._reqResp.IsSendDone();
		t._reqResp.IsReceiveDone();
		t._reqResp.GetBytesSent();
		t._reqResp.GetBytesReceived();

		percent = t.GetPercentComplete();
	}

	string outcomeCode = t.FinishUpload();
	if ( outcomeCode != _successfulUploadCode )
		log += "FinishTransport() failed\n";

	if ( !t._reqResp.IsSendDone() || !t._reqResp.IsReceiveDone())
		log += "IsSendDone() or IsReceiveDone() failed\n";

	if ( t._reqResp.GetBytesSent() == 0 || t._reqResp.GetBytesReceived() == 0 )
		log += "GetBytesSent() or GetBytesReceived() failed\n";

	if ( !t.StartUpload( uri.GetString(), filePath, HttpReqResp::HRR_DUAL ))
		log += "StartUpload() failed\n";

	percent = t.GetPercentComplete();
	while (( percent.length() == 1 || percent.at(0) < '7' ) && percent.length() < 3 )
	{
		HttpReqResp::PumpMessages( 100 );

		// don't know what the answers should be, but let's cause some trouble
		t._reqResp.IsSendDone();
		t._reqResp.IsReceiveDone();
		t._reqResp.GetBytesSent();
		t._reqResp.GetBytesReceived();

		percent = t.GetPercentComplete();
	}

	outcomeCode = t.FinishUpload();
	if ( outcomeCode != _successfulUploadCode )
		log += "FinishTransport() failed\n";

	if ( !t._reqResp.IsSendDone() || !t._reqResp.IsReceiveDone())
		log += "IsSendDone() or IsReceiveDone() failed\n";

	if ( t._reqResp.GetBytesSent() == 0 || t._reqResp.GetBytesReceived() == 0 )
		log += "GetBytesSent() or GetBytesReceived() failed\n";

	return log;
}
