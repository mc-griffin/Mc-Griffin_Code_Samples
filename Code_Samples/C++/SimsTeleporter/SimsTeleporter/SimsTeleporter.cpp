// SimsTeleporter.cpp: implementation of the SimsTeleporter class.
//
//////////////////////////////////////////////////////////////////////

#include "SimsTeleporter.h"
#include "PackingList.h"
#include "PackingListParser.h"
#include "FileUtils.h"
#include "Utilities.h"
#include "Logger.h"
#include <process.h> // _beginthread()


static const string s_title = "Maxis Sims Teleporter";
static const string s_boundary = "------M-A-X-I-S--S-I-M-S-";
// I don't know what the maximum upload size should be.
// I used 50 MBs since that's what we used for HotDate NPCs.
static const DWORD s_maxUploadSize = 52428800;


struct DownloadData
{
	SimsDownloader *const _downloader;
	const string *const _scriptPath;
	const string *const _assetId;
	const string *const _simsPath;

	DownloadData( SimsDownloader *const downloader,
			const string *const scriptPath, const string *const assetId,
			const string *const simsPath )
	: _downloader( downloader )
	, _scriptPath( scriptPath )
	, _assetId( assetId )
	, _simsPath( simsPath )
	{
		assert( downloader && scriptPath && assetId && simsPath );
	}
};



// SimsUploader

SimsUploader::SimsUploader( const string &title, const string &boundary,
					DWORD maxUploadSize )
	: MaxisTeleporter( title, boundary, maxUploadSize, "this shouldn't be used" )
{}


bool SimsUploader::StartUpload( const string &scriptPath, const string &filePath,
									const string &subject )
{
	// The subject parameter is provided by the game.  It is used as an ID
	// by the database.

	LOG( string( "SimsUploader::StartUpload(" )
		+ scriptPath + "," + filePath + ")" );

	if ( scriptPath.empty() || filePath.empty() || subject.empty())
		{ assert(false); return false; }

	_subject = subject;

	HttpUri uri;
	uri.SetUrl( scriptPath );
	string loginKey = GetLoginKey();
	if ( loginKey.empty())
	{
		Exception e(
			"No login key found in SimsUploader::StartUpload",
			"You must be logged in as a product-registered user "
				"to use the exchange." );
		e.TellUser( _reqResp._title );
		return false;
	}
	uri.AddModifier( loginKey );

	return MaxisTeleporter::StartUpload( uri.GetString(), filePath,
		HttpReqResp::HRR_DUAL, HttpReqResp::HRR_WININET, MaxisTeleporter::MT_OLD_BODY );
}


bool SimsUploader::SetReqBodyWithAsset( const string &famFilePath )
{
	if ( famFilePath.empty()) { assert(false); return false; }

	if ( _subject.empty())
	{
		LOG( "SimsUploader::SetReqBodyWithAsset: _subject is empty" );
		assert(false);
		return false;
	}

	// set pub and pubPath
	string dir;
	if ( !SplitFilePath( famFilePath, &dir, NULL, NULL ))
		{ assert(false); return false; }

	const string pub = _subject + "." + PubFile::_fileExt;
	const string pubPath = dir + "\\" + pub;

	try
	{
		if ( !FileExists( famFilePath ))
		{
			string friendly = "Before you can upload this family, you must "
								"open the game and save the family.";
			string tech = "SimsUploader::SetReqBodyWithAsset: " + famFilePath
							+ " not found";
			throw Exception( tech, friendly );
		}
		
		if ( !FileExists( pubPath ))
			throw Exception( "PUB file not found.", "SimsUploader::SetReqBodyWithAsset" );

		PubFile pubFile;
		if ( !pubFile.Initialize( pubPath ))
			throw Exception(
					"PubFile::Initialize() failed in SimsUploader::SetReqBodyWithAsset",
					"The family you want to exchange is invalid." );

		const vector<PubFileImplicant> implicants = pubFile.GetFilesImpliedInPub();
		// log pub file implicants
		{
			string imps;
			for ( DWORD i = 0; i < implicants.size(); ++i )
			{
				const PubFileImplicant &imp = implicants[i];
				imps += ( imp.GetPostBodyRepresentation() + "\n" );
			}
			LOG( "SimsUploader::SetReqBodyWithAsset: Pub file implicants:\n" + imps );
		}

		DWORD fileCount = implicants.size() + 2; // fam, pub

		// prepare to fill body
		PostRequest *post = static_cast<PostRequest *>( _reqResp._request );
		RequestBody &body = post->_body;

		// append "asset_type"
		body.Append( "asset_type", "FAM" ); // check this

		// append "count_files"
		{
			string countFiles = NumberToString( fileCount );
			body.Append( "count_files", countFiles );
			LOG( "SimsUploader::SetReqBodyWithAsset: count_files is " + countFiles );
		}

		body.Append( "subject", _subject );
		LOG( "SimsUploader::SetReqBodyWithAsset: subject is " + _subject );
		
		body.AppendAsFileEncode( pubFile.GetString(), "pubfile", pub );

		string famFileTmpName =
					IsMakinMagicFamily( famFilePath ) ? "mtffile" : "famfile";

		body.AppendFile( famFileTmpName, famFilePath );

		AppendFilesImpliedInPubFile( body, pubFile );

		body.AppendActionUpload();
	}
	catch ( const Exception &e )
	{
		// e.TellUser( _reqResp._title );
		// return false;

		// Only MaxisTeleporter::StartUpload() calls this method, SetReqBodyWithAsset().
		// If MaxisTeleporter::StartUpload() receives false from this method, then
		// it gives the user a generic error message.  If I throw an Exception,
		// then MaxisTeleporter::StartUpload() displays it to the user.
		LOG( "SimsUploader::SetReqBodyWithAsset: throws " + e.GetTechnicalPhrase());
		throw e;
	}
	catch (...)
	{
		Exception e(
			"SimsUploader::SetReqBodyWithAsset: An unexpected exception occurred." );
		e.TellUser( _reqResp._title );
		return false;
	}

	return true;
}


bool SimsUploader::AppendFilesImpliedInPubFile( RequestBody &body, const PubFile &pubFile )
{
	SimsConfig sims;
	if ( !sims.Initialize()) { assert(false); return false; }

	// may be required by AlbumType?
	// assert( !sims.GetNeighNum().empty() && !sims.GetNeighPath().empty());

	const string simsPath = sims.GetSimsPath();

	const vector<PubFileImplicant> implicants = pubFile.GetFilesImpliedInPub();

	for ( DWORD i = 0; i < implicants.size(); ++i )
	{
		const PubFileImplicant &imp = implicants[i];

		string filePath = simsPath + imp.GetRelativePathFileName();
		if ( filePath.empty() || MAX_PATH <= filePath.length())
			{ assert(false); return false; }

		{
			string s = "AppendFilesImpliedInPubFile: append ";
			s += filePath;
			LOG( s );
		}

		switch ( imp.GetType())
		{
			case SkinsDirType:
				body.AppendFile( "skin", filePath );
				break;

			case WallsDirType:
				body.AppendFile( "wall", filePath );
				break;

			case FloorsDirType:
				body.AppendFile( "floor", filePath );
				break;

			case AlbumType:
			{
				string tmpName = imp.GetPostBodyRepresentation();
				body.AppendFile( "album", filePath, tmpName );
				break;
			}

			default:
				assert(false);
				return false;
		}
	}

	return true;
}


bool SimsUploader::IsMakinMagicFamily( const string &filePath )
{
	if ( filePath.empty() || MAX_PATH < filePath.size())
		{ assert(false); return false; }

	DWORD dot = filePath.rfind( "." );
	if ( dot == string::npos || dot == 0 ) { assert(false); return false; }

	string ext = ::ToLower( filePath.substr( dot + 1 ));

	return ext == "mtf";
}


// PklGetter (download)


string PklGetter::GetPkl( const Server &server, const HttpUri &uri )
{
	LOG( "PklGetter::GetPkl" );

	try
	{
		if ( !server.IsSet()) throw "Server::IsSet() failed";

		if ( !CreateRequest( &server, HRR_GET, false ))
			throw "CreateRequest(HRR_GET) failed";

		_request->SetUrl( uri.GetString());

		DWORD count = _request->SetMinimalHeaders();
		assert( 0 < count );

		bool cookie = IncludeCookie(); assert( cookie );

		if ( !IsTransportReady())
			throw "IsTransportReady() failed";

		short outcome = HRR_INVALID_STATE;
		outcome = SendReceive( HttpReqResp::HRR_SINGLE_BLOCKING );
		if ( outcome != HRR_OK )
			throw "SendReceive() failed";

		string codeAndPhrase = _response.GetCodeAndPhrase();
		if ( codeAndPhrase.find( "404" ) == 0 )
		{
			LOG( "PklGetter::GetPkl: 404" );
			throw Exception( "asset not found",
					"The family you requested is not available" );
		}

		// Detect errors sent from server, like
		// "Invalid filename. 		/family_exchange/n/n11n/Fab_4/Fab_4.pkl".
		// The only valid response begins with an unsigned long.
		string response = _response.GetBody();
		if ( response.empty())
			throw "response body is empty";

		{
			const WORD bufSize = 16;
			char buf[bufSize] = "";
			DWORD minLen = response.length() < bufSize ? response.length() : bufSize;

			for ( WORD i = 0; i < minLen; ++i )
			{
				char c = response[i];
				if ( isdigit( c ))
					buf[i] = c;
				else if ( c == '\n' )
				{
					buf[i] = '\0';
					break;
				}
				else
				{
					string exc;
					if ( response.size() < 128 )
						exc = "invalid response body:\n" + response;
					else
						exc = "invalid response body begins with: "
									+ response.substr( 0, minLen );
					throw exc;
				}
			}
			assert( buf[i] == '\0' );

			if ( !IsUnsignedLong( buf ))
			{
				char err[128] = "";
				sprintf( err, "invalid response body begins with: %s", buf );
				throw err;
			}
		}
	}
	catch ( const Exception &e )
	{
		e.TellUser( _title );
		return "";
	}
	catch ( string &s )
	{
		LOG( string("PklGetter::GetPkl: error: ") + s );
		assert(false);
		return "";
	}
	catch ( char *st )
	{
		LOG( string("PklGetter::GetPkl: error: ") + st );
		assert(false);
		return "";
	}
	catch (...)
	{
		LOG( "PklGetter::GetPkl: unknown exception" );
		assert(false);
		return "";
	}

	LOG( "PklGetter::GetPkl sets _gotPkl to true" );
	_gotPkl = true;
	
	return _response.GetBody();
}



// SimsDownloader

SimsDownloader::SimsDownloader( const string &title )
	: _title( title )
	, _server( NULL )
	, _getPkl( title )
	, _totalBytes( 0 ), _bytesReceived( 0 ), _totalBytesUnencoded( 0 )
	, _filesToDownloadCount( 0 ), _downloadedFilesCount( 0 )
	, _currentDownloader( NULL )
	, _sendDone( false ), _receiveDone( false )
	, _errorOut( false ), _canceled( false )
	, _downloadThread( NULL_THREAD ), _threadId( 0 )
{}


bool SimsDownloader::Initialize( const Server *server )
{
	if ( server == NULL || !server->IsSet()) { assert(false); return false; }
	_server = server;
	return true;
}


bool SimsDownloader::StartDownload( const string &scriptPath, const string &assetId,
					 const string &simsPath )
{
	LOG( string("SimsDownloader::StartDownload(") + scriptPath + ","
		+ assetId + "," + simsPath + ")");

	if ( scriptPath.empty() || assetId.empty()
			|| scriptPath.empty() || MAX_PATH <= scriptPath.length())
		{ assert(false); return false; }

	// get ready, probably totally unnecessary
	{
		assert( _bytesReceived == 0 );
		assert( !_sendDone && !_receiveDone );
		assert( _filesToDownloadCount == 0 && _downloadedFilesCount == 0 );
		assert( _currentDownloader == NULL );

		_bytesReceived = 0;
		_sendDone = _receiveDone = false;
		_filesToDownloadCount = _downloadedFilesCount = 0;
		_currentDownloader = NULL;
	}

	try
	{
		if ( !HttpReqResp::IsConnectedToInternet())
		{
			assert( InternetOpen( _title.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, 
									NULL, 0, INTERNET_INVALID_PORT_NUMBER ) == NULL );
			throw Exception(
				"SimsDownloader::StartDownload: IsConnectedToInternet failed",
				"You are not connected to the Internet." );
		}

		// data is deallocated in DownloadThread()
		DownloadData *data = new DownloadData( this, &scriptPath, &assetId,
														&simsPath );

		_downloadThread = (void *) _beginthreadex( NULL, 0,
								 DownloadThread, data, 0, &_threadId );

		// 	_beginthread returns -1 on an error
		if ( _downloadThread == NULL_THREAD )
			throw Exception(
				"SimsDownloader::StartDownload: _beginthreadex failed",
				"A threading error occurred in your operating system." );
	}
	catch ( Exception &e )
	{
		e.TellUser( _title );
		return false;
	}
	catch (...)
	{
		LOG( "SimsDownloader::StartDownload: unknown exception" );
		assert(false);
		return false;
	}

	return true;
}


unsigned int WINAPI SimsDownloader::DownloadThread( LPVOID param )
{
	LOG( "SimsDownloader::DownloadThread" );

	if ( param == NULL ) { assert(false); return HRR_INVALID_PARAMETER; }

	// First: copy param into local memory then deallocate param
	DownloadData *data = (DownloadData *) param;

	SimsDownloader *downloader = data->_downloader;
	if ( downloader == NULL ) { assert(false); return HRR_INVALID_PARAMETER; }
	string scriptPath = *data->_scriptPath;
	string assetId = *data->_assetId;
	string simsPath = *data->_simsPath;

	delete param;
	param = data = NULL;

	DWORD result = downloader->DownloadFamily( scriptPath, assetId, simsPath );

	LOG( "End SimsDownloader::DownloadThread, thread about to _endthreadex" );

	_endthreadex(0);

	assert(false); // I assume we don't get here
	return result;
}


bool SimsDownloader::DownloadFamily( const string &scriptPath, const string &assetId,
										const string &simsPath )
{
	LOG( "SimsDownloader::DownloadFamily begin" );

	if ( scriptPath.empty() || assetId.empty() || _server == NULL || !_server->IsSet()
			|| simsPath.empty() || MAX_PATH <= simsPath.length())
		{ assert(false); return false; }

	// build uri
	HttpUri uri;
	uri.SetUrl( scriptPath );
	uri.AddModifier( string( "asset_id=" ) + assetId );
	uri.AddModifier( "asset_type=FAM" );
	string loginKey =
		MaxisTeleporter::GetLoginKeyFromCookie( _server->GetCookie());
	if ( loginKey.empty())
	{
		Exception e(
			"No login key found in SimsDownloader::DownloadFamily",
			"You must be logged in as a product-registered user "
				"to use the exchange." );
		e.TellUser( _title );
		return false;
	}
	uri.AddModifier( loginKey );

	try
	{
		string pklContents = _getPkl.GetPkl( *_server, uri );
		if ( pklContents.empty())
		{
			throw Exception(
				"SimsDownloader::DownloadFamily: Initialize failed",
				"The teleporter received invalid data. (1)" );
		}
		else
		{
			string msg = "SimsDownloader::DownloadFamily: PKL contents:\n";
			msg += pklContents;
			LOG( msg );
			// MessageBox( NULL, pklContents.c_str(), "PKL", MB_OK );
		}

		// get packing list
		PackingListParser pkl;
		bool parse = pkl.Initialize( pklContents );
		if ( !parse )
			throw Exception(
				"SimsDownloader::DownloadFamily: Initialize failed",
				"The teleporter received invalid data. (2)" );

		_totalBytesUnencoded = pkl.GetFileSizesSum();
		if ( _totalBytesUnencoded == 0 )
			throw Exception(
				"SimsDownloader::DownloadFamily: GetFileSizesSum returned 0",
				"The teleporter received invalid data. (3)" );
		LOG( "SimsDownloader::DownloadFamily: GetFileSizesSum returned "
				+ NumberToString( _totalBytesUnencoded ));

		// Compute _totalBytes, the total bytes we will receive.
		//
		// _totalBytesUnencoded is the non-base64 encoded sum of bytes
		// the server intends to send us.  However, the server actually
		// sends us files that are base64 encoded.  So, we use the
		// non-base64 encoded sum to compute the base64 encoded sum.
		//
		// The computation:
		// base64 encoding (RFC1341) takes 3 bytes and encodes them as 4.
		//
		// Notes
		// * This computation is an estimate.  I need to understand how much
		// of a difference between _totalBytes and _totalBytesUnencoded is
		// acceptable.  (see WasSuccessful())
		// * I could have asked Noel to send the base64 encoded sum
		// in the PKL file, but it didn't seem necessary.

		_totalBytes = _totalBytesUnencoded * ( 4 / 3.0 );

		const vector<string> filePaths = pkl.GetFilePaths();
		if ( filePaths.size() == 0 )
			throw Exception(
				"SimsDownloader::DownloadFamily: GetFilePaths returned a vector of size 0",
				"The teleporter received invalid data. (4)" );

		_filesToDownloadCount = filePaths.size();
		LOG( "SimsDownloader::DownloadFamily: _filesToDownload set to "
				+ NumberToString( _filesToDownloadCount ));

		// download files
		for ( DWORD i = 0; i < filePaths.size(); ++i )
		{
			bool wasNotCanceled = DownloadFile( filePaths[i], uri, simsPath );
			// The only chance the user has to cancel is on filePath[0].
			// For all other files, cancel means the file was already on
			// the user's disk.
			if ( i == 0 && !wasNotCanceled )
			{
				_canceled = true;
				LOG( "SimsDownloader::DownloadFamily: user canceled download, "
						"_canceled set to true" );
				break;
			}
		}
	}
	catch ( Exception &e )
	{
		_errorOut = true;
		LOG( "SimsDownloader::DownloadFamily: _errorOut set to true" );

		e.TellUser( _title );
		return false;
	}
	catch ( LPCSTR s )
	{
		_errorOut = true;
		LOG( string("SimsDownloader::DownloadFamily: error: ") + s
				+ " and _errorOut set to true" );
		assert(false);
		return false;
	}
	catch (...)
	{
		_errorOut = true;
		LOG( "SimsDownloader::DownloadFamily: unknown exception "
				"and _errorOut set to true" );
		assert(false);
		return false;
	}

	bool ret = WasSuccessful();
	LOG( "SimsDownloader::DownloadFamily return " + string(ret ? "true" : "false"));
	return ret;
}


// This method throws all errors, so a return value of false means either:
// 1. the user canceled the overwrite dialog box or 2. the downloaded file is
// already in one of the user's UserDatas.  In either case, false means the
// file was not saved to the user's disk.  A return value of true indicates
// a save occurred.
bool SimsDownloader::DownloadFile( const string &relativeFilePath,
							const HttpUri &uriIn, const string &simsPath )
{
	LOG( "SimsDownloader::DownloadFile(" + relativeFilePath + ","
		+ uriIn.GetString() + "," + simsPath + ")");

	if ( _currentDownloader != NULL )
		throw Exception( "_currentDownloader is not NULL",
				"The teleporter is in a bad state" );

	if ( relativeFilePath.empty() || MAX_PATH <= relativeFilePath.length()
			|| uriIn.GetString().empty() || simsPath.empty()
			|| MAX_PATH <= simsPath.length()
			|| _currentDownloader != NULL )
		throw Exception( "SimsDownloader::DownloadFile: bad input",
				"The teleporter received invalid data. (10)" );

	// set encodedFileName
	DWORD fileNameStarts = relativeFilePath.rfind( "\\" );
	if ( fileNameStarts == string::npos || relativeFilePath.length() <= fileNameStarts )
		throw Exception( "SimsDownloader::DownloadFile: bad input",
				"The teleporter received invalid data. (11)" );

	const string relativePath = relativeFilePath.substr( 0, fileNameStarts );
	// jump past "\\"
	++fileNameStarts;
	const string unencodedFileName = relativeFilePath.substr( fileNameStarts );
	const string encodedFileName = HttpUri::UrlEncode( unencodedFileName );

	MaxisTeleporter tele( _title, "not used", 0, relativePath );
	bool init = tele.Initialize( _server );
	if ( !init )
		throw Exception( "SimsDownloader::DownloadFile: MaxisTeleporter::Initialize "
				"returned false", "An unexpected error occurred. (10)" );

	HttpUri uri( uriIn );
	uri.AddModifier( "file=" + encodedFileName );

	_currentDownloader = &tele;
	LOG( "SimsDownloader::DownloadFile: set _currentDownloader" );
	LOG( "SimsDownloader::DownloadFile: _downloadedFilesCount is "
			+ NumberToString( _downloadedFilesCount ));

	bool down = tele.StartDownload( uri.GetString(), HttpReqResp::HRR_SINGLE_BLOCKING,
										HttpReqResp::HRR_WININET, true );
	if ( !down )
		throw Exception( "SimsDownloader::DownloadFile: MaxisTeleporter::StartDownload "
				"returned false", "An unexpected error occurred. (11)" );

	// Finish transport: upload sets response code, download saves body as file.
	// If they are downloading to UserDataX, then alert them before they write
	// over anything.  If they are downloading somewhere else, do not allow
	// them to replace the current files.
	string outcome;
	if ( relativeFilePath.find( "UserData" ) != string::npos )
	{
		SfpConflictAlert alertOverWrite;
		alertOverWrite.SetTitle( _title );
		outcome = tele.FinishTransport( simsPath, alertOverWrite );
	}
	else
	{
		SfpDoNotOverWrite doNotOverWrite;
		outcome = tele.FinishTransport( simsPath, doNotOverWrite );
		assert( outcome != MaxisTeleporter::_canceledDownloadCode || tele.WasCanceled());
		// An outcome of _canceledDownloadCode here doesn't mean that the user canceled
		// the download, it means the file is already on the user's disk.
		// If outcome is _canceledDownloadCode don't set _canceled to true, because
		// that will stop the rest of the download.
	}
	
	LOG( "SimsDownloader::DownloadFile: FinishTransport returned " + outcome );
	assert( tele.WasSuccessful() || tele.WasCanceled());

	_bytesReceived += _currentDownloader->GetBytesReceived();
	LOG( "SimsDownloader::DownloadFile: _bytesReceived set to "
			+ NumberToString( _bytesReceived ));
	
	_currentDownloader = NULL;
	LOG( "SimsDownloader::DownloadFile: _currentDownloader set to NULL" );

	if ( !_errorOut )
	{
		++_downloadedFilesCount;
		LOG( "SimsDownloader::DownloadFile: _downloadedFilesCount incremented to "
				+ NumberToString( _downloadedFilesCount ));
	}

	return !_errorOut && outcome == MaxisTeleporter::_successfulDownloadCode;
}


string SimsDownloader::GetPercentComplete() const
{	
	if ( _errorOut )
	{
		LOG( "SimsDownloader::GetPercentComplete: _errorOut is true" );
		return "100";
	}
	if ( _canceled )
	{
		LOG( "SimsDownloader::GetPercentComplete: _canceled is true" );
		return "100";
	}

	if ( _totalBytes == 0 )
		return "0";

	DWORD bytesReceived = _bytesReceived;

	if ( _currentDownloader )
		bytesReceived += _currentDownloader->GetBytesReceived();

	DWORD percent = ( 100 * bytesReceived ) / _totalBytes;
	if ( 100 < percent )
	{
		assert(false);
		return "100";
	}

	char buf[4] = "";
	sprintf( buf, "%d", percent );

	return buf;
}


string SimsDownloader::FinishTransport()
{
	LOG( "SimsDownloader::FinishTransport" );

	// 1 minute time limit on waiting, to prevent this thread from hanging
	// when transport thread is in bad state
	DWORD beginTime = GetTickCount();

	while (( !_getPkl.GotPkl() || _downloadedFilesCount < _filesToDownloadCount )
				&& !_errorOut && !_canceled )
	{
		if ( 60000 < GetTickCount() - beginTime )
		{
			LOG( "SimsDownloader::FinishTransport: 1 min timeout elapsed!" );
			assert(false);
			break;
		}

		bool pump = HttpReqResp::PumpMessages( 100 );
		if ( !pump )
			break;
	}

	if ( WasSuccessful())
	{
		LOG( "SimsDownloader::FinishTransport return 108" );
		assert( !WasCanceled() && !_errorOut );
		return MaxisTeleporter::_successfulDownloadCode;
	}

	if ( WasCanceled())
	{
		LOG( "SimsDownloader::FinishTransport return 109" );
		assert( !WasSuccessful() && !_errorOut );
		return MaxisTeleporter::_canceledDownloadCode;
	}

	LOG( "SimsDownloader::FinishTransport return -1" );
	assert( _errorOut );
	return "-1";
}


bool SimsDownloader::WasSuccessful() const
{
	if ( _errorOut ) return false;
	if ( _canceled ) return false;

	// if you don't know why I didn't say "_totalBytes == _bytesReceived",
	// see my comment in DownloadFamily().
	assert( abs( _totalBytes - _bytesReceived ) <= 12 );

	return _getPkl.GotPkl() && 0 < _filesToDownloadCount
		&& _downloadedFilesCount == _filesToDownloadCount;
}


bool SimsDownloader::WasCanceled() const
{
	return _canceled;
}


void SimsDownloader::Reset()
{
	LOG( "SimsDownloader::Reset" );

	// leave _server alone

	_getPkl.Reset();
	_totalBytes = _bytesReceived = _totalBytesUnencoded = 0;
	_filesToDownloadCount = _downloadedFilesCount = 0;
	_currentDownloader = NULL;
	_sendDone = _receiveDone = false;
	_errorOut = _canceled = false;
	_downloadThread = NULL_THREAD;
	_threadId = 0;
}


// SimsTeleporter

SimsTeleporter::SimsTeleporter()
	: _uploader( s_title, s_boundary, s_maxUploadSize )
	, _downloader( s_title )
	, _direction((SimsTeleporter::Direction) 0 )
{}


bool SimsTeleporter::Initialize( const string &serverName )
{
	bool outcome = true;

	if ( !_server.SetNameAddress( serverName ))
		return false;

	if ( !_uploader.Initialize( &_server ))
		outcome = false;

	if ( !_downloader.Initialize( &_server ))
		outcome = false;

	return outcome;
}


bool SimsTeleporter::StartDownload( const string &scriptPath, const string &assetId,
						const string &simsInstallPath )
{
	_direction = Down;
	if ( !_downloader.StartDownload( scriptPath, assetId, simsInstallPath ))
	{
		_direction = (Direction) 0;
		return false;
	}
	return true;
}


bool SimsTeleporter::StartUpload( const string &scriptPath, const string &filePath,
									const string &subject )
{
	// check for legal family name
	try
	{
		DWORD i = 0;
		if ( !IsLegalFamilyName( subject, &i ))
		{
			string errorMessage;

			if ( subject.empty())
			{
				errorMessage = "The family you want to upload has no name.";
			}
			else
			{
				errorMessage = "The name of the family you want to upload, \"";
				errorMessage += subject + "\", is not legal.";

				char bad[4] = "";
				sprintf( bad, "%c", subject[i] );

				errorMessage += "\n";
				errorMessage += "The illegal character is \'";
				errorMessage += bad;
				errorMessage += "\' at character position number ";
				errorMessage += NumberToString( i + 1 ); // start counting at 1
				errorMessage += ".";
			}

			errorMessage += "\n\n";
			errorMessage += "Please rename your family and try uploading again.";

			throw Exception( "IsLegalFamilyName(" + subject + ") returned false",
								errorMessage );
		}
	}
	catch ( Exception &e )
	{
		e.TellUser( s_title );
		return false;
	}
	catch (...)
	{
		string errorMessage =
			"The name of the family you want to upload is not legal.";
		Exception e( "SimsTeleporter::StartUpload: unknown exception", errorMessage );
		e.TellUser( s_title );
		return false;
	}

	if ( IsDefaultFamilyName( subject ))
	{
		string errorMessage = "You cannot teleport a family that was included with "
			"the game,\nunless you have first saved the game. (2)";

		Exception e( "IsDefaultFamilyName(" + subject + ") returned true",
						errorMessage );
		e.TellUser( s_title );
		return false;
	}

	_direction = Up;
	if ( !_uploader.StartUpload( scriptPath, filePath, subject ))
	{
		_direction = (Direction) 0;
		return false;
	}
	return true;
}


string SimsTeleporter::GetPercentComplete() const
{
	string percent;
	switch ( _direction )
	{
		case Up:
			percent = _uploader.GetPercentComplete();
			break;

		case Down:
			percent = _downloader.GetPercentComplete();
			break;
	}
	LOG( string( "SimsTeleporter::GetPercentComplete returns " ) + percent ); 

	assert( !percent.empty());
	return percent;
}


string SimsTeleporter::FinishTransport( const string &simsPath )
{
	switch ( _direction )
	{
		case Up:
			return _uploader.FinishUpload();

		case Down:
			return _downloader.FinishTransport();
	}
	assert(false);
	return "";
}


void SimsTeleporter::Reset()
{
	_downloader.Reset();
	_uploader.Reset();
	_direction = (SimsTeleporter::Direction) 0;
}


bool SimsTeleporter::IsLegalFamilyName( const string &familyName,
				DWORD *indexOfIllegalChar /*=NULL*/)
{
	if ( familyName.empty())
	{
		if ( indexOfIllegalChar != NULL )
			*indexOfIllegalChar = 0;
		assert(false);
		return false;
	}
	if ( MAX_PATH <= familyName.size())
	{
		if ( indexOfIllegalChar != NULL )
			*indexOfIllegalChar = MAX_PATH;
		assert(false);
		return false;
	}

	// these characters are illegal anywhere in a family name
	static const string illegalChars[] = 
	{
		"\\",
		"/",
		"*",
		".",
		"\"",
		"`",
		"~",
		" "
	};
	static const DWORD illegalCharsCount
						= sizeof( illegalChars ) / sizeof( string );

	for ( int c = 0; c < illegalCharsCount; ++ c)
	{
		DWORD i = familyName.find( illegalChars[ c ]);
		if ( i != string::npos )
		{
			if ( indexOfIllegalChar != NULL )
				*indexOfIllegalChar = i;
			assert(false);
			return false;
		}
	}

	// '-' cannot be the first character
	if ( familyName[0] == '-' )
	{
		if ( indexOfIllegalChar != NULL )
			*indexOfIllegalChar = 0;
		assert(false);
		return false;
	}

	return true;
}


/*
Found in <sims install path>\UserData\Web Pages\publish.html:

function defaultFam()
{
	subject = parent.nav.document.PubInfo.LotName.value;
    if (subject == "Goth_5" ||
        subject == "Newbie_1" ||
        subject == "Bachelor_2" ||
        subject == "Pleasant_4" ||
        subject == "Roomies_3")
    {
	    alert("WARNING: You cannot Teleport a family that was included with the game, unless you have first saved the game.");
	}
}

The reason I am including this check in the ActiveX control is because I'm not
convinced that defaultFam() gets called.
*/
bool SimsTeleporter::IsDefaultFamilyName( const string &familyName )
{
/*	I commented this out because it was preventing the testers from
	uploading an altered Bachelor_2, which should be allowed.  For the
	sake of not stalling the testers, Mark T decided to cut this test out,
	and allow users to upload the default families, even if they have not
	been changed.

    if ( familyName == "Goth_5"
			|| familyName == "Newbie_1"
			|| familyName == "Bachelor_2"
			|| familyName == "Pleasant_4"
			|| familyName == "Roomies_3" )
		return true;
*/
	return false;
}



string SimsTeleporter::Test( bool testNetwork, string serverName, HINSTANCE inst )
{
	if ( !testNetwork )
		return "Test SimsTeleporter requires network connection\n";

	Logger::Start( "SOFTWARE\\Maxis\\The Sims", "SimsTeleporterTest.txt" );

	string log = "Test SimsTeleporter\n";

	for ( int i = 0; i < 10; ++i )
	{
		{
			char szCount[16] = "";
			sprintf( szCount, "count: %d", i );
			LOG( szCount );
		}

		log += TestDownload( serverName, inst );
		log += TestUpload( serverName );
	}

//	TestErrors( serverName );
	
	return log;
}


string SimsTeleporter::TestUpload( string serverName )
{
	string log;

	Server server;
	if ( !server.SetNameAddress( serverName ))
		log += "Server::SetNameAddress() failed\n";

	serverName = "nmolon.thesims.max.ad.ea.com";

	SimsConfig simsConfig;
	simsConfig.Initialize();
	string simsPath = simsConfig.GetSimsPath();

	string subject = "Roomies_3"; // Testy_6 data in "Test Data" directory
	string fam = simsPath + "\\UserData\\Export\\" + "Roomies_3.fam";
	//string fam = simsPath + "\\UserData\\Export\\" + "Newbie_6.FAM";
	assert( FileExists( fam ));

	SimsUploader t( s_title, s_boundary, s_maxUploadSize );

	if ( !t.Initialize( &server ))
		log += "Initialize() failed\n";

	string scriptPath = "/teleport/families/up/ul_manager.php";
	bool upload = t.StartUpload( scriptPath, fam, subject );
	if ( !upload )
		log += "StartUpload() failed\n";

	string percent = t.GetPercentComplete();
	while ( 0 < percent.length() && percent.length() < 3
				&& percent.at(0) < '7' )
	{
		Sleep( 500 );
		percent = t.GetPercentComplete();
	}
	if ( percent.length() == 0 )
		log += "GetPercentComplete() failed\n";

	string outcomeCode = t.FinishUpload();
	if ( outcomeCode.empty())
		log += "FinishTransport() failed\n";
	string uploadPhrase = t.GetOutcomePhrase();
	if ( uploadPhrase.empty())
		log += "GetOutcomePhrase() failed\n";

	if ( !t.WasSuccessful())
		log += "WasSuccessful() returned false\n";

	if ( t.GetAccount().empty() || t.GetCode().empty())
		log += "GetAccount() or GetCode() failed\n";

	return log;
}


string SimsTeleporter::TestDownload( string serverName, HINSTANCE inst )
{
	string log;

	// download a family

	SimsConfig simsConfig;
	simsConfig.Initialize();
	const string simsPath = simsConfig.GetSimsPath();

	Server server;
	if ( !server.SetNameAddress( serverName ))
		log += "Server::SetNameAddress() failed\n";

	SimsDownloader t( s_title );
		
	if ( !t.Initialize( &server ))
		log += "Initialize() failed\n";

	string scriptPath = "/teleport/families/down/dl_manager.php";
	string assetId = "205";
	bool download = t.StartDownload( scriptPath, assetId , simsPath );
	if ( !download )
		log += "StartDownload() failed\n";

	string percent = t.GetPercentComplete();
	while ( percent.length() == 1 || percent.at(0) < '7' ) 
	{
		Sleep( 50 );
		percent = t.GetPercentComplete();
	}

	string outcome = t.FinishTransport();
	if ( outcome != MaxisTeleporter::_successfulDownloadCode
						&& outcome != MaxisTeleporter::_canceledDownloadCode )
		log += "unknown error 1\n";

	if ( !t.WasSuccessful() && !t.WasCanceled())
		log += "unknown error 2\n";

	// It might be nice to have a SimsDownloader method that returns
	// the downloaded PackingList.  That way I could check the download.

	return log;
}


void SimsTeleporter::TestErrors( string serverName )
{
	// error conditions that Mark mentioned
	{
		SimsTeleporter t;
		t.StartUpload( "scriptPath", "filePath", "" );
		t.StartUpload( "scriptPath", "filePath", "-" );
		t.StartUpload( "scriptPath", "filePath", "bad\\name" );
		t.StartUpload( "scriptPath", "filePath", "bad/name" );
		t.StartUpload( "scriptPath", "filePath", "bad*name" );
		t.StartUpload( "scriptPath", "filePath", "bad.name" );
		t.StartUpload( "scriptPath", "filePath", "bad\"name" );
		t.StartUpload( "scriptPath", "filePath", "bad`name" );
		t.StartUpload( "scriptPath", "filePath", "bad~name" );
		t.StartUpload( "scriptPath", "filePath", "bad name" );
		t.StartUpload( "scriptPath", "filePath", "Pleasant_4" );
	}

	// try to download an asset_id that isn't in the database
	{
		Server server;
		server.SetNameAddress( serverName );	
			
		string assetId = "1";
		string scriptPath = "/teleport/families/down/dl_manager.php";
		string simsPath = "doesn't matter";

		SimsDownloader t( s_title );

		t.Initialize( &server );
		t.StartDownload( scriptPath, assetId, simsPath );
		t.FinishTransport();
	}
}


