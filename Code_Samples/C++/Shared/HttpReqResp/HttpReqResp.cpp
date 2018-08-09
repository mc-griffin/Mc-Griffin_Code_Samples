// HttpReqResp.cpp: implementation of the HttpReqResp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HttpReqResp.h"
#include "Error.h"
#include "HttpUri.h"
#include "Utilities.h"
#include "FileUtils.h"
#include <process.h> // _beginthread()
#include <errno.h>


// pass data from SendRequest() to TransportThread()
struct TransportData
{
	const HttpReqResp::Transport _transport;
	HttpReqResp *const _reqResp;

	TransportData( HttpReqResp *reqResp, HttpReqResp::Transport transport )
		: _reqResp( reqResp ), _transport( transport )
	{}
};


// s_waitTimeScaler is a scaling factor used to compute how long a user
// should wait for a transport to end, given the size of that transport.
// s_waitTimeScaler is expressed in the unit milliseconds per byte (ms/b).
//
// s_waitTimeScaler was computed to approximate the stategy
// "wait 1 minute for half a megabyte", which is expressed
// ( 2^5 * 5^4 * 3 ) ms / 2^19 b, which is approximately 0.11444091796875.
// I rounded up to .12.
const double HttpReqResp::s_waitTimeScaler = 0.12;


HANDLE NULL_THREAD = HANDLE( -1 );


HttpReqResp::HttpReqResp( string title, string boundary /*=string()*/)
	: _title( title ), _boundary( boundary )
	, _request( NULL ), _server( NULL )
	, _transportThread( NULL_THREAD ), _threadId( 0 )
	, _bytesSent( 0 ), _bytesReceived( 0 )
	, _sendDone( false ), _headReceived( false ), _receiveDone( false )
	, _hrrError( 0 )
	, _mtu( 1460 )
{
	assert( _mtu == 1460 );
	assert( !_title.empty());
}


HttpReqResp::~HttpReqResp()
{
	LOG( "~HttpReqResp" );
	Reset();
}


// requestInProgress is a hack to compensate for the case where an
// HttpReqResp object is reused for a number of requests.  See
// HttpReqResp::CreateRequest()
void HttpReqResp::Reset( bool requestInProgress /*=false*/ )
{
	LOG( "HttpReqResp::Reset" );

	// leave _server alone

	_response.Reset();

	delete _request;
	_request = NULL;

	_bytesSent = _bytesReceived = 0;
	_sendDone = _headReceived = _receiveDone = false;

	if ( !requestInProgress && _transportThread != NULL_THREAD )
	{
		LOG( "HttpReqResp::Reset: _transportThread non-NULL_THREAD, CloseThread" );

		bool term = CloseThread( true );
		assert( term && _transportThread == NULL_THREAD && _threadId == 0 );
	}

	_hrrError = 0;
	_technicalPhrase.erase();
	_lastErrorAndPhrase.erase();

	if ( !requestInProgress )
		_transactionId.erase();
}


bool HttpReqResp::CreateRequest( const Server *server,
					HttpReqResp::RequestType requestType,
						bool useOldBody /*=false*/)
{
	if ( server == NULL || !server->IsSet())
		{ assert(false); return false; }

	// you can only use the old body on a POST
	if ( useOldBody && requestType != HRR_POST )
		{ assert(false); return false; }

	if ( IsSet())
	{
		LOG( "HttpReqResp::CreateRequest(): previous state overwritten by new request" );
		Reset( true );
	}

	switch ( requestType )
	{
		case HRR_GET:
			_request = new GetRequest( server );
			break;

		case HRR_POST:
			if ( _boundary.empty()) { assert(false); return false; }
			_request = new PostRequest( server, _boundary, useOldBody );
			break;

		case HRR_HEAD:
			_request = new HeadRequest( server );
			break;

		default:
			assert(false);
			return false;
			break;
	}

	_server = server;

	return IsSet();
}


bool HttpReqResp::IsSet() const
{
	return  _server != NULL && _server->IsSet()
				&& _request != NULL // && _request->IsSet() must set headers
					&& !_title.empty();
}


bool HttpReqResp::IsConnectedToInternet()
{
#ifdef _DEBUG
	DWORD moment = GetTickCount();
#endif

	DWORD state = INTERNET_CONNECTION_MODEM|INTERNET_CONNECTION_LAN|INTERNET_CONNECTION_PROXY;
	// MSDN's documentation seems to be wrong because it says that the first
	// parameter to InternetGetConnnectedState() is [out].
	// But "Internet Client SDK Part II" talks about passing flags in via
	// the first parameter.
	bool connected = InternetGetConnectedState( &state, 0 ) == TRUE;
	if ( !connected )
		LOG( "HttpReqResp::IsConnectedToInternet() return false" );

	// is InternetGetConnectedState() causing slowness?
	assert( GetTickCount() - moment < 100 );

/* This function will always return true, because I have seen too many
   BBS messages like this one:

Author:  CingaWolf 
Date:  Sep-04-02 07:29 AM PDT 
Subject:  Not Connected to the internet???? 
 
I am trying to help a friend download a few of your really cool families, but it says we are not connected to the internet.
She has windows XP and is on a ISP
We tried going throught AOL 7.0 and Microsoft explorer both got same answer.
Please help, advise.
Cinga
Visit http://thesims.ea.com/us/exchange/detail/album_index.php?family_id=69056&action=HOME 
This is Galen's album contains a story, and no nudity or bad language.
G rated Please rate it how you feel realizing this is his first attempt and all the frustration the poor kid went through. He will be posting a new adventure next week.
I may repost it for him can I do that, now that he has the album fixed (we hope)?
*/
//	return connected;
	return true;

/* More on the problems with InternetGetConnectedState()

from MSDN:
Making a Dialup Connection to the Internet
Brian Hart

"InternetGetConnectedState() detects whether the user uses a modem to connect to
the Internet or not. Unfortunately, my tests have shown that
InternetGetConnectedState()’s behavior is unpredictable; it has said I use a LAN
when in fact I dial in to a provider. It’s probably confused because I have both
a modem and a network adapter in my computer. Hopefully, Microsoft will refine it
in later versions. I wouldn’t recommend depending solely on what
InternetGetConnectedState() has to say, but it can help out, for example, in
knowing how to initialize an option.
*/
}


bool HttpReqResp::IsTransportReady() const
{
	if ( !IsSet() || !_request->IsSet())
		return false;
	return IsConnectedToInternet();
}


// throws
int HttpReqResp::SendReceive( HttpReqResp::Threading thr,
			HttpReqResp::Transport trans /*=HRR_WININET*/)
{
	if (( thr != HRR_SINGLE_BLOCKING && thr != HRR_DUAL )
			|| ( trans != HRR_WININET && trans != HRR_WINSOCK ))
	{
		assert(false);
		return HRR_INVALID_PARAMETER;
	}

	if ( !IsSet())
	{
		assert(false);
		return HRR_INVALID_STATE;
	}

	// log
	{
		string msg = "Start HttpReqResp::SendReceive(";
		msg += ( NumberToString( thr ) + "," + NumberToString( trans ) + ")" );
		LOG( msg );
	}

	assert( _bytesSent == 0 && _bytesReceived == 0 );
	_bytesSent = 0;
	_bytesReceived = 0;

	assert( !_sendDone && !_receiveDone );

	if ( !IsConnectedToInternet())
	{
		// Using INTERNET_INVALID_PORT_NUMBER is the same as supplying the
		// default port number.
		assert( InternetOpen( _title.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, 
								NULL, 0, INTERNET_INVALID_PORT_NUMBER ) == NULL );
		throw Exception(
			"Http:ReqResp::SendReceive(): IsConnectedToInternet() failed",
			"You are not connected to the Internet." );
	}

	if ( thr == HRR_DUAL )
	{
		// data is deallocated in TransportThread()
		TransportData *data = new TransportData( this, trans );

		/* MSDN: C Run-Time Library Functions for Thread Control
		! Warning   If you are going to call C run-time routines from a program built
		with LIBCMT.LIB, you must start your threads with the _beginthread function.
		Do not use the Win32 functions ExitThread and CreateThread.
		*/	
		_transportThread = (void *) _beginthreadex( NULL, 0,
								 TransportThread, data, 0, &_threadId );

		// 	_beginthread returns -1 on an error
		if ( _transportThread == NULL_THREAD )
		{
			delete [] data;
			LOG( "_beginthreadex failed in HttpReqResp::SendReceive, deallocate TransportData" );
			char err[256] = "";
			int n = sprintf( err, "HttpReqResp::SendReceive: errno is %d: %s",
				errno, strerror( errno ));
			if ( 0 < n && n < 255 )
				LOG( err );
			assert(false);
			return HRR_CREATE_THREAD_FAILURE;
		}
	}
	else if ( thr == HRR_SINGLE_BLOCKING )
	{
		assert( _transportThread == NULL_THREAD );

		DWORD result = TransportWork( trans );
	}

	LOG( "End HttpReqResp::SendReceive" );

	return HRR_OK;
}


// returns a signed number
// memory allocated in SendReceive (param) is deallocated here
unsigned int WINAPI HttpReqResp::TransportThread( LPVOID param )
{
	LOG( "Start HttpReqResp::TransportThread" );

	if ( param == NULL ) { assert(false); return HRR_INVALID_PARAMETER; }

	// First: copy param into local memory then deallocate param
	TransportData *dataIn = (TransportData *) param;

	HttpReqResp *reqResp = dataIn->_reqResp;
	Transport transport = dataIn->_transport;

	delete param;
	param = dataIn = NULL;

	if ( reqResp == NULL ) { assert(false); return HRR_INVALID_PARAMETER; }

	DWORD result = reqResp->TransportWork( transport );
	//	DWORD result = TransportWork( data ); when this method was static like TranportThread

	LOG( "End HttpReqResp::TransportThread, thread about to _endthreadex" );
	/* MSDN: _beginthread():
	_endthread automatically closes the thread handle (whereas _endthreadex does not). 
	Therefore, when using _beginthread and _endthread, do not explicitly close
	the thread handle by calling CloseHandle(). This behavior differs
	from ExitThread().
	A thread created with _beginthreadex is terminated by a call to _endthreadex.
	*/

	_endthreadex(0);

	assert(false); // I assume we don't get here
	return result;
}


unsigned int HttpReqResp::TransportWork( HttpReqResp::Transport trans )
{
	if ( trans != HRR_WININET && trans != HRR_WINSOCK )
	{
		assert(false);
		return HRR_INVALID_PARAMETER;
	}

	if ( !IsSet()) { assert(false); return HRR_INVALID_STATE; }

	int result = HRR_INVALID_STATE;
	// this try block ensures all exceptions for the thread are handled here
	try
	{
		if ( trans == HRR_WININET )
			result = SendReceiveWininet();
		else if ( trans == HRR_WINSOCK )
			result = SendReceiveWinsock();

		if ( IsHrrError( result ))
		{
			_hrrError = result;

			char szError[16] = "";
			sprintf( szError, "%d", result );
			string msg = "HttpReqResp::TransportThread: transport error is ";
			msg += szError;
			throw Exception( msg, "An internet transport error has occurred. (1)" );
		}
	}
	catch ( const Exception &e )
	{
		_technicalPhrase = e.GetTechnicalPhrase();
		if ( 0 < e.GetTheLastError())
			_lastErrorAndPhrase = e.GetLastErrorAndPhrase();

		// Don't throw e, because the transport thread must catch its own exceptions.
		e.TellUser( _title.c_str(), GetTransactionId());
	}
	catch (...)
	{
		_hrrError = result;

		char szError[16] = "";
		sprintf( szError, "%d", result );
		string msg = "HttpReqResp::TransportThread: transport error is ";
		msg += szError;
		// Don't throw e, because the transport thread must catch its own exceptions.
		Exception e( msg, "An internet transport error has occurred. (2)" );
		e.TellUser( _title.c_str(), GetTransactionId());
	}

	assert( _sendDone && _receiveDone );

	return result;
}


bool HttpReqResp::Suspend() const
{
	if ( _transportThread == NULL_THREAD ) { return false; }
	DWORD suspend = SuspendThread( _transportThread );
	assert( suspend != -1 );
	return suspend != -1;
}


bool HttpReqResp::Resume() const
{
	if ( _transportThread == NULL_THREAD ) { return false; }
	DWORD resume = ResumeThread( _transportThread );
	assert( resume != -1 );
	return resume != -1;
}


short HttpReqResp::WaitForTransportEnd( DWORD millisecondsToWait )
{
	// log
	{
		char millis[16] = "";
		sprintf( millis, "%d", millisecondsToWait );
		LOG( string( "HttpReqResp::WaitForTransportEnd(" ) + millis + ")" );
	}

	if ( _transportThread == NULL_THREAD ) return HRR_INVALID_STATE;

	int hrrOutcome = GetThreadStatus();
	if ( IsHrrError( hrrOutcome ) || hrrOutcome == HRR_END || hrrOutcome == HRR_QUIT )
		return hrrOutcome;

	DWORD beginTime = GetTickCount();

	// wait until timeout or transport thread exits
	while ( true )
	{
		if ( 0 < millisecondsToWait )
		{
			DWORD elapsedTime = GetTickCount() - beginTime;
			if ( millisecondsToWait < elapsedTime )
			{
				LOG( "HttpReqResp::WaitForTransportEnd return HRR_TIMEOUT");
				return HRR_TIMEOUT;
			}
		}

		int hrrOutcome = GetThreadStatus();
		if ( IsHrrError( hrrOutcome ) || hrrOutcome == HRR_END || hrrOutcome == HRR_QUIT )
			return hrrOutcome;

		DWORD wait = WaitForSingleObject( _transportThread, 0 );
		if ( wait == WAIT_OBJECT_0 )
		{
			if ( _transportThread != NULL_THREAD )
			{
				LOG( "HttpReqResp::WaitForTransportEnd: WAIT_OBJECT_0, close handle, set _transportThread to NULL_THREAD" );
				bool closed = CloseThread(); assert(closed);
				assert( _transportThread == NULL_THREAD && _threadId == 0 );
			}
			else
			{
				LOG( "_transportThread == NULL_THREAD in HttpReqResp::WaitForTransportEnd so return HRR_INVALID_STATE" );
				assert(false);
				return HRR_INVALID_STATE;
			}

			LOG( "HttpReqResp::WaitForTransportEnd() return HRR_END" );
			return HRR_END;
		}
		else if ( wait == WAIT_FAILED )
		{
			LOG( "WaitForSingleObject() returned WAIT_FAILED in HttpReqResp::WaitForTransportEnd()" );
			assert(false);

			// allow 3 WAIT_FAILEDs before quitting
			DWORD errorCount = 0;
			if ( 3 < errorCount )
			{
				LOG( "Too many WAIT_FAILEDs, return HRR_INVALID_STATE" );
				return HRR_INVALID_STATE;
			}
			else
				++errorCount;

			DWORD error = GetLastError();
			assert( error == ERROR_INVALID_HANDLE );

			bool pump = PumpMessages( 100 );
			if ( !pump )
				break;
		}
		else
		{
			assert( wait == WAIT_TIMEOUT );

			bool pump = PumpMessages( 10 );
			if ( !pump )
				break;
		}
	}

	LOG( "HttpReqResp::WaitForTransportEnd() returns HRR_QUIT" );
	return HRR_QUIT;
}


// performs cleanup if _sendDone && _receiveDone but _transportThread != NULL_THREAD
int HttpReqResp::GetThreadStatus()
{
	if ( _transportThread == NULL_THREAD )
	{
		LOG( "_transportThread == NULL_THREAD in HttpReqResp::GetThreadStatus so return HRR_INVALID_STATE" );
		return HRR_INVALID_STATE;
	}

	if ( _sendDone && _receiveDone )
	{
		LOG( "_sendDone && _receiveDone in HttpReqResp::GetThreadStatus" );
		bool closed = CloseThread(); assert(closed);
		assert( _transportThread == NULL_THREAD && _threadId == 0 );

		return HRR_END;
	}

	return HRR_OK;
}


bool HttpReqResp::CloseThread( bool terminate /*=false*/)
{
	LOG( "HttpReqResp::CloseThread" );

	if ( _transportThread == NULL_THREAD )
	{
		LOG( "HttpReqResp::CloseThread: _transportThread is NULL_THREAD!" );
		assert(false);
		return false;
	}

	if ( terminate )
	{
		assert( _transportThread != NULL_THREAD );
		BOOL terminated = TerminateThread( _transportThread, 1 );
		// This assert() has been occasionally going off for 1 year, but it does
		// not correspond to any bad effects.  So let's turn it off the assert,
		// since its annoying.
		// assert( terminated );

		if ( !terminated )
		{
			assert( _transportThread != NULL_THREAD );
			// just by declaring an exception, the error will appear in the logs
			Exception e( "TerminateThread() failed" );
			// e.TellUser( _title );
		}

		string log = "HttpReqResp::CloseThread: TerminateThread returned ";
		log += ( terminated? "true" : "false" );
		LOG( log );
	}

	/* MSDN: _beginthread():
	_endthread automatically closes the thread handle (whereas _endthreadex does not). 
	Therefore, when using _beginthread and _endthread, do not explicitly close
	the thread handle by calling CloseHandle(). This behavior differs
	from ExitThread().
	Like the Win32 ExitThread API, _endthreadex does not close the thread handle.
	Therefore, when you use _beginthreadex and _endthreadex, you must close the
	thread handle by calling the Win32 CloseHandle API.
	*/

	BOOL closed = CloseHandle( _transportThread );
	assert(closed);

	_transportThread = NULL_THREAD;
	_threadId = 0;
	LOG( "HttpReqResp::CloseThread: _transportThread set to NULL_THREAD" );

	return closed == TRUE;
}


// false means that we encountered WM_CLOSE or WM_QUIT
bool HttpReqResp::PumpMessages( DWORD millisecondsToPump )
{
	DWORD beginTime = GetTickCount();

	while ( GetTickCount() - beginTime < millisecondsToPump )
	{
		MSG m;
		BOOL msgFound = PeekMessage( &m, NULL, 0, 0, PM_NOREMOVE );
		if ( msgFound )
		{
			BOOL got = GetMessage( &m, NULL, 0, 0 ); assert( got );
			if ( got )
			{
				TranslateMessage( &m ); 
				DispatchMessage( &m ); 
			}

			DWORD msgId = LOWORD( m.message );

			if ( msgId == WM_QUIT || msgId == WM_CLOSE )
				return false;
		}
	}

	return true;
}


double HttpReqResp::WaitTime( DWORD bytes ) // in milliseconds
{
	return s_waitTimeScaler * bytes;
}


// throws
int HttpReqResp::SendReceiveWinsock()
{
	if ( !IsSet())
	{
		assert(false);
		return HRR_INVALID_STATE;
	}

	int rc = -1;
    WSADATA wsdata;

    rc = WSAStartup( MAKEWORD(1,1), &wsdata );
	if ( rc != 0 )
		throw WsaStartupException(
					"WSAStartup failed in HttpRepResp::SendReceiveWinsock()",
					"Your WinSock service is not working." );
			
	SOCKET s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( s == INVALID_SOCKET )
		throw SocketException(
					"socket() returned INVALID_SOCKET in HttpReqResp::SendReceiveWinsock()",
					"Unable to open a communication channel." );

	assert( inet_addr( _server->GetAddress().c_str()) != INADDR_NONE );
	
	// probably unneeded
	struct in_addr remote_addr = {0};
	remote_addr.S_un.S_addr = inet_addr( _server->GetAddress().c_str());

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons( _server->GetPort());
	// server.sin_addr = remote_addr;
	server.sin_addr.s_addr = inet_addr( _server->GetAddress().c_str());

	rc = connect( s, (struct sockaddr *) &server, sizeof( server ));
	if ( rc != 0 )
	{
		assert( rc == SOCKET_ERROR );
		throw SocketException(
					"connect() returned SOCKET_ERROR in HttpReqResp::SendReceiveWinsock()",
					"Unable to connect to the server." );
	}

	// making sure the coast is clear...select()
#ifdef _DEBUG
	{
		fd_set writeFDS, oobFDS;
		FD_ZERO( &writeFDS );
		FD_ZERO( &oobFDS );

		FD_SET( s, &writeFDS );
		FD_SET( s, &oobFDS );

		struct timeval timeOut = {0};
		timeOut.tv_sec = 1;
		timeOut.tv_usec = 0;

		int ready = select( -1, NULL, &writeFDS, &oobFDS, &timeOut );                        
		assert( ready != SOCKET_ERROR );
		assert( 0 < ready );
		assert( FD_ISSET( s, &writeFDS ));
		assert( !FD_ISSET( s, &oobFDS ));

		assert( GetLastError() == 0 );
		assert( WSAGetLastError() == 0 );
	}
#endif

	SetLastError(0);
	WSASetLastError(0);

	SendWinsock( s );
	ReceiveWinsock( s );

	rc = closesocket( s );
	assert( rc == 0 );

	rc = WSACleanup();

	return HRR_OK;
}


void HttpReqResp::SendWinsock( SOCKET s )
{
	// Log
	{
		string msg = "HttpReqResp::SendWinsock: ";
		msg += "To: ";
		msg += _server->GetString();
		msg += Logger::s_lineEnd;
		msg += _request->GetHead();
		LOG( msg.c_str());
	}

	assert( _bytesSent == 0 && _bytesReceived == 0 );
	assert( !_sendDone && !_receiveDone );

	// send request and headers
	const string requestHead = _request->GetHead();
	int sent = send( s, requestHead.c_str(), requestHead.length(), 0 );
	if ( sent == SOCKET_ERROR || sent < requestHead.length())
		throw SocketException(
					"send() returned SOCKET_ERROR in HttpReqResp::SendWinsock(), 1",
					"Unable to send data." );

	// send body
	if ( _request->GetBody() != NULL && 0 < _request->GetBodyLength())
	{
		sent = send( s, _request->GetBody(), _request->GetBodyLength(), 0 );
		if ( sent == SOCKET_ERROR || sent < _request->GetBodyLength())
			throw SocketException(
						"send() returned SOCKET_ERROR in HttpReqResp::SendWinsock(), 2",
						"Unable to send data to server." );
	}

	_sendDone = true;

	LOG( "HttpReqResp::SendWinsock: _sendDone = true" );
}


void HttpReqResp::ReceiveWinsock( SOCKET s )
{
	// peek for response
#ifdef _DEBUG
	{
		DWORD start = GetTickCount();
		int pending = recv( s, NULL, 0, MSG_PEEK );
		assert( pending != SOCKET_ERROR && 0 <= pending );
		DWORD stop = GetTickCount();
		// sometimes recv(MSG_PEEK) takes a long time! investigate
		assert( stop - start < 1000 );
	}
#endif

	assert( _mtu == 1460 );
	char buf[ 1460 + 1 ] = "";

	char *body = NULL;
	DWORD bodyBufLen = 0; // set to statedBodyLen + _mtu
	string statedBodyLen; // from headers
	DWORD actualBodyLen = 0;

	string head;
	DWORD bytesReceived = 0;

	int received = recv( s, buf, _mtu, 0 );
	while ( received != SOCKET_ERROR && 0 < received )
	{
		if ( body == NULL )
		{
			// necessary?
			buf[ received ] = '\0';

			head += buf;
			bytesReceived += received;

			// If we find the end of the headers, set
			// _body, bodyBufLen,  statedBodyLen, and actualBodyLen
			DWORD headEnd = head.rfind( "\r\n\r\n" );
			if ( headEnd != string::npos )
			{
				// trim off the body part
				head.resize( headEnd + 4 );

				DWORD set = _response.SetHead( head.c_str(), head.length());
				assert( set == head.length());

				if ( _request->GetMethod() == "HEAD" )
					break;

				DWORD iContentLength = head.find( "Content-Length: " );
				if ( iContentLength == string::npos )
					{ assert(false); return; }

				assert( head.find( "Content-Length: ", iContentLength + 16 ) == string::npos );

				DWORD iBodySize = iContentLength + 16;
				DWORD endOfBodySize = head.find( "\r", iBodySize );
				if ( endOfBodySize == string::npos )
					{ assert(false); return; }

				DWORD lengthOfBodySize = endOfBodySize - iBodySize;
				if ( 32 <= lengthOfBodySize || lengthOfBodySize == 0 )
					{ assert(false); return; }

				// set statedBodyLen and nStatedBodyLen
				DWORD nStatedBodyLen = 0;
				{
					char szContentLength[32] = "";
					const char *pBodySize = head.c_str() + iBodySize;
					strncpy( szContentLength, pBodySize, lengthOfBodySize );
					assert( szContentLength[ lengthOfBodySize ] == '\0' );
					statedBodyLen = szContentLength;
					nStatedBodyLen = atoi( szContentLength );
				}

				// set  bodyBufLen, actualBodyLen, and actualBodyLen
				bodyBufLen = nStatedBodyLen + _mtu;
				_response.AllocateBody( bodyBufLen );
				actualBodyLen = bytesReceived - ( headEnd + 4 );
				bool app = _response.AppendToBody( buf + received - actualBodyLen, actualBodyLen );
				assert(app);
			}
		}
		else if ( actualBodyLen + received < bodyBufLen )
		{
			bool app = _response.AppendToBody( buf, received );
			assert(app);
			actualBodyLen += received;
		}
		else { assert(false); return; }

		received = recv( s, buf, _mtu, 0 );
	}
	assert( received != SOCKET_ERROR );

	// no more to recv()
#ifdef _DEBUG
	{
		int pending = recv( s, NULL, 0, MSG_PEEK );
		assert( pending == SOCKET_ERROR || pending < 1 );
	}
#endif

	LOG( "HttpReqResp::ReceiveWinsock: _receiveDone = true" );
	_receiveDone = true;
}


int HttpReqResp::SendReceiveWininet()
{
	if ( !IsSet())
	{
		assert(false);
		return HRR_INVALID_STATE;
	}

	HINTERNET hOpen = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;
	try
	{
		hOpen = InternetOpen( _title.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, 
								NULL, 0, INTERNET_INVALID_PORT_NUMBER  );
		if ( hOpen == NULL )
			throw Exception(
				"InternetOpen() failed in HttpReqResp::SendReceiveWininet()",
				"Unable to connect to the Internet." );

		hConnect = InternetConnect( hOpen, _server->GetAddress().c_str(),
									_server->GetPort(), NULL, NULL,
									INTERNET_SERVICE_HTTP, 0, 0 );
		if ( hConnect == NULL )
		{
			throw Exception(
				"InternetConnect() failed in HttpReqResp::SendReceiveWininet()",
				"Unable to connect to the server." );
		}

		// create an HTTP request
		hRequest = HttpOpenRequest( hConnect, _request->GetMethod().c_str(),
								_request->GetUrl().c_str(), "HTTP/1.1",
								"", NULL, INTERNET_FLAG_NO_CACHE_WRITE, 0 );
		if ( hRequest == NULL )
		{
			throw Exception(
				"HttpOpenRequest() failed in HttpReqResp::SendReceiveWininet()",
				"Your Wininet service is not working." );
		}

		SendWininet( hRequest );
		bool received = ReceiveWininet( hRequest );
	}
	catch (...)
	{
		if ( !_sendDone )
		{
			LOG( "HttpReqResp::SendReceiveWininet: unknown exception, _sendDone is false, set to true" );
			_sendDone = true;
		}
		if ( !_receiveDone )
		{
			LOG( "HttpReqResp::SendReceiveWininet: unknown exception, _receiveDone is false, set to true" );
			_receiveDone = true;
		}

		BOOL close;
		if ( hRequest ) { close = InternetCloseHandle( hRequest ); assert( close ); }
		if ( hConnect ) { close = InternetCloseHandle( hConnect ); assert( close ); }
		if ( hOpen ) { close = InternetCloseHandle( hOpen ); assert( close ); }

		throw;		
	}

	if ( !_sendDone )
	{
		LOG( "HttpReqResp::SendReceiveWininet: _sendDone is false, set to true" );
		_sendDone = true;
	}
	if ( !_receiveDone )
	{
		LOG( "HttpReqResp::SendReceiveWininet: _receiveDone is false, set to true" );
		_receiveDone = true;
	}

	BOOL close;
	if ( hRequest ) { close = InternetCloseHandle( hRequest ); assert( close ); }
	if ( hConnect ) { close = InternetCloseHandle( hConnect ); assert( close ); }
	if ( hOpen ) { close = InternetCloseHandle( hOpen ); assert( close ); }

	return HRR_OK;
}


bool HttpReqResp::SendWininet( HINTERNET hRequest )
{
	if ( hRequest == NULL ) { assert(false); return false; }
	if ( !IsSet()) { assert(false); return false; }

	// Log
	{
		string msg = "HttpReqResp::SendWininet: ";
		msg += "To: ";
		msg += _server->GetString();
		msg += Logger::s_lineEnd;
		msg += _request->GetHead();
		LOG( msg.c_str());
	}

	assert( _bytesSent == 0 && _bytesReceived == 0 );
	assert( !_sendDone && !_receiveDone );
	assert( _mtu == 1460 );

	DWORD bodyLen = _request->GetBodyLength();
	const char *body =  _request->GetBody();

	INTERNET_BUFFERS bufferIn = {0};

	bufferIn.dwStructSize = sizeof( INTERNET_BUFFERS );

	string headers = _request->_headers.GetString();
	bufferIn.lpcszHeader = headers.c_str();
	bufferIn.dwHeadersLength = headers.size();
	bufferIn.dwHeadersTotal = headers.size();

	if ( 0 < bodyLen )
	{
		bufferIn.dwBufferTotal = bodyLen;
		bufferIn.dwBufferLength = _mtu;
	}

	// HSR_INITIATE iterative operation (completed by HttpEndRequest)
	if ( !HttpSendRequestEx( hRequest, &bufferIn, NULL, HSR_INITIATE, 0 ))
		throw Exception(
				"HttpSendRequestEx() failed in HttpReqResp::SendWininet()",
				"An error occurred while sending data to the server. (1)" );

	if ( 0 < bodyLen && body != NULL )
	{
		while ( _bytesSent < bodyLen && _mtu <= bodyLen - _bytesSent )
		{
			DWORD bytesWritten = 0;

			BOOL outcome = InternetWriteFile( hRequest,
								body + _bytesSent, _mtu, &bytesWritten );
			if ( !outcome )
				throw Exception(
					"InternetWriteFile() failed in HttpReqResp::SendWininet()",
					"An error occurred while sending data to the server. (2)" );

			assert( _mtu == bytesWritten );

			_bytesSent += bytesWritten;
		}

		DWORD leftToSend = bodyLen - _bytesSent;
		assert( leftToSend < _mtu );

		if ( 0 < leftToSend )
		{
			DWORD bytesWritten = 0;
			BOOL outcome = InternetWriteFile( hRequest,
								body + _bytesSent, leftToSend, &bytesWritten );
			if ( !outcome )
				throw Exception(
					"InternetWriteFile() failed in HttpReqResp::SendWininet()",
					"An error occurred while sending data to the server. (5)" );

			assert( leftToSend == bytesWritten );

			_bytesSent += leftToSend;
		}

		if ( _bytesSent == bodyLen )
			LOG( "All of HTTP body sent in HttpReqResp::SendWininet" );
	}
	assert( _bytesSent == bodyLen );

	if( !HttpEndRequest( hRequest, NULL, HSR_INITIATE, 0 ))
		throw Exception(
				"HttpEndRequest() failed in HttpReqResp::SendWininet()",
				"An error occurred while sending data to the server. (6)" );
	// the above Exception was thrown when an Apache log file filled up

	LOG( "HttpReqResp::SendWininet: _sendDone = true" );
	_sendDone = true;

	return _bytesSent == bodyLen;
}


// Define "head": response code line and headers.
void HttpReqResp::ReceiveHead( HINTERNET hRequest )
{
	// Get HTTP statusHeaders, sent by server.
	// Find out how much memory we need to hold all headers.
	// We won't check error code of this call, since it
	// will fail with error 122  (The data area passed to a system 
	// call is too small).

	DWORD headSize = 0;
	BOOL query = HttpQueryInfo( hRequest, HTTP_QUERY_RAW_HEADERS_CRLF,NULL,
											&headSize, NULL );
	assert( !query && GetLastError() == ERROR_INSUFFICIENT_BUFFER );
	SetLastError(0);

	char *head = new char[ headSize + 1 ];

	if ( !HttpQueryInfo( hRequest, HTTP_QUERY_RAW_HEADERS_CRLF,
			(LPVOID) head, &headSize, NULL ))
	{
		delete [] head;
		throw Exception(
			"HttpQueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF) failed in HttpReqResp::ReceiveHead()",
			"An error has occurred in the response from the server. (1)" );
	}

	head[ headSize ] = '\0';

	_response.SetHead( head );

	delete [] head;

	_headReceived = true;
}


bool HttpReqResp::ReceiveBodySize( HINTERNET hRequest, DWORD *bodySize )
{
	if ( bodySize == NULL ) { assert(false); return false; }

	// get size of HTTP body
	char szBodySize[16] = "";
	DWORD dwBodySizeLen = sizeof( szBodySize );

	if ( !HttpQueryInfo( hRequest, HTTP_QUERY_CONTENT_LENGTH,
		(LPVOID) szBodySize, &dwBodySizeLen, NULL ))
	{
		DWORD error = GetLastError();
		if ( error != ERROR_HTTP_HEADER_NOT_FOUND )
			throw Exception(
				"HttpQueryInfo(HTTP_QUERY_CONTENT_LENGTH) failed in HttpReqResp::ReceiveBodySize()",
				"An error has occurred in the response from the server. (2)" );
		
		// In spite of the ERROR_HTTP_HEADER_NOT_FOUND error, there
		// usually is still a body...  What's going on?
	}

	if ( szBodySize[0] == '\0' )
		return false;

#ifdef _DEBUG
	int bodySizeLen = strlen( szBodySize );
	for ( int i = 0; i < bodySizeLen; ++i )
	{
		assert( isdigit( szBodySize[i]));
	}
#endif

	*bodySize = atoi( szBodySize );
	return true;
}


bool HttpReqResp::ReceiveWininet( HINTERNET hRequest )
{
	if ( hRequest == NULL ) { assert(false); return false; }

	assert( _sendDone && !_receiveDone  );

	assert( !_response.IsSet());
	assert( _bytesReceived == 0 );

	ReceiveHead( hRequest );

	// log
	{
		string msg = "HttpReqResp::ReceiveWininet:\r\n";
		msg += _response.GetCodeAndPhrase();
		msg += Logger::s_lineEnd;
		msg += _response._headers.GetString();
		LOG( msg.c_str());
	}

	DWORD bodyLenFromHeader = 0;
	bool getExpected = _response.GetExpectedContentLength( &bodyLenFromHeader );
	if ( !getExpected )
		LOG( "HttpReqResp::ReceiveWininet: _response.GetExpectedContentLength return false" );

	// on my Win2000 Pro, receiveBodySize is false, but I still get a body below
	DWORD bodyLenFromWininet = 0;
	bool receiveBodySize = ReceiveBodySize( hRequest, &bodyLenFromWininet );
	if ( !receiveBodySize )
		LOG( "HttpReqResp::ReceiveWininet: ReceiveBodySize returned false" );

	// getExpected and receiveBodySize => bodyLenFromHeader == receiveBodySize
	assert( !getExpected || !receiveBodySize || bodyLenFromHeader == bodyLenFromWininet );

	// If we know how big the body is, tell _response to allocate a big buffer for
	// the body.  One big allocation is *much* faster than a series of smaller ones.
	if ( receiveBodySize )
	{
		_response.AllocateBody( bodyLenFromWininet );
	}
	else if ( getExpected )
	{
		_response.AllocateBody( bodyLenFromHeader );
	}

	assert( _mtu == 1460 );
	char buf[ 1460 + 1 ] = "";

	DWORD bytesRead = 0;

	BOOL gotSome = InternetReadFile( hRequest, (LPVOID) buf, _mtu, &bytesRead );
	while ( gotSome && 0 < bytesRead )
	{
		if ( _bytesReceived == 0 )
			LOG( "HttpReqResp::ReceiveWininet: received first part of response buf" );

		assert( bytesRead <= _mtu );
		_bytesReceived += bytesRead;

		bool add = _response.AppendToBody( buf, bytesRead );
		if ( !add ) { assert(false); return false; }

		buf[0] = '\0';
		bytesRead = 0;
		gotSome = InternetReadFile( hRequest, (LPVOID) buf, _mtu, &bytesRead );
	}

	if ( !gotSome )
	{
		throw Exception(
			"InternetReadFile() failed in HttpReqResp::ReceiveWininet()",
			"An error occurred while receiving data from the server." );
	}

	if ( _request->GetMethod() != "HEAD" )
	{
		// ReceiveBodySize() returns true => _bytesReceived == bodyLenFromWininet
		assert( !receiveBodySize || _bytesReceived == bodyLenFromWininet );
		// _response.GetExpectedContentLength() => _bytesReceived == bodyLenFromHeader
		assert( !getExpected || _bytesReceived == bodyLenFromHeader );
	}

	// log
	{
		char szBytesReceived[16] = "";
		sprintf( szBytesReceived, "%d", _bytesReceived );

		if ( getExpected && bodyLenFromHeader != _bytesReceived )
		{
			char szBodyLenFromHeader[16] = "";
			sprintf( szBodyLenFromHeader, "%d", bodyLenFromHeader );
			string msg = "HttpReqResp::ReceiveWininet: bodyLenFromHeader is ";
			msg += szBodyLenFromHeader;
			msg +=", _bytesReceived is ";
			msg += szBytesReceived;
			LOG( msg );
		}
		if ( receiveBodySize && bodyLenFromWininet != _bytesReceived )
		{
			char szBodyLenFromWininet[16] = "";
			sprintf( szBodyLenFromWininet, "%d", bodyLenFromWininet );
			string msg = "HttpReqResp::ReceiveWininet: bodyLenFromWininet is ";
			msg += szBodyLenFromWininet;
			msg +=", _bytesReceived is ";
			msg += szBytesReceived;
			LOG( msg );
		}

		char szBodySize[16] = "";
		sprintf( szBodySize, "%d", _response.GetActualContentLength());
		string msg = "HttpReqResp::ReceiveWininet: buf size in bytes is ";
		msg += szBodySize;
		LOG( msg );

		if ( 0 < bodyLenFromHeader && bodyLenFromHeader == _bytesReceived )
			LOG( "All of expected HTTP buf received in HttpReqResp::ReceiveWininet" );
	}

	LOG( "HttpReqResp::ReceiveWininet: _receiveDone = true" );
	_receiveDone = true;

	return true;
}


// if _sendDone is false, you must Suspend() first, because
// _bytesSent is still changing.
short HttpReqResp::CalculatePercentSend() const
{
	if ( !IsSet())
	{
		assert(false);
		return 0;
	}

	DWORD bodySize = _request->GetBodyLength();

	if ( bodySize == 0 )
		return 100;

	short percent = short(( _bytesSent * 100 ) / bodySize );
	if ( percent < 0 ) { assert(false); return 0; }
	if ( 100 < percent ) { assert(false); return 100; }

	char szPercent[8] = "";
	sprintf( szPercent, "%d", percent );
	LOG( string( "HttpReqResp::CalculatePercentSend: " ) + szPercent );
	
	return percent;
}


// may return HRR error code
short HttpReqResp::GetPercentSend() const
{
	LOG( "HttpReqResp::GetPercentSend" );

	if ( _sendDone )
		return CalculatePercentSend();
	
	if ( _transportThread == NULL_THREAD || !IsSet())
	{
		assert(false);
		return _transportThread == NULL_THREAD ? HRR_NO_THREAD : HRR_INVALID_STATE;
	}

	if ( _bytesSent == 0 )
		return 0;

	bool suspend = Suspend();
	if ( !suspend )
		return HRR_SUSPEND_THREAD_FAILURE;

	short percent = CalculatePercentSend();

	bool resume = Resume();
	if ( !resume )
		return HRR_RESUME_THREAD_FAILURE;

	return percent;
}


// if _receiveDone is false, you must Suspend() first, because
// _bytesReceived is still changing.
short HttpReqResp::CalculatePercentReceive() const
{
	DWORD contentLength = 0;
	bool getExpLen = _response.GetExpectedContentLength( &contentLength );
	if ( !getExpLen )
	{
		if ( _receiveDone )
		{
			// If there is no "Content-Length", but we're done receiving
			// then we'll say we've received 100% because that is less
			// misleading than any other answer.
			LOG( "HttpReqResp::CalculatePercentReceive: _receiveDone "
					"but no Content-Length, return 100" );
			//assert(false);
			return 100;
		}

		return 0;
	}

	if ( contentLength == 0 )
		return 100;

	short percent = short(( _bytesReceived * 100 ) / contentLength );
	if ( percent < 0 ) { assert(false); return 0; }
	if ( 100 < percent ) { assert(false); return 100; }

	char szPercent[8] = "";
	sprintf( szPercent, "%d", percent );
	LOG( string( "HttpReqResp::CalculatePercentReceive: " ) + szPercent );

	return percent;
}


// may return HRR error code
short HttpReqResp::GetPercentReceive() const
{
	LOG( "HttpReqResp::GetPercentReceive" );

	if ( _receiveDone )
		return CalculatePercentReceive();

	if ( _transportThread == NULL_THREAD )
	{
		assert(false);
		return HRR_NO_THREAD;
	}

	if ( _bytesReceived == 0 )
		return 0;

	bool suspend = Suspend();
	if ( !suspend )
		return HRR_SUSPEND_THREAD_FAILURE;

	short percent = CalculatePercentReceive();

	bool resume = Resume();
	if ( !resume )
		return HRR_RESUME_THREAD_FAILURE;

	if ( percent < 0 )
	{
		assert(false);
		percent = 0;
	}

	return percent;
}


DWORD HttpReqResp::GetBytesSent() const
{
	return _bytesSent;
}


DWORD HttpReqResp::GetBytesReceived() const
{
	return _bytesReceived;
}


bool HttpReqResp::IsSendDone() const
{
	return _sendDone;
}


bool HttpReqResp::IsHeadReceived() const
{
	return _headReceived;
}


bool HttpReqResp::IsReceiveDone() const
{
	return _receiveDone;
}


bool HttpReqResp::WasTransportSuccessful() const
{
	return IsSet() && GetPercentSend() == 100 && GetPercentReceive() == 100;
}


DWORD HttpReqResp::SetCookies() const
{
	if ( _server == NULL || !_server->IsSet()
			|| !_response.IsSet()) { assert(false); return 0; }

	return _response._headers.SetCookies( _server->GetName());
}


bool HttpReqResp::IncludeCookie()
{
	// don't require that _request->IsSet() because headers could be empty
	if ( _server == NULL || !_server->IsSet() || _request == NULL )
		{ assert(false); return false; }
	
	return _request->_headers.AppendCookie( _server->GetName());
}


void HttpReqResp::SetTransactionId( const string &transactionId )
{
	_transactionId.erase();
	_transactionId = transactionId;
}


string HttpReqResp::Test( bool testUi, bool testNetwork,
					const string &serverName, const string &serverAddress )
{
	// Test IncludeCookie() and SetCookies()

	// Test SendReceiveWinsock!  Compare with SendReceiveWininet!

	if ( !testNetwork )
	{
		return "Test HttpReqResp requires network connection\n";
		return false;
	}

	Logger::Start( "SOFTWARE\\Maxis\\The Sims", "HttpReqRespTest.txt" );

	string log = "Test HttpReqResp\n";

	// requires a specific file to be set up on the server
	// log += TestUnencodedDownload( serverName );

	for ( short i = 0; i < 10; ++i )
	{
		char count[16] = "";
		sprintf( count, "count: %d", i );
		LOG( count );
		
		// HRR_DUAL
		log += TestWinsock( serverName, HttpReqResp::HRR_DUAL );
		log += TestHead( serverName, HttpReqResp::HRR_DUAL );
		log += TestUploadLot( serverName, HttpReqResp::HRR_DUAL );
		log += TestDownloadLot( serverName, HttpReqResp::HRR_DUAL );

		// HRR_SINGLE_BLOCKING
		log += TestWinsock( serverName, HttpReqResp::HRR_SINGLE_BLOCKING );
		log += TestHead( serverName, HttpReqResp::HRR_SINGLE_BLOCKING );
		log += TestUploadLot( serverName, HttpReqResp::HRR_SINGLE_BLOCKING );
		log += TestDownloadLot( serverName, HttpReqResp::HRR_SINGLE_BLOCKING );
	}

	Exception::Test(true);

	log += TestErrors( serverName );

	// test GetPercentSend(), GetPercentReceive()

	log += HttpUri::Test();

	return log;
}


string HttpReqResp::TestErrors( const string &serverName )
{
	string log;

	const string boundary = "------T-E-S-T--B-O-U-N-D-A-R-Y-";
	const string endingBoundary = string("--") + boundary + "--";
	const string bodyEnding = string("\r\n") + endingBoundary;

	HttpReqResp hrr( "HttpReqResp::Test()", boundary );

	Server server;
	if ( !server.SetNameAddress( serverName ))
		log += "Server::SetNameAddress() failed\n";

	if ( !IsConnectedToInternet())
		log += "You are not connected to the Internet\n";

	int outcome = HRR_OK;
	try
	{
		// fail because CreateRequest() hasn't been called
		outcome = hrr.SendReceive( HRR_DUAL );
		if ( outcome != HRR_INVALID_STATE )
			log += "SendReceive() failed";
	}
	catch ( const Exception &e )
	{
		e.TellUser( "HttpReqResp::Test" );
		log += e.GetTechnicalPhrase() + "\n";
		return log;
	}

	// download

	if ( !hrr.CreateRequest( &server, HRR_GET ) || !hrr.IsSet())
		log += "Intialize() failed\n";

	if ( !hrr._request->SetUrl( "/" ))
		log += "Request::SetUrl failed\n";

	if ( hrr._request->SetMinimalHeaders() != 1 )
		log += "Request::SetMinimalHeaders() failed\n";

	if ( !hrr.IsTransportReady())
		log += "IsTransportReady() failed\n";

	bool includeCookie = hrr.IncludeCookie();
	if ( !includeCookie )
		log += "IncludeCookie() failed\n";

	list<string> matches;
	// find should be case-insensitive
	DWORD found = hrr._request->_headers.Find( "cOokIe", &matches );
	if ( found == -1 || matches.size() != 1 || matches.begin()->empty())
		log += "IncludeCookie() or Find() failed\n";

	outcome = hrr.SendReceive( HRR_DUAL );
	if ( outcome != HRR_OK )
		log += "SendReceive() failed";

	short waited = hrr.WaitForTransportEnd( 0 );
	if ( waited != HRR_END )
		log += "WaitForTransport(0) failed\n";

	if ( waited == HRR_END )
	{
		log += "Http request:";
		char newline[4] = "\r\n";
		log += newline;
		log += hrr._request->GetHead();
		log += newline;
		log += "Http response:";
		log += newline;
		log += hrr._response.GetCodeAndPhrase();
		log += newline;
		log += hrr._response._headers.GetString();
		log += newline;
		log += "body size in bytes: ";
		char szBodyLen[16] = "";
		sprintf( szBodyLen, "%d", hrr._response.GetActualContentLength());
		log += szBodyLen;
		log += newline;
	}
	else
		log += "WaitForTransport(0) failed\n";

	if ( !hrr.IsSendDone() || !hrr.IsReceiveDone())
		log += "IsSendDone() or IsReceiveDone() failed\n";

	waited = hrr.WaitForTransportEnd( 0 );
	if ( waited != HRR_INVALID_STATE )
		log += "WaitForTransport(0) failed\n";

	return log;
}


string HttpReqResp::TestUploadLot( const string &serverName, HttpReqResp::Threading thr )
{
	string log;

	const string boundary = "------M-A-X-I-S--H-O-T--D-A-T-E--L-O-T-";

	Server server;
	int outcome = HRR_OK;
	try
	{
		if ( !server.SetNameAddress( serverName ))
			log += "Server::SetNameAddress() failed\n";

	}
	catch ( const Exception &e )
	{
		e.TellUser( "HttpReqResp::TestTestUploadLot()" );
		log += e.GetTechnicalPhrase() + "\n";
		return log;
	}

	// test with old style request body
	HttpReqResp hrr( "HttpReqResp::TestUploadLot()", boundary );
	if ( !hrr.CreateRequest( &server, HRR_POST, true )
				|| hrr._request == NULL || hrr._request->IsSet())
		log += "CreateRequest() failed\n";

	HttpUri uri;
	uri.SetUrl( "/teleport/hotdate/ul_manager.php" );
	string loginKey = "login_key=";
	{
		string cookie;
		{
			string cookieUrl = "http://";
			cookieUrl += serverName;
			char cookieData[512] = "";
			DWORD cookieSize = 511;
			InternetGetCookie( cookieUrl.c_str(), NULL, cookieData, &cookieSize );
			cookie = cookieData;
		}
		int mxlg = cookie.find( "MXLG=" );
		int endMxlg = cookie.find( ";", mxlg + 1 );
		mxlg += 5;
		if ( endMxlg == string::npos )
			loginKey += cookie.substr( mxlg, cookie.size() - mxlg );
		else
			loginKey += cookie.substr( mxlg, endMxlg - mxlg );
	}
	uri.AddModifier( loginKey );
	
	if ( !hrr._request->SetUrl( uri.GetString()))
		log += "Request::SetUrl failed\n";

	// set body
	DWORD bodyLength = 0;
	{
		string filePath =
"C:\\Documents and Settings\\gmcclellan\\My Documents\\src\\Depot\\PlayerXProjects\\WebDevelopment\\ClientTech\\Teleporters\\Common\\HttpReqResp\\HttpReqRespTestMfc\\POST data\\PostBody.txt";
		HANDLE hFile = CreateFile( filePath.c_str(), GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
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

		PostRequest *postRequest = static_cast<PostRequest *>( hrr._request );
		postRequest->_body.Set( bodyBuf );
		bodyLength = postRequest->GetBodyLength();

		BOOL closed = CloseHandle( hFile ); assert( closed );

		assert( endOfLot == bodyLength );
	}

	// set headers
	{
		NameValuePair type, length, host, cookie;
		type.Set( "Content-Type: multipart/form-data; boundary=--------M-A-X-I-S--H-O-T--D-A-T-E--L-O-T---" );
		// length
		{
			char buf[16] = "";
			sprintf( buf, "%d", bodyLength );
			length.Set( "Content-Length", buf );

		}
		// host
		{
			string sHost = "Host: ";
			sHost += serverName;
			sHost += ":80";
			host.Set( sHost.c_str());
		}
		// cookie
		{
			string cookieUrl = "http://";
			cookieUrl += serverName;
			char cookieData[512] = "";
			DWORD cookieSize = 511;
			InternetGetCookie( cookieUrl.c_str(), NULL, cookieData, &cookieSize );
			string sCookie = "Cookie: ";
			sCookie += cookieData;
			cookie.Set( sCookie.c_str());
		}
		vector< NameValuePair > headers;
		headers.push_back( type );
		// BoundsChecker reports a dangling pointer in the next push_back.
		headers.push_back( length );
		headers.push_back( host );
		headers.push_back( cookie );

		DWORD app = hrr._request->_headers.Append( headers );
		if ( app != 4 )
			log += "RequestHeaders::Append() failed\n";
	}

	if ( !hrr.IsTransportReady())
		log += "IsTransportReady() failed\n";

	outcome = hrr.SendReceive( thr );
	if ( outcome != HRR_OK )
		log += "SendReceive() failed";

	if ( thr == HttpReqResp::HRR_DUAL )
	{
		// trying to provoke race conditions
		for ( int i = 0; i < 5; ++i )
		{
			Sleep( 500 - ( i * 120 ));
			hrr.GetPercentReceive();
			hrr.GetPercentSend();
		}
		
		short waited = HRR_INVALID_STATE;
		try
		{
			waited = hrr.WaitForTransportEnd( 0 );
			if ( waited != HRR_END )
				log += "WaitForTransport(0) failed\n";
		}
		catch ( const Exception &e )
		{
			e.TellUser( "HttpReqRespTest" );
		}
		catch (...)
		{
			LOG( "Unexpected exception in HttpReqResp::TestUploadLot" );
		}

		if ( waited == HRR_END )
		{
			log += "Http request:";
			char newline[4] = "\r\n";
			log += newline;
			log += hrr._request->GetHead();
			log += newline;
			log += "Http response:";
			log += newline;
			log += hrr._response.GetCodeAndPhrase();
			log += newline;
			log += hrr._response._headers.GetString();
			log += newline;
			log += "body size in bytes: ";
			char szBodyLen[16] = "";
			sprintf( szBodyLen, "%d", hrr._response.GetActualContentLength());
			log += szBodyLen;
			log += newline;
		}
		else
			log += "WaitForTransport(0) failed\n";
	}

	string cap = hrr._response.GetCodeAndPhrase();
	if ( cap.empty() || cap != "200 OK" )
		log += "SendReceive() or GetCodeAndPhrase() failed\n";

	if ( !hrr.IsSendDone() || !hrr.IsReceiveDone())
		log += "IsSendDone() or IsReceiveDone() failed\n";

	return log;
}


string HttpReqResp::TestDownloadLot( const string &serverName, HttpReqResp::Threading thr )
{
	string log;

	Server server;
	int outcome = HRR_OK;
	try
	{
		if ( !server.SetNameAddress( serverName ))
			log += "Server::SetNameAddress() failed\n";

	}
	catch ( const Exception &e )
	{
		e.TellUser( "HttpReqResp::TestTestDownloadLot()" );
		log += e.GetTechnicalPhrase() + "\n";
		return log;
	}

	HttpReqResp hrr( "HttpReqResp::TestDownloadLot()" );
	if ( !hrr.CreateRequest(  &server, HRR_GET ) || hrr._request == NULL || hrr._request->IsSet())
		log += "CreateRequest() failed\n";

	HttpUri uri;
	uri.SetUrl( "/teleport/hotdate/dl_manager.php" );
	uri.AddModifier( "asset_id=117" );
	uri.AddModifier( "asset_type=LOT" );
	string loginKey = "login_key=";
	{
		string cookie;
		{
			string cookieUrl = "http://";
			cookieUrl += serverName;
			char cookieData[512] = "";
			DWORD cookieSize = 511;
			InternetGetCookie( cookieUrl.c_str(), NULL, cookieData, &cookieSize );
			cookie = cookieData;
		}
		int mxlg = cookie.find( "MXLG=" );
		int endMxlg = cookie.find( ";", mxlg + 1 );
		mxlg += 5;
		if ( endMxlg == string::npos )
			loginKey += cookie.substr( mxlg, cookie.size() - mxlg );
		else
			loginKey += cookie.substr( mxlg, endMxlg - mxlg );
	}
	uri.AddModifier( loginKey );

	if ( !hrr._request->SetUrl( uri.GetString()))
		log += "Request::SetUrl failed\n";

	// set headers
	{
		NameValuePair host, cookie;
		// host
		{
			string sHost = "Host: ";
			sHost += serverName;
			sHost += ":80";
			host.Set( sHost.c_str());
		}
		// cookie
		{
			string cookieUrl = "http://";
			cookieUrl += serverName;
			char cookieData[512] = "";
			DWORD cookieSize = 511;
			InternetGetCookie( cookieUrl.c_str(), NULL, cookieData, &cookieSize );
			string sCookie = "Cookie: ";
			sCookie += cookieData;
			cookie.Set( sCookie.c_str());
		}
		vector< NameValuePair > headers;
		headers.push_back( host );
		headers.push_back( cookie );

		DWORD app = hrr._request->_headers.Append( headers );
		if ( app != 2 )
			log += "RequestHeaders::Append() failed\n";
	}

	if ( !hrr.IsTransportReady())
		log += "IsTransportReady() failed\n";

	outcome = hrr.SendReceive( thr );
	if ( outcome != HRR_OK )
		log += "SendReceive() failed";

	if ( thr == HttpReqResp::HRR_DUAL )
	{
		// trying to provoke race conditions
		for ( int i = 0; i < 5; ++i )
		{
			Sleep( 500 - ( i * 120 ));
			hrr.GetPercentReceive();
			hrr.GetPercentSend();
		}
		
		short waited = HRR_INVALID_STATE;
		try
		{
			waited = hrr.WaitForTransportEnd( 0 );
			if ( waited != HRR_END )
				log += "WaitForTransport(0) failed\n";
		}
		catch ( const Exception &e )
		{
			e.TellUser( "HttpReqRespTest" );
		}
		catch (...)
		{
			LOG( "Unexpected exception in HttpReqResp::TestDownloadLot" );
		}

		if ( waited == HRR_END )
		{
			log += "Http request:";
			char newline[4] = "\r\n";
			log += newline;
			log += hrr._request->GetHead();
			log += newline;
			log += "Http response:";
			log += newline;
			log += hrr._response.GetCodeAndPhrase();
			log += newline;
			log += hrr._response._headers.GetString();
			log += newline;
			log += "body size in bytes: ";
			char szBodyLen[16] = "";
			sprintf( szBodyLen, "%d", hrr._response.GetActualContentLength());
			log += szBodyLen;
			log += newline;
		}
		else
			log += "WaitForTransport(0) failed\n";
	}

	if ( hrr._response.GetCodeAndPhrase() != "200 OK" )
		log += "SendReceive() or GetCodeAndPhrase() failed\n";

	if ( !hrr.IsSendDone() || !hrr.IsReceiveDone())
		log += "IsSendDone() or IsReceiveDone() failed\n";

	return log;
}


string HttpReqResp::TestHead( const string &serverName, HttpReqResp::Threading thr )
{
	string log;

	const string boundary = "------T-E-S-T--B-O-U-N-D-A-R-Y-";
	const string endingBoundary = string("--") + boundary + "--";
	const string bodyEnding = string("\r\n") + endingBoundary;

	Server server;
	int outcome = HRR_OK;
	try
	{
		if ( !server.SetNameAddress( serverName ))
			log += "Server::SetNameAddress() failed\n";
	}
	catch ( const Exception &e )
	{
		e.TellUser( "HttpReqResp::TestHead" );
		log += e.GetTechnicalPhrase() + "\n";
		return log;
	}

	HttpReqResp hrr( "HttpReqResp::TestHead()", boundary );
	if ( !hrr.CreateRequest( &server, HRR_HEAD ) || hrr._request == NULL || hrr._request->IsSet())
		log += "CreateRequest() failed\n";

	HttpUri uri;
	uri.SetUrl( "/teleport/hotdate/dl_manager.php" );
	uri.AddModifier( "asset_id=117" );
	uri.AddModifier( "asset_type=LOT" );
	string loginKey = "login_key=";
	{
		string cookie;
		{
			string cookieUrl = "http://";
			cookieUrl += serverName;
			char cookieData[512] = "";
			DWORD cookieSize = 511;
			InternetGetCookie( cookieUrl.c_str(), NULL, cookieData, &cookieSize );
			cookie = cookieData;
		}
		int mxlg = cookie.find( "MXLG=" );
		int endMxlg = cookie.find( ";", mxlg + 1 );
		mxlg += 5;
		if ( endMxlg == string::npos )
			loginKey += cookie.substr( mxlg, cookie.size() - mxlg );
		else
			loginKey += cookie.substr( mxlg, endMxlg - mxlg );
	}
	uri.AddModifier( loginKey );

	if ( !hrr._request->SetUrl( uri.GetString()))
		log += "Request::SetUrl failed\n";

	if ( hrr._request->SetMinimalHeaders() != 1 )
		log += "Request::SetMinimalHeaders() failed\n";

	if ( !hrr.IsTransportReady())
		log += "IsTransportReady() failed\n";

	bool includeCookie = hrr.IncludeCookie();
	if ( !includeCookie )
		log += "IncludeCookie() failed\n";

	list<string> matches;
	// find should be case-insensitive
	DWORD found = hrr._request->_headers.Find( "cOokIe", &matches );
	if ( found == -1 || matches.size() != 1 || matches.begin()->empty())
		log += "IncludeCookie() or Find() failed\n";

	outcome = hrr.SendReceive( thr );
	if ( outcome != HRR_OK )
		log += "SendReceive() failed";

	if ( thr == HttpReqResp::HRR_DUAL )
	{
		// trying to provoke race conditions
		for ( int i = 0; i < 5; ++i )
		{
			Sleep( 500 - ( i * 120 ));
			hrr.GetPercentReceive();
			hrr.GetPercentSend();
		}

		short waited = hrr.WaitForTransportEnd( 0 );
		if ( waited != HRR_END )
			log += "WaitForTransport(0) failed\n";

		if ( waited == HRR_END )
		{
			log += "Http request:";
			char newline[4] = "\r\n";
			log += newline;
			log += hrr._request->GetHead();
			log += newline;
			log += "Http response:";
			log += newline;
			log += hrr._response.GetCodeAndPhrase();
			log += newline;
			log += hrr._response._headers.GetString();
			log += newline;
			log += "body size in bytes: ";
			char szBodyLen[16] = "";
			sprintf( szBodyLen, "%d", hrr._response.GetActualContentLength());
			log += szBodyLen;
			log += newline;
		}
		else
			log += "WaitForTransport(0) failed\n";
	}

	if ( !hrr.IsSendDone() || !hrr.IsReceiveDone())
		log += "IsSendDone() or IsReceiveDone() failed\n";

	return log;
}


string HttpReqResp::TestWinsock( const string &serverName, HttpReqResp::Threading thr )
{
	string log;

	const string boundary = "------T-E-S-T--B-O-U-N-D-A-R-Y-";
	const string endingBoundary = string("--") + boundary + "--";
	const string bodyEnding = string("\r\n") + endingBoundary;

	Server server;
	int outcome = HRR_OK;
	try
	{
		if ( !server.SetNameAddress( serverName ))
			log += "Server::SetNameAddress() failed\n";
	}
	catch ( const Exception &e )
	{
		e.TellUser( "HttpReqResp::Test" );
		log += e.GetTechnicalPhrase() + "\n";
		return log;
	}
	HttpReqResp hrr( "HttpReqResp::Test()", boundary );

	if ( !hrr.CreateRequest( &server, HRR_HEAD ) || hrr._request == NULL || hrr._request->IsSet())
		log += "CreateRequest() failed\n";

	HttpUri uri;
	uri.SetUrl( "/teleport/hotdate/dl_manager.php" );
	uri.AddModifier( "asset_id=117" );
	uri.AddModifier( "asset_type=LOT" );
	string loginKey = "login_key=";
	{
		string cookie;
		{
			string cookieUrl = "http://";
			cookieUrl += serverName;
			char cookieData[512] = "";
			DWORD cookieSize = 511;
			InternetGetCookie( cookieUrl.c_str(), NULL, cookieData, &cookieSize );
			cookie = cookieData;
		}
		if ( !cookie.empty())
		{
			int mxlg = cookie.find( "MXLG=" );
			int endMxlg = cookie.find( ";", mxlg + 1 );
			mxlg += 5;
			if ( endMxlg == string::npos )
				loginKey += cookie.substr( mxlg, cookie.size() - mxlg );
			else
				loginKey += cookie.substr( mxlg, endMxlg - mxlg );
		}
	}
	uri.AddModifier( loginKey );

	if ( !hrr._request->SetUrl( uri.GetString()))
		log += "Request::SetUrl failed\n";

	if ( hrr._request->SetMinimalHeaders() != 1 )
		log += "Request::SetMinimalHeaders() failed\n";

	if ( !hrr.IsTransportReady())
		log += "IsTransportReady() failed\n";

	bool includeCookie = hrr.IncludeCookie();
	if ( !includeCookie )
		log += "IncludeCookie() failed\n";

	list<string> matches;
	// find should be case-insensitive
	DWORD found = hrr._request->_headers.Find( "cOokIe", &matches );
	if ( found == -1 || matches.size() != 1 || matches.begin()->empty())
		log += "IncludeCookie() or Find() failed\n";

	outcome = hrr.SendReceive(  thr, HRR_WINSOCK );
	if ( outcome != HRR_OK )
		log += "SendReceive() failed";

	if ( thr == HttpReqResp::HRR_DUAL )
	{
		// trying to provoke race conditions
		for ( int i = 0; i < 5; ++i )
		{
			Sleep( 500 - ( i * 120 ));
			hrr.GetPercentReceive();
			hrr.GetPercentSend();
		}

		short waited = hrr.WaitForTransportEnd( 0 );
		if ( waited != HRR_END )
			log += "WaitForTransport(0) failed\n";

		if ( waited == HRR_END )
		{
			log += "Http request:";
			char newline[4] = "\r\n";
			log += newline;
			log += hrr._request->GetHead();
			log += newline;
			log += "Http response:";
			log += newline;
			log += hrr._response.GetCodeAndPhrase();
			log += newline;
			log += hrr._response._headers.GetString();
			log += newline;
			log += "body size in bytes: ";
			char szBodyLen[16] = "";
			sprintf( szBodyLen, "%d", hrr._response.GetActualContentLength());
			log += szBodyLen;
			log += newline;
		}
		else
		log += "WaitForTransport(0) failed\n";
	}

	if ( !hrr.IsSendDone() || !hrr.IsReceiveDone())
		log += "IsSendDone() or IsReceiveDone() failed\n";

	return log;
}



string HttpReqResp::TestUnencodedDownload( const string &serverName )
{
	string log;

	const string boundary = "------T-E-S-T--B-O-U-N-D-A-R-Y-";
	const string endingBoundary = string("--") + boundary + "--";
	const string bodyEnding = string("\r\n") + endingBoundary;

	Server server;
	int outcome = HRR_OK;
	try
	{
		if ( !server.SetNameAddress( serverName ))
			log += "Server::SetNameAddress() failed\n";
	}
	catch ( const Exception &e )
	{
		e.TellUser( "HttpReqResp::TestDownload()" );
		log += e.GetTechnicalPhrase() + "\n";
		return log;
	}

	HttpReqResp hrr( "HttpReqResp::TestDownload()", boundary );
	if ( !hrr.CreateRequest( &server, HRR_GET ) || hrr._request == NULL || hrr._request->IsSet())
		log += "CreateRequest() failed\n";

	// http://nmolon.thesims.max.ad.ea.com/downloads/getfile_debug.php?filename=/sc4_exchange/SimCityScape/12mb_city.sc4&verbose=Yes
	HttpUri uri;
	uri.SetUrl( "/downloads/getfile_debug.php" );
	uri.AddModifier( "filename=/sc4_exchange/SimCityScape/12mb_city.sc4" );
// IS THIS STILL REQUIRED?
	uri.AddModifier( "base64=No" );
	//uri.AddModifier( "verbose=No" );

	if ( !hrr._request->SetUrl( uri.GetString()))
		log += "Request::SetUrl failed\n";

	if ( hrr._request->SetMinimalHeaders() != 1 )
		log += "Request::SetMinimalHeaders() failed\n";

	if ( !hrr.IsTransportReady())
		log += "IsTransportReady() failed\n";

	bool includeCookie = hrr.IncludeCookie();
	if ( !includeCookie )
		log += "IncludeCookie() failed\n";

	list<string> matches;
	// find should be case-insensitive
	DWORD found = hrr._request->_headers.Find( "cOokIe", &matches );
	if ( found == -1 || matches.size() != 1 || matches.begin()->empty())
		log += "IncludeCookie() or Find() failed\n";

	outcome = hrr.SendReceive( HRR_DUAL );
	if ( outcome != HRR_OK )
		log += "SendReceive() failed";

	for ( int i = 0; i < 5; ++i )
	{
		Sleep( 500 );
		hrr.GetPercentReceive();
		hrr.GetPercentSend();
	}

	short waited = hrr.WaitForTransportEnd( 0 );
	if ( waited != HRR_END )
		log += "WaitForTransport(0) failed\n";

	if ( waited == HRR_END )
	{
		log += "Http request:";
		char newline[4] = "\r\n";
		log += newline;
		log += hrr._request->GetHead();
		log += newline;
		log += "Http response:";
		log += newline;
		log += hrr._response.GetCodeAndPhrase();
		log += newline;
		log += hrr._response._headers.GetString();
		log += newline;
		log += "body size in bytes: ";
		char szBodyLen[16] = "";
		sprintf( szBodyLen, "%d", hrr._response.GetActualContentLength());
		log += szBodyLen;
		log += newline;

		bool save = hrr._response.SaveBodyAsFile( "test.sc4", false );
		if (!save ) log += "SaveBodyAsFile() failed\n";
	}
	else
		log += "WaitForTransport(0) failed\n";

	if ( !hrr.IsSendDone() || !hrr.IsReceiveDone())
		log += "IsSendDone() or IsReceiveDone() failed\n";

	return log;
}


