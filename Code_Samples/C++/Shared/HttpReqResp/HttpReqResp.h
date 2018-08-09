// HttpReqResp.h: interface for the HttpReqResp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPREQRESP_H__9235E7B5_A9A6_4A3F_A799_18B1B9A71C62__INCLUDED_)
#define AFX_HTTPREQRESP_H__9235E7B5_A9A6_4A3F_A799_18B1B9A71C62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <windows.h>
#include <wininet.h>
#include <winsock.h>
#include <assert.h>

// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>

using namespace std;

#include "GetRequest.h"
#include "PostRequest.h"
#include "HeadRequest.h"
#include "Server.h"
#include "Response.h"
#include "Logger.h"


class HttpReqResp  
{
public:
	HttpReqResp( string title, string boundary = string());
	virtual ~HttpReqResp();

	Request *_request;
	const string _boundary;

	Response _response;

	const Server *_server;
	const DWORD _mtu;

	enum RequestType { HRR_GET = 1, HRR_POST, HRR_HEAD }; // HRR_PUT
	bool CreateRequest( const Server *server, RequestType requestType,
							bool useOldBody = false );

	bool IsSet() const;
	static bool IsConnectedToInternet();
	bool IsTransportReady() const;

	bool IncludeCookie();

	enum Threading
	{
		HRR_SINGLE_BLOCKING = 1,		// no threads created
		HRR_DUAL,						// 1 thread created and destroyed
		HRR_SINGLE_MESSAGES				// no threads created, Win32 asynchronous
										// (not implemented)
	};
	enum Transport { HRR_WININET = 1, HRR_WINSOCK };
	int SendReceive( Threading threading, Transport transport = HRR_WININET );

	// use these methods only after SendReceive() with HRR_DUAL
	bool Suspend() const;
	bool Resume() const;

	short GetPercentSend() const;
	short GetPercentReceive() const;
	bool IsSendDone() const;
	bool IsHeadReceived() const;
	bool IsReceiveDone() const;
	DWORD GetBytesSent() const;
	DWORD GetBytesReceived() const;
	
	enum { HRR_TIMEOUT, HRR_END, HRR_QUIT };
	short WaitForTransportEnd( DWORD millisecondsToWait );

	int GetThreadStatus();

	bool CloseThread( bool terminate = false );
	// end of HRR_DUAL methods

	static const double s_waitTimeScaler; // in milliseconds per byte
	static double WaitTime( DWORD bytes );

	static bool PumpMessages( DWORD millisecondsToPump );

	bool WasTransportSuccessful() const;

	// I don't know how to test this
	DWORD SetCookies() const;

	const string _title; // appears in dialog boxes

	void Reset( bool requestInProgress = false );

	void SetTransactionId( const string &transactionId );
	string GetTransactionId() { return _transactionId; }

protected:
	unsigned int TransportWork( Transport trans );

	// HRR_DUAL
	static unsigned int WINAPI TransportThread( LPVOID param );

	HANDLE _transportThread;
	unsigned int _threadId; // for logging
	// end HRR_DUAL

	int SendReceiveWininet();
	bool SendWininet( HINTERNET hRequest );
	bool ReceiveWininet( HINTERNET hRequest );
	void ReceiveHead( HINTERNET hRequest );
	bool ReceiveBodySize( HINTERNET hRequest, DWORD *bodySize );

	int SendReceiveWinsock();
	void SendWinsock( SOCKET s );
	void ReceiveWinsock( SOCKET s );

	DWORD _bytesSent;
	DWORD _bytesReceived;

	bool _sendDone;
	bool _headReceived;
	bool _receiveDone;

	short CalculatePercentSend() const;
	short CalculatePercentReceive() const;

	// exception and error state
	string _technicalPhrase;
	string _lastErrorAndPhrase;
	short _hrrError;
	// I wanted to keep transaction ids out of HttpReqResp, but because
	// the transport thread must handle its own exceptions, including the
	// error dialog box, and the error dialog box should contain the
	// transaction id, we must store the transaction id.
	string _transactionId;

public:
	static string Test( bool testNetwork, bool testUi,
							const string &serverName, const string &serverAddress );
	static string TestErrors( const string &serverName );
	static string TestUploadLot( const string &serverName, Threading thr );
	static string TestDownloadLot( const string &serverName, Threading thr );
	static string TestHead( const string &serverName, Threading thr );
	static string TestWinsock( const string &serverName, Threading thr );
	static string TestUnencodedDownload( const string &serverName );
	// old tests
	static string TestUpload( bool testUi,
							const string &serverName, const string &serverAddress );
	static string TestDownload( bool testUi,
							const string &serverName, const string &serverAddress );
};


extern HANDLE NULL_THREAD;


// It doesn't look like MaxisMike ever got Cancel() to work.
// To kill a process, DI first sent PostMessage(hWnd, WM_CLOSE, 0, 0),
// and if that didn't work, TerminateProcess().
// I should first try to tell the process to exit (MaxisMike's poison pill),
// then use TerminateThread().
// bool Cancel();


#endif // !defined(AFX_HTTPREQRESP_H__9235E7B5_A9A6_4A3F_A799_18B1B9A71C62__INCLUDED_)

