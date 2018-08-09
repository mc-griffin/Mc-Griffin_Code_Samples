// Response.cpp: implementation of the Response class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Response.h"
#include "Base64Coder.h"


Response::~Response()
{
	if ( _body != NULL )
		delete [] _body;
	_body = NULL;
}

bool Response::IsSet() const
{
	if ( _statusCode == 0 )
		return false;

	if ( _phrase.empty() || _version.empty())
		return false;

	if ( _headers.Count() == 0 )
		return false;

	return true;
}


void Response::Reset()
{
	_headers.Reset();
	_phrase.erase();
	_version.erase();
	_statusCode = 0;
	if ( _body != NULL )
		delete [] _body;
	_body = NULL;
	_bodyLength = 0;
	_bodyBufLength = 0;
}


string Response::GetCodeAndPhrase() const
{
	if ( !IsSet()) { assert(false); return ""; }
	string s;
	char code[8] = "";
	sprintf( code, "%d", _statusCode );
	s += code;
	s += " ";
	s += _phrase;
	return s;
}


bool Response::GetExpectedContentLength( DWORD *expectedLen ) const
{
	if ( !IsSet() || expectedLen == NULL )
		return false;

	string len;
	bool get = _headers.GetContentLength( &len );
	if ( !get )
		return false;

	*expectedLen = atoi( len.c_str());

	assert( len == _headers.FindOnly( "Content-Length" ));

	return true;
}


DWORD Response::GetActualContentLength() const
{
	return _bodyLength;
}


// Return true iff buffer[index] is the last '\n' of "\r\n\r\n".
bool Response::AtEndOfHead( LPCSTR buffer, DWORD index )
{
	if ( buffer == NULL ) { assert(false); return false; }

	assert( index < strlen( buffer ));

	if ( index < 3 ) // stlen( "\r\n\r\n" ) is 4 and we want to index the last '\n'
		{ assert(false); return false; }

	return buffer[index-3]=='\r'
			&& buffer[index-2] == '\n'
				&& buffer[index-1] == '\r'
					&& buffer[index] == '\n';
}


// Return the number of characters in the head up to and including "\r\n\r\n".
// 0 means error.
DWORD Response::SetHead( LPCSTR head, DWORD len /*=0*/)
{
	// protected from buffer overflow:

	// declare version, statusCode, and phrase
	const int versionBufLen = 9; // "HTTP/1.1"
	char version[versionBufLen] = "";
	const int statusCodeBufLen = 4; 
	char statusCode[statusCodeBufLen] = "";
	const int phraseBufLen = 256; // is there a maximum phrase length?
	char phrase[phraseBufLen] = "";

	DWORD endHead = 0;

	try
	{
		if ( head == NULL || *head == '\0' || isspace( *head ))
			throw "bad input";

		if ( len == 0 )
			len = strlen( head );

		if ( len < 19 ) // strlen( "HTTP/1.1 200 OK\r\n\r\n" ) is 19
			throw "too short";

		// find "\r\n\r\n", set endHead
		endHead = 18;
		while ( endHead < len && !AtEndOfHead( head, endHead ))
			++endHead;

		if ( endHead == len ) throw "no ending CRLFCRLF";

		DWORD i = 0;

		// set version
		while ( i < len && i < versionBufLen && !isspace( head[ i ]))
		{
			version[ i ] = head[ i ];
			++i;
		}
		if ( i >= len || i >= versionBufLen ) throw "incomplete reply 1";
		version[ i ] = '\0';

		// go past white space
		while ( i < len && isspace( head[ i ]))
			++i;
		if ( i >= len ) throw "incomplete reply 2";

		// set statusCode
		DWORD beginStatusCode = i;
		while ( i < len && ( i - beginStatusCode ) < statusCodeBufLen
					&& !isspace( head[ i ]))
		{
			if ( !isdigit( head[ i ])) throw "expected a digit";	
			statusCode[ i - beginStatusCode ] = head[ i ];
			++i;
		}
		if ( i >= len || ( i - beginStatusCode ) >= statusCodeBufLen )
			throw"incomplete reply 3";
		statusCode[ i - beginStatusCode ] = '\0';

		// go past white space
		while ( i < len && isspace( head[ i ]))
			++i;
		if ( i >= len ) throw "incomplete reply 4";

		// set phrase
		DWORD beginPhrase = i;
		while ( i < len && ( i - beginPhrase ) < phraseBufLen
					&& head[ i ] != '\r' )
		{
			phrase[ i - beginPhrase ] = head[ i ];
			++i;
		}
		if ( i >= len || ( i - beginPhrase ) >= phraseBufLen )
			throw "incomplete reply 5";
		phrase[ i - beginPhrase ] = '\0';

		// skip past "\r\n"
		++i;
		if ( i + 1 >= len || head[ i ] != '\n' )
			throw "incomplete reply 6";
		++i;

		assert( !isspace( head[ i ]));
		// shouldn't be necessary
		{
			// go past white space
			while ( i < len && isspace( head[ i ]))
				++i;
			if ( i >= len ) throw "No headers received";
		}

		bool setHeaders = 0 < _headers.AppendLines( head + i );
		if ( !setHeaders )
			throw "_headers.AppendLines failed";
	}
	catch ( LPCSTR s )
	{
		string msg = "In Response::SetHead: ";
		msg + s;
		LOG( msg );
		assert(false);
		return 0;
	}
	catch (...)
	{
		LOG( "unknown exception in Response::SetHead" );
		LOG( head );
		assert(false);
		return 0;
	}

	_version = version;
	_statusCode = atoi( statusCode );
	_phrase = phrase;

	return endHead + 1; // +1 to give the length of the header
}


// bodyBufLen can be the size of the expected body or bigger.
bool Response::AllocateBody( DWORD bodyBufLen )
{
	if ( 0 == bodyBufLen ) { assert(false); return false; }
	_bodyLength = 0;
	++bodyBufLen; // +1 to include a trailing zero
	_body = new char[ bodyBufLen ];
	_bodyBufLength = bodyBufLen;
	return true;
}


bool Response::AppendToBody( LPCSTR bodyPart, DWORD len )
{
	if ( bodyPart == NULL || len == 0 )
		{ assert(false); return false; }

	if ( _body == NULL )
	{
		DWORD newLen = len + 1;// +1 to include a trailing zero
		_body = new char[ newLen ];
		_bodyBufLength = newLen;
	}
	else
	{
		DWORD newLen = _bodyLength + len;
		if ( _bodyBufLength < newLen )
		{
			++newLen;  // +1 to include a trailing zero
			char *newBody = new char[ newLen ];
			memcpy( newBody, _body, _bodyLength );
			delete [] _body;
			_body = newBody;
			_bodyBufLength = newLen;
		}
	}

	memcpy( _body + _bodyLength, bodyPart, len );

	_bodyLength += len;

	if ( _bodyLength < _bodyBufLength )
		_body[ _bodyLength ] = '\0';
	else
	{
		// For GetBody() to work, there always must be a trailing zero.
		// If this assert goes off, you must fix the problem so that
		// _body has a trailing zero.
		assert(false);
	}

	return true;
}


const char *const Response::GetBody() const
{
	assert( _body != NULL && _body[ _bodyLength ] == '\0' );
	return _body;
}


bool Response::GetBody( char *buf, DWORD bufLen ) const
{
	if ( buf == NULL || _body == NULL ) { assert(false); return false; }
	if ( bufLen != _bodyLength ) { assert(false); return false; }
	memcpy( buf, _body, _bodyLength );
	return true;
}


// axioms
// "\rx" && x!="\n" -> "\r\nx"
// "x\n" && x!="\r" -> "x\r\n"
// "\r\n" -> "\r\n"
//
// consequences
// "\r\r\n" -> "\r\n\r\n"
// "\r\n\r" -> "\r\n\r\n"
// "\n\n" -> "\r\n\r\n"
// "\r\r" -> "\r\n\r\n"
bool Response::EnsureCRLF( LPCSTR body, string *winBody )
{
	if ( body == NULL || *body == '\0' || winBody == NULL )
	{
		assert(false);
		return false;
	}

	assert( winBody->empty());
	winBody->erase();

	int len = strlen( body );
	bool justPassedCR = false;
	int i = 0;
	while ( i < len )
	{
		char c = body[i];

		if ( c == '\r' || ( c == '\n' && !justPassedCR ))
			winBody->append( "\r\n", 2 );
		else if ( !justPassedCR )
			winBody->append( 1, c );

		justPassedCR = c == '\r';
		++i;
	}

	return true;
}


// test this
//			THROWS!
bool Response::SaveBodyAsFile( const string &filePath, bool base64decode /*=true*/) const
{
	LOG( string( "Response::SaveBodyAsFile(" ) + filePath + ","
						+ ( base64decode ? "true" : "false" ) + ")" );

	// I should probably do Graham's trick of explicitly turning
	// the file to not read-only.

	HANDLE hFile = CreateFile( filePath.c_str(), GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		string toUser = "Unable to create the file ";
		toUser += filePath;
		throw Exception(
				"CreateFile() failed in Response::SaveBodyAsFile()", toUser );
	}

	bool success = false;

	if ( base64decode )
	{
		BYTE *decoded = NULL;
		DWORD decodedLength = 0;
		{
			Base64Coder coder;

			coder.Decode((BYTE *) _body, _bodyLength );

			decodedLength = coder.GetDecodeBufferLength();
			decoded = new BYTE[ decodedLength ];

			DWORD len = coder.GetDecodeBuffer( decoded, decodedLength );
			assert( decodedLength == len ); 
		}

		DWORD bytesWritten = 0;
		BOOL written = WriteFile( hFile, decoded, decodedLength,
							&bytesWritten, NULL );
		delete [] decoded;

		success = written && bytesWritten == decodedLength;
		assert( success );
	}
	else
	{
		DWORD bytesWritten = 0;
		BOOL written = WriteFile( hFile, _body, _bodyLength, &bytesWritten, NULL );
		success = written && bytesWritten == _bodyLength;
		assert( success );
	}

	BOOL close = CloseHandle( hFile ); ASSERT( close );

	if ( !success )
	{
		string toUser = "Unable to save the file ";
		toUser += filePath;
		throw Exception(
				"WriteFile() failed in Response::SaveBodyAsFile()", toUser );
	}

	return success;
}


string Response::Test( const string &serverName, const string &serverAddress )
{
	// Test SetCookies()

	string log = "Test Response\n";
	
	Response r;

	if ( r.IsSet())
		log += "IsSet() failed\n";

	DWORD dummy = 0;
	if ( r.GetActualContentLength() != 0 || r.GetExpectedContentLength( &dummy ))
		log += "GetActualContentLength() or GetExpectedContentLength() failed\n";

	if ( r.SetHead( NULL ) || r.SetHead( "" )
			|| r.SetHead( " " ))
		log += "SetHead() failed\n";

	string s = "HTTP/1.1 666 This is the phrase.\r\n";
	s += "a: b\r\n";
	s += "Content-Length: 69\r\n";
	s += "c: d\r\n";
	s += "\r\n";

	if ( !r.SetHead( s.c_str()))
		log += "SetHead() failed\n";

	if ( !r.IsSet())
		log += "IsSet() failed\n";

	if ( r._statusCode != 666 )
		log += "SetHead() failed\n";

	if ( r._phrase.compare( "This is the phrase." ) != 0 )
		log += "SetHead() failed\n";

	if ( r._version.compare( "HTTP/1.1" ) != 0 )
		log += "SetHead() failed\n";

	if ( r._headers.GetString() != "a: b\r\nContent-Length: 69\r\nc: d\r\n\r\n" )
		log += "SetHead() failed\n";

	char body1[] = "This is an example of body content.\n";
	char body2[] = "Here's more content.";
	string body = string( body1 ) + body2;

	r.AppendToBody( body1, strlen( body1 ));
	r.AppendToBody( body2, strlen( body2 ));

	DWORD expLen = 0;
	bool getExpLen = r.GetExpectedContentLength( &expLen );
	if ( !getExpLen || expLen != 69 )
		log += "GetContentLength() failed\n";

	if ( r.GetActualContentLength() != body.length())
		log += "GetContentLength() failed\n";

	// Unfinished
	// SaveBodyAsFile()
	char currentDir[ MAX_PATH ] = "";
	{
		DWORD got = GetCurrentDirectory( MAX_PATH, currentDir );
		if ( !got )
			log += "OS error: GetCurrentDirectory() returned FALSE\n";

		char unusualName[] = "__presumably_this_file_name_is_unused__";
		string filePath = string( currentDir ) + "\\" + unusualName;

		if ( GetFileAttributes( filePath.c_str()) != 0xFFFFFFFF ) // exists
			log += "SaveBodyAsFile() failed\n";

		try
		{
			if ( !r.SaveBodyAsFile( filePath.c_str(), false ))
				log += "SaveBodyAsFile() failed\n";
		}
		catch ( const Exception &e )
		{
			// if ( testUi ) ?
			e.TellUser( "Response::Test()" );
			log += "SaveBodyAsFile() failed\n";
		}

		if ( GetFileAttributes( filePath.c_str()) == 0xFFFFFFFF ) // doesn't exist
			log += "SaveBodyAsFile() failed\n";

		FILE *f = fopen( filePath.c_str(), "r" );
		if ( f == NULL )
			log += "SaveBodyAsFile() failed\n";

		const DWORD fileBufLen = 1024;
		char fileBuf[ fileBufLen ] = "";
		DWORD fileLen = fread( fileBuf, 1, fileBufLen, f );
		if ( fileLen != body.length())
			log += "SaveBodyAsFile() failed\n";

		fclose( f );

		string fileContents = fileBuf;
		if ( fileContents != body )
			log += "SaveBodyAsFile() failed\n";

		BOOL deleted = DeleteFile( filePath.c_str()); assert( deleted );
	}

	// EnsureCRLF()
	{
		string a = "";
		if ( EnsureCRLF( NULL, NULL ) || EnsureCRLF( a.c_str(), NULL )
				|| EnsureCRLF( NULL, &a ) || EnsureCRLF( a.c_str(), &a ))
			log += "EnsureCRLF(1) failed\n";

		a = "a";
		if ( EnsureCRLF( NULL, NULL ) || EnsureCRLF( a.c_str(), NULL )
				|| EnsureCRLF( NULL, &a ))
			log += "EnsureCRLF(2) failed\n";

		a = "\n";
		string b;

		if ( !EnsureCRLF( a.c_str(), &b ) || b != "\r\n" )
			log += "EnsureCRLF(3) failed\n";

		a = "\r";
		b.erase();
		if ( !EnsureCRLF( a.c_str(), &b ) || b != "\r\n" )
			log += "EnsureCRLF(4) failed\n";

		a = "\r\n";
		b = "";
		if ( !EnsureCRLF( a.c_str(), &b ) || b != "\r\n" )
			log += "EnsureCRLF(5) failed\n";

		a = "\n\r";
		b.erase();
		if ( !EnsureCRLF( a.c_str(), &b ) || b != "\r\n\r\n" )
			log += "EnsureCRLF(6) failed\n";

		a = "\r\r";
		b = "";
		if ( !EnsureCRLF( a.c_str(), &b ) || b != "\r\n\r\n" )
			log += "EnsureCRLF(7) failed\n";

		a = "\n\n";
		b.erase();
		if ( !EnsureCRLF( a.c_str(), &b ) || b != "\r\n\r\n" )
			log += "EnsureCRLF(8) failed\n";
			
		a = "\r\r\n";
		b = "";
		if ( !EnsureCRLF( a.c_str(), &b ) || b != "\r\n\r\n" )
			log += "EnsureCRLF(9) failed\n";

		a = "\n\r\n";
		b.erase();
		if ( !EnsureCRLF( a.c_str(), &b ) || b != "\r\n\r\n" )
			log += "EnsureCRLF(10) failed\n";

		a = "hey\nthere";
		b = "";
		if ( !EnsureCRLF( a.c_str(), &b ) || b != "hey\r\nthere" )
			log += "EnsureCRLF(11) failed\n";
	}

	return log;
}

// Test Data
#if 0
HTTP/1.1 200 OK
Date: Thu, 17 Jan 2002 00:10:32 GMT
Server: Apache/1.3.22 (Unix) PHP/4.1.1
Set-Cookie: Apache=207.21.8.100.48901011226232938; path=/; expires=Sat, 16-Feb-02 00:10:32 GMT
X-Powered-By: PHP/4.1.1
Set-Cookie: PHPSESSID=9142c3452c968c05ae358793c4a8f6e7; path=/
Expires: Thu, 19 Nov 1981 08:52:00 GMT
Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0
Pragma: no-cache
Connection: close
Transfer-Encoding: chunked
Content-Type: text/html


HTTP/1.1 301 Moved Permanently
Date: Wed, 16 Jan 2002 23:21:22 GMT
Server: Apache/1.3.22 (Unix)
Location: http://localhost:8080/us/
Connection: close
Transfer-Encoding: chunked
Content-Type: text/html; charset=iso-8859-1

e5 
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<HTML><HEAD>
<TITLE>301 Moved Permanently</TITLE>
</HEAD><BODY>
<H1>Moved Permanently</H1>
The document has moved <A HREF="http://localhost:8080/us/">here</A>.<P>
</BODY></HTML>

0



HTTP/1.1 200 OK
Date: Thu, 17 Jan 2002 00:10:56 GMT
Server: Apache/1.3.22 (Unix) PHP/4.1.1
Set-Cookie: Apache=207.21.8.100.48561011226256464; path=/; expires=Sat, 16-Feb-02 00:10:56 GMT
X-Powered-By: PHP/4.1.1
Set-Cookie: PHPSESSID=3688c8533bcb2213bc2ada25cea80ab1; path=/
Expires: Thu, 19 Nov 1981 08:52:00 GMT
Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0
Pragma: no-cache
Connection: close
Transfer-Encoding: chunked
Content-Type: text/html

0
#endif




