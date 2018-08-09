// Exception.cpp: implementation of the Exception class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Exception.h"
#include "Error.h"


Exception::Exception( const string &technical, string friendly /*=string()*/)
{
	assert( !technical.empty());

	_technical = technical;
	_friendly = friendly;

	_lastError = ::GetLastError();

	// set _lastErrorPhrase
	LPSTR errorPhrase = NULL;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
					| FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
					_lastError,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &errorPhrase, 0, NULL );

	if ( errorPhrase == NULL )
	{
		_lastErrorPhrase = ExplainExtendedError( _lastError );
		if ( _lastErrorPhrase.empty())
			_lastErrorPhrase = "No description available.";
	}
	else
	{
		_lastErrorPhrase = errorPhrase;
		HLOCAL hl = LocalFree( errorPhrase ); assert( hl == 0 );
	}

	SetLastError(0);

	// trim off trailing white space (including '\n')
	if ( !_lastErrorPhrase.empty())
	{
		int i = _lastErrorPhrase.length() - 1;
		while ( 0 < i && isspace( _lastErrorPhrase.at( i )))
			--i;
		_lastErrorPhrase.resize( i + 1 );
		assert( !_lastErrorPhrase.empty());
	}

	// log
	{
		string explain = "Exception constructor: ";
		explain += "_technical is \"";
		explain += _technical;
		explain += "\", _friendly is \"";
		explain += _friendly;
		explain += "\", _lastErrorPhrase is \"";
		explain += _lastErrorPhrase;
		explain += "\", _lastError is ";
		char number[16] = "";
		sprintf( number, "%d", _lastError );
		explain += number;
		explain += ".";
		LOG( explain.c_str());
	}
}


string Exception::ExplainExtendedError( DWORD lastError )
{
	// see MSDN Q193625: INFO: WinInet Error Codes (12001 through 12156)

	switch ( lastError )
	{
		case 12001:
			return "No more handles could be generated at this time: "
				"ERROR_INTERNET_OUT_OF_HANDLES";

		case 12002:
			return "The request has timed out: ERROR_INTERNET_TIMEOUT";

		case 12004:
			return "An internal error has occurred: "
				"ERROR_INTERNET_INTERNAL_ERROR";

		case 12005:
			return "The URL is invalid: ERROR_INTERNET_INVALID_URL";

		case 12007:
			return "The server name could not be resolved: "
				"ERROR_INTERNET_NAME_NOT_RESOLVED";

		case 12029:
			return "The attempt to connect to the server failed: "
				"ERROR_INTERNET_CANNOT_CONNECT";

		case 12030:
			return "The connection with the server has been terminated: "
				"ERROR_INTERNET_CONNECTION_ABORTED";

		case 12031:
			return "The connection with the server has been reset: "
				"ERROR_INTERNET_CONNECTION_RESET";

		case 12150:
			return "The requested header could not be located: "
				"ERROR_HTTP_HEADER_NOT_FOUND";

		case 12152:
			return "The server response could not be parsed: "
				"ERROR_HTTP_INVALID_SERVER_RESPONSE";
	}

	return "";
}


bool Exception::IsSet() const { return !_technical.empty(); }

string Exception::GetTechnicalPhrase() const { return _technical; }

string Exception::GetFriendlyPhrase() const { return _friendly; }

DWORD Exception::GetTheLastError() const { return _lastError; }

string Exception::GetLastErrorPhrase() const { return _lastErrorPhrase; }

string Exception::GetLastErrorAndPhrase() const
{
	if ( !IsSet()) return "";

	char number[32] = "";
	sprintf( number, "(%d)", _lastError );

	return _lastErrorPhrase + " " + number;
}


// polymorphic, used by TellUser()
string Exception::GetExplanation( const string &title,
									string transactionId /*=string()*/) const
{
	if ( title.empty() || !IsSet())
	{
		assert(false);
		return	"Exception object is in an invalid state.";
	}

	string desc = title;
	desc += " encountered the following error:\n";
	if ( !_friendly.empty())
		desc += _friendly;
	else if (! _lastErrorPhrase.empty())
		desc += _lastErrorPhrase;
	else
	{
		desc += ( _technical + " " );
		char number[32] = "";
		sprintf( number, "(%d)", _lastError );
		desc += number;
	}

	desc += "\n\n";
	desc += "The operation was canceled.";

	if ( !transactionId.empty())
	{
		desc += "\n\n";
		desc += "Maxis Transaction Id: ";
		desc += transactionId;
		desc += "\nUse this Id to identify your error to Maxis.";
	}

	return desc;
}


int Exception::TellUser( const string &title, string transactionId /*=string()*/,
						HWND wnd /*=NULL*/) const
{
	if ( title.empty()) { assert(false); return HRR_INVALID_PARAMETER; }
	if ( !IsSet()) { assert(false); return HRR_INVALID_STATE; }

	// log transaction id
	LOG( "Exception::TellUser: transaction id is " + transactionId );

	string explain = GetExplanation( title, transactionId );

	if ( wnd == NULL )
		wnd = GetTopWindow( NULL );

	return MessageBox( wnd, explain.c_str(), title.c_str(), MB_OK );
}


WsaStartupException::WsaStartupException( const string &technical, string friendly /*=string()*/)
	: Exception( technical, friendly )
{
	_wsaLastError = WSAGetLastError();
	WSASetLastError(0);
	// log
	{
		string explain = "WsaStartupException constructor: ";
		explain += "_wsaLastError is ";
		char number[16] = "";
		sprintf( number, "%d", _wsaLastError );
		explain += number;
		explain += ".";
		LOG( explain.c_str());
	}
}


// polymorphic, used by TellUser()
string WsaStartupException::GetExplanation( const string &title,
									string transactionId /*=string()*/) const
{
	if ( title.empty() || !IsSet())
	{
		assert(false);
		return	"WsaStartupException object is in an invalid state.";
	}

	string desc = title;
	desc += " encountered the following error:\n";
	if ( !_friendly.empty())
		desc += _friendly;
	else
	{
		desc += ( _technical + " " );
		char number[32] = "";
		sprintf( number, "(wsa %d) (%d)", _wsaLastError, _lastError );
		desc += number;
	}

	desc += "\n\n";
	desc += "The operation was canceled.";

	if ( !transactionId.empty())
	{
		desc += "\n\n";
		desc += "Maxis Transaction Id: ";
		desc += transactionId;
		desc += "\nUse this Id to identify your error to Maxis.";
	}

	return desc;
}


SocketException::SocketException( const string &technical, string friendly /*=string()*/)
	: Exception( technical, friendly )
{
	_wsaLastError = WSAGetLastError();
	WSASetLastError(0);
	// log
	{
		string explain = "SocketException constructor: ";
		explain += "_wsaLastError is ";
		char number[16] = "";
		sprintf( number, "%d", _wsaLastError );
		explain += number;
		explain += ".";
		LOG( explain.c_str());
	}
}


// polymorphic, used by TellUser()
string SocketException::GetExplanation( const string &title,
								string transactionId /*=string()*/) const
{
	if ( title.empty() || !IsSet())
	{
		assert(false);
		return	"SocketException object is in an invalid state.";
	}

	string desc = title;
	desc += " encountered the following error:\n";
	if ( !_friendly.empty())
		desc += _friendly;
	else
	{
		desc += ( _technical + " " );
		char number[32] = "";
		sprintf( number, "(wsa %d) (%d)", _wsaLastError, _lastError );
		desc += number;
	}

	desc += "\n\n";
	desc += "The operation was canceled.";

	if ( !transactionId.empty())
	{
		desc += "\n\n";
		desc += "Maxis Transaction Id: ";
		desc += transactionId;
		desc += "\nUse this Id to identify your error to Maxis.";
	}

	return desc;
}


string Exception::Test( bool testUi )
{
	string log;

	log += "Test Exceptions\n";

	LPCSTR t = "this is an example of a technical phrase";
	LPCSTR f = "this is an example of a friendly phrase";
	LPCSTR title = "Exception::Test()";

	assert( GetLastError() == 0 || GetLastError() == 120 );
	if ( GetLastError() != 0 )
		SetLastError( 0 );

	try
	{
		throw Exception( t, f );
	}
	catch ( const Exception &e )
	{
		if ( e.GetTechnicalPhrase().compare( t ) != 0 )
			log += "GetTechnicalPhrase() failed\n";

		if ( e.GetFriendlyPhrase().compare( f ) != 0 )
			log += "GetFriendlyPhrase() failed\n";

		if ( e.GetTheLastError() != 0 )
			log += "GetTheLastError() failed\n";

		if ( testUi )
			e.TellUser( title, "this is my fake transaction id" );
	}
	catch (...)
	{
		log += "Exception() failed\n";
	}

	try
	{
		SetLastError( 1 ); // ERROR_INVALID_FUNCTION, arbitrary choice of error
		throw Exception( t );
	}
	catch ( const Exception &e )
	{
		if ( e.GetLastErrorPhrase().compare( "Incorrect function." ) != 0 )
			log += "GetLastErrorPhrase() failed\n";

		if ( testUi )
			e.TellUser( title, "this is my fake transaction id" );
	}
	catch (...)
	{
		log += "Exception() failed\n";
	}

	try
	{
		throw Exception(
			"You shouldn't see this sentence",
			"I intentionally threw this Exception" );
	}
	catch ( const Exception &e )
	{
		if ( testUi )
			e.TellUser( title, "this is my fake transaction id" );
	}
	catch (...)
	{
		log += "Unexpected exception thrown\n";
	}

	SetLastError( 0xFFFFFFF0 );
	const Exception e( "I intentionally called SetLastError() with an unused error number." );
	if ( testUi )
		e.TellUser( title, "this is my fake transaction id" );
	SetLastError( 0 );

	try
	{
		throw SocketException( "socket exception technical", "socket exception friendly" );
	}
	catch ( Exception e )
	{
		// is is a SocketException object?
		if ( testUi )
			e.TellUser( "If you don't see \"socket exception\", then an internal error has occurred: ",
							"this is my fake transaction id" );
	}

	try
	{
		throw WsaStartupException( "wsa exception technical", "wsa exception friendly" );
	}
	catch ( WsaStartupException e )
	{
		if ( testUi )
			e.TellUser( "If you don't see \"wsa exception\", then an internal error has occurred: ",
							"this is my fake transaction id");
	}
	catch ( Exception e )
	{
		if ( testUi )
			e.TellUser( "If you see this message, an internal error has occurred: ",
							"this is my fake transaction id");
	}

	return log;
}


