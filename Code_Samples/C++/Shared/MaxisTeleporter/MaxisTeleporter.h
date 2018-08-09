// MaxisTeleporter.h: interface for the MaxisTeleporter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAXISTELEPORTER_H__50F056C2_E1ED_4933_BC48_8583687B8500__INCLUDED_)
#define AFX_MAXISTELEPORTER_H__50F056C2_E1ED_4933_BC48_8583687B8500__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <windows.h>
#include <assert.h>

// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>
#include <list>

using namespace std;


#include "HttpReqResp.h"
#include "MaxisTeleporter1.h"


// MaxisTeleporter HAS A HttpReqResp member, instead of MaxisTeleporter IS A
// HttpReqResp subclass, because MaxisTeleporter may ultimately contain a list
// of HttpReqResp objects instead of just one.


class MaxisTeleporter : public MaxisTeleporter1
{
protected:
	HttpReqResp _reqResp;
	const Server *_server;

	enum Direction { MT_NULL, MT_UPLOAD, MT_DOWNLOAD } _direction;

public:
	MaxisTeleporter( const string &title, const string &boundary,
						DWORD maxUploadSize, const string &relativeImportPath );

	// begin MaxisTeleporter1
	bool Initialize( const Server *server );

	enum BodyFormat { MT_FORM_MIME = 0, MT_FORM_MIME_BASE64, MT_OLD_BODY };

	bool StartUpload( const string &url, const string &filePath,
			HttpReqResp::Threading thr,
				HttpReqResp::Transport transport = HttpReqResp::HRR_WININET,
					BodyFormat bodyFormat = MT_FORM_MIME );
	bool StartDownload( const string &url, HttpReqResp::Threading thr,
				HttpReqResp::Transport transport = HttpReqResp::HRR_WININET,
					bool base64DecodeResponseBody = false );

	string GetPercentComplete() const;

	string FinishTransport( const string &installPath, SaveFilePolicy &policy );
	string FinishUpload();

	string GetOutcomePhrase() const;

	string GetAccount() const;
	string GetCode() const;

	bool WasSuccessful() const;
	bool WasCanceled() const;
	// end MaxisTeleporter1

	const DWORD _maxUploadSize; // in bytes
	const string _relativeImportPath;

	virtual bool SetReqBodyWithAsset( const string &filePath ) { return false; }

	bool IsTransportReady() const;

	// include Cookie: in request
	bool IncludeCookie();
	// write the Set-Cookie:s to disk
	DWORD SetCookies() const;

	string GetLoginKey() const;
	static string GetLoginKeyFromCookie( const string &cookie );

	bool IsSendDone() const;
	bool IsReceiveDone() const;
	DWORD GetBytesSent() const;
	DWORD GetBytesReceived() const;

	short WaitForTransportEnd();

	enum SaveOutcome { MT_ERROR = 0, MT_SUCCESS, MT_CANCEL };
	SaveOutcome SaveBodyAsFile( const string &installPath, SaveFilePolicy &policy,
				bool decodeBase64 = true );

	void Reset(); // "virtual" caused problems in HdNpcTeleporterTest!

	static string GenerateTransactionId();
	bool SetTransactionId();
	string GetTransactionId() const;
	string GetTransactionIdWithName() const;

	static const char *_successfulUploadCode;
	static const char *_successfulDownloadCode;
	static const char *_canceledDownloadCode;

protected:
	bool CreateUploadRequest( bool useOldBody = false );
	bool CreateDownloadRequest();

	string GetCookie() const;

	bool _wasCanceled; // by user

	// The transaction id helps find a user's problem in the server
	// logs.  That is, suppose the user has an upload or download error,
	// and the user receives a transaction id with the error message.
	// Then we can search the server logs for that transaction id.
	//
	// A transaction id is 12 hex digits.  The first 4 hex digits are
	// randomly generated and serve to distinguish the user from all
	// other users who may be teleporting.  The last 8 hex digits are
	// a UNIX time stamp.  Remember that UNIX time runs out January 19, 2038.
	//
	// While we can't guarantee that no 2 users will generate the same
	// transaction id, the chances are acceptably small.
	string _transactionId;

	// _base64DecodeResponseBody is set by StartDownload() and used by
	// FinishTransport()
	bool _base64DecodeResponseBody;

public:
	bool SetAttribute( const string &name, const string &value );
	const list<NameValuePair> &GetAttributes() const { return _attributes; }

	static string Test( bool testNetwork, string serverName, HINSTANCE inst );
	static string TestDownloadLot( string serverName, HINSTANCE inst );
	static string TestUploadLot( string serverName );
	static string TestUploadLotTwice( string serverName );
};

#endif // !defined(AFX_MAXISTELEPORTER_H__50F056C2_E1ED_4933_BC48_8583687B8500__INCLUDED_)
