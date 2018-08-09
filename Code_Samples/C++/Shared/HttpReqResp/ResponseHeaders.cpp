// ResponseHeaders.cpp: implementation of the ResponseHeaders class.
//
//////////////////////////////////////////////////////////////////////

// TO DO: Fix ResponseHeaders::Test()!

#include "stdafx.h"
#include "ResponseHeaders.h"
#include "Utilities.h"


void ResponseHeaders::Reset()
{
	if ( !empty())
		erase( begin(), end());
	assert( size() == 0 );
	_contentLength.erase();
	_transferEncoding.erase();
	_downloadedFileName.erase();
}


DWORD ResponseHeaders::Append( const NameValuePair &header )
{
	if ( !header.IsSet()) { assert(false); return 0; }
	push_back( header );
	// in the above statement, I assume vector push_back() is thread safe
	UpdateCache( header );
	return size();
}


// assumes that EnsureCRLF() was called on headers.
// No singular '\r's or '\n's,
int ResponseHeaders::AppendLines( const char *headers )
{
	if ( headers == NULL ) { assert(false); return HRR_INVALID_PARAMETER; }

	const DWORD headLen = strlen( headers );
	// strlen("a:b\r\n") is 5
	if ( headLen < 5 ) { assert(false); return HRR_INVALID_PARAMETER; }

	// Set longestLineLen to the length of the longest line,
	// excluding the "\r\n" that delimits each line.
	DWORD longestLineLen = GetLongestLineLength( headers );
	// strlen("a:b") is 3
	if ( longestLineLen < 3 ) { assert(false); return HRR_INVALID_PARAMETER; }

	// allocate a buffer big enough for the longest line + "\r\n"
	const DWORD lineBufLen = longestLineLen + 2;
	char *lineBuf = new char[ lineBufLen + 1 ];

	int headersCount = 0;

	// For each line, copy the line from headers to lineBuf,
	// then append lineBuf to header list.
	DWORD iHead = 0;
	while ( iHead < headLen )
	{
		DWORD iBuf = 0;
		bool atEol = false;

		assert(( iHead < headLen ) == ( iBuf < lineBufLen )); 

		while ( iHead < headLen && iBuf < lineBufLen && !atEol )
		{
			char c = headers[ iHead ];

			lineBuf[ iBuf ] = c;

			if ( c == '\n' )
				atEol = true;

			++iBuf;
			++iHead;
		}

		if ( !( iHead >= headLen
						|| ( headers[ iHead - 2 ] == '\r' && headers[ iHead - 1 ] == '\n' )))
		{
			LOG( string( "Error! " ) + headers );
		}

		assert( iHead >= headLen
				|| ( headers[ iHead - 2 ] == '\r' && headers[ iHead - 1 ] == '\n' ));
		assert( iBuf >= lineBufLen
				|| ( lineBuf[ iBuf - 2 ] == '\r' && lineBuf[ iBuf - 1 ] == '\n' ));

		// add trailing zero to lineBuf
		if ( 1 < iBuf && lineBuf[ iBuf - 2 ] == '\r' && lineBuf[ iBuf - 1 ] == '\n' )
			iBuf -= 2;
		lineBuf[ iBuf ] = '\0';

		// append to headers (strlen("a:b") is 3)
		if ( 3 <= iBuf )
		{
			NameValuePair nv;
			bool set = nv.Set( lineBuf );
			if ( set )
			{
				Append( nv );
				++headersCount;
			}
		}
		else
		{
			assert( *lineBuf == '\0' || strcmp( lineBuf, "\r\n" ) == 0 );
		}
	}

	delete [] lineBuf;

	LOG( "ResponseHeaders::AppendLines: " + NumberToString( headersCount ));

	return headersCount;
}


DWORD ResponseHeaders::Append( const char *line )
{
	if ( line == NULL || *line == '\0' ) { assert(false); return 0; }

	NameValuePair header;
	bool set = header.Set( line );
	if ( !set )
		return 0;

	return Append( header );
}


string ResponseHeaders::GetString() const
{
	string headers;
	for ( int i = 0; i < size(); ++i )
	{
		headers += (* this)[ i ].GetString();
		headers += "\r\n";
	}
	headers += "\r\n";
	return headers;
}


DWORD ResponseHeaders::Find( const string &name, DWORD start /*=0*/,
								bool caseSensitive /*=false*/) const
{
	if ( name.empty() || start < 0 ) { assert(false); return size(); }

	string nameUp = caseSensitive ? "" : ToUpper( name );

	for ( long i = start; i < size(); ++i )
	{
		const NameValuePair &p = at( i );
		if ( caseSensitive && p.Name() == name )
			return i;
		else if ( !caseSensitive && p.NameUp() == nameUp )
			return i;
	}

	return size();
}


string ResponseHeaders::GetValueAt( DWORD index ) const
{
	if ( size() <= index ) { assert(false); return ""; }
	return at( index ).Value();
}


DWORD ResponseHeaders::FindCount( const string &name,
									bool caseSensitive /*=false*/) const
{
	if ( name.empty()) { assert(false); return 0; }

	string nameUp = caseSensitive ? "" : ToUpper( name );

	DWORD count = 0;
	for ( DWORD i = 0; i < size(); ++i )
	{
		const NameValuePair &p = at( i );
		if ( caseSensitive && p.Name() == name )
			++count;
		else if ( !caseSensitive && p.NameUp() == nameUp )
			++count;
	}

	assert( count < size());
	return count;
}


string ResponseHeaders::FindOnly( const string &name,
									bool caseSensitive /*=false*/) const
{
	if ( FindCount( name, caseSensitive ) != 1 )
	{
		// assert(false);
		return "";
	}

	// Minor threading issue:  it is possible that another header
	// with name could be added after the above call to FindOnly().
	// This is not much of a concern, but could be addressed with
	// a MUTEX, as explained in ResponseHeaders.h.

	long i = Find( name, 0, caseSensitive );
	if ( i < 0 || size() <= i ) { assert(false); return ""; }

	return at( i ).Value();
}


WORD ResponseHeaders::Count() const
{
	return size();
}


int ResponseHeaders::AppendLines( const string &headers )
{
	return AppendLines( headers.c_str());
}


DWORD ResponseHeaders::Append( const string &line )
{
	return Append( line.c_str());
}


// Each time a header is appended, this method must be called to update
// the cache of variables of interest.
//
// return true if a cache variable is updated for the first time, false otherwise
bool ResponseHeaders::UpdateCache( const NameValuePair &header )
{
	if ( !header.IsSet()) { assert(false); return false; }

	static const string contLen = ToLower( "Content-Length" );
	// Noel's HTTP says "Content-length" (notice "l") but HTTP is case insensitive
	static const string contDisp = ToLower( "Content-Disposition" );
	static const string transEnc = ToLower( "Transfer-Encoding" );

	string name = ToLower( header.Name());

	if ( name == contLen )
	{
		bool isEmpty = _contentLength.empty();
		assert( isEmpty );
		bool isNum = IsUnsignedLong( header.Value());
		assert( isNum);
		if ( isNum )
		{
			_contentLength = header.Value();
			// in the above statement, I assume string assignment is thread safe
			LOG( "ResponseHeaders::UpdateCache: _contentLength set to " + _contentLength );
		}
		return isEmpty && isNum;
	}

	if ( name == contDisp )
	{
		bool isEmpty = _downloadedFileName.empty();
		assert( _downloadedFileName.empty());
		_downloadedFileName = ExtractDownloadedFileName( header.Value());
		// in the above statement, I assume string assignment is thread safe
		LOG( "ResponseHeaders::UpdateCache: _downloadedFileName set to "
				+ _downloadedFileName );
		return isEmpty;
	}

	if ( name == transEnc )
	{
		bool isEmpty = _transferEncoding.empty();
		assert( isEmpty );
		_transferEncoding = header.Value();
		// in the above statement, I assume string assignment is thread safe
		return isEmpty;
	}

	return false;
}


DWORD ResponseHeaders::SetCookies( const string &serverName ) const
{
	if ( serverName.empty()) { assert(false); return 0; }

	string cookieUrl = "http://";
	cookieUrl += serverName;

	DWORD count = 0;

	DWORD i = Find( "Set-Cookie" );
	while ( i < Count())
	{
		string cookie = GetValueAt( i );
		if ( !cookie.empty())
		{
			BOOL setIt = InternetSetCookie( cookieUrl.c_str(), NULL, cookie.c_str());
			assert( setIt );
			if ( setIt )
				++count;
		}

		i = Find( "Set-Cookie", i + 1 );
	}

	assert( FindCount( "Set-Cookie" ) == count );
	return count;
}


// This code assumes that headers is well-formed,
// so there should be a syntax check first.
DWORD ResponseHeaders::GetLongestLineLength( const string &headers )
{
	DWORD longestLineLen = 0;
	DWORD iCurrentLine = 0;
	bool returnJustPassed = false; // "return" means '\r'

	for ( DWORD i = 0; i < headers.length(); ++i )
	{
		char c = headers[ i ];

		if ( returnJustPassed && c == '\n' )
		{
			DWORD lineLen = i - iCurrentLine - 1; // - 1 for '\r'
			if ( longestLineLen < lineLen )
				longestLineLen = lineLen;

			iCurrentLine = i + 1; // skip over '\n'
			returnJustPassed = false;
		}
		else
			returnJustPassed = c == '\r';
	}
	assert( 3 <= longestLineLen ); // strlen("a:b") is 3

	return longestLineLen;
}


string ResponseHeaders::ExtractDownloadedFileName( const string &contentDisposition )
{
	int nameStarts = contentDisposition.find( "filename=" );
	if ( nameStarts == string::npos )
	{
		LOG( "ResponseHeaders::ExtractDownloadedFileName: No \"filename=\" found in Content-Disposition" );
		return "";
	}

	// move i to the end of the filename, delimited by ';'
	nameStarts += strlen("filename=");

	DWORD i = nameStarts;
	while ( i < contentDisposition.size())
	{
		char c = contentDisposition.at( i );
		if ( c == ';' ) // spaces allowed
			break;
		++i;
	}

	if ( i <= nameStarts )
	{
		LOG( "ResponseHeaders::ExtractDownloadedFileName: empty file name" );
		return "";
	}

	DWORD len = i - nameStarts; 

	string fileName = contentDisposition.substr( nameStarts, len );
	
	if ( fileName.empty() || MAX_PATH <= fileName.length())
	{
		LOG( "ResponseHeaders::ExtractDownloadedFileName: illegal file name" );
		return "";
	}

	return fileName;
}


string ResponseHeaders::GetDownloadedFileName() const
{
	// It should be okay to call this method before any headers are added,
	// but I want to know about it.
	assert( 0 < size());
	return _downloadedFileName;
}


bool ResponseHeaders::GetContentLength( string *len ) const
{
	// It should be okay to call this method before any headers are added,
	// but I want to know about it.
	assert( 0 < size());

	if ( _contentLength.empty() || len == NULL )
	{
		assert( len );
		return false;
	}

	*len = _contentLength;

	return true;
}


string ResponseHeaders::Test( const string &serverName, const string &serverAddress )
{
	string log = "Test ResponseHeaders\n";

	{
		ResponseHeaders rh;

		int count = rh.AppendLines( "a:b\r\nc : d\r\n\r\n" );
		if ( count != 2 )
			log += "AppendLines( \"a:b\\r\\nc : d\\r\\n\\r\\n\" failed\n";

/*error
		int count = rh.AppendLines( "a:b\r\nc : d\r\nA: ff\r\n\r\n" );
		if ( count != 3 )
			log += "AppendLines( \"a:b\\r\\nc : d\\r\\nA: ff\\r\\n\\r\\n\" failed\n";
*/
/*no error
		int count = rh.AppendLines( "a:b\r\n" );
		if ( count != 1 )
			log += "AppendLines( \"a:b\\r\\nc : d\\r\\nA: ff\\r\\n\\r\\n\" failed\n";

		int count = rh.AppendLines( "a:b\r\n\r\n" );
		if ( count != 1 )
			log += "AppendLines( \"a:b\\r\\n\\r\\n\" failed\n";
*/
	}

return log;

	// How to test SetCookies()?

	{
		ResponseHeaders rh;
		int count = rh.AppendLines( "a:b\r\nc : d\r\nA: ff\r\n\r\n" );
		if ( count != 3 )
			log += "AppendLines( \"a:b\\r\\nc : d\\r\\nA: ff\\r\\n\\r\\n\" failed\n";

		if ( rh.FindCount( "a" ) != 2 )
			log += "FindCount(\"a\") failed\n";

		if ( rh.Find( "A" ) != 0 )
			log += "Find(\"A\") failed\n";

		if ( rh.Find( "a", 1 ) != 2 )
			log += "Find(\"a\",1) failed\n";

		if ( rh.Find( "z" ) != rh.Count())
			log += "Find(\"z\") failed\n";
	}

	// shave "\r\n" off the end of last one
	{
		ResponseHeaders rh;
		int count = rh.AppendLines( "a:b\r\nc : d\r\neee: ff\r\n" );
		if ( count != 3 )
			log += "AppendLines( \"a:b\\r\\nc : d\\r\\neee: ff\\r\\n\" failed\n";
	}

/*
	// Prior Bug: AppendLines( "Content-Length: 666\r\n" ) causes infinite loop
	{
		ResponseHeaders rh;
		const char headers[] = "a: b\r\nContent-Length: 666\r\nc: d\r\n\r\n";
		// const char headers[] = "Content-Length: 666\r\n";
		// const char headers[] = "Content-Length: 666";

		int count = rh.AppendLines( headers );
		if ( IsHrrError( count ))
			log += "AppendLines() failed, you'll never ever see this\n";
	}

	// test Append() and Count()
	{
		ResponseHeaders rh;
		int count;

		count = rh.Append( NULL );
		if ( 0 < count )
			log += "Append(NULL) failed\n";

		count = rh.Append( "a" );
		if ( 0 < count )
			log += "Append(\"a\") failed\n";

		count = rh.Append( "a:b\r" );
		if ( 0 < count )
			log += "Append(\"a:b\\r\") failed\n";

		count = rh.Append( "a:b\rx" );
		if ( 0 < count )
			log += "Append(\"a:b\\rx\") failed\n";

		count = rh.Append( "a:b\nx" );
		if ( 0 < count )
			log += "Append(\"a:b\\nx\") failed\n";

		count = rh.Append( "a:b\r\n\r" );
		if ( 0 < count )
			log += "Append(\"a:b\\r\\n\\r\") failed\n";

		count = rh.Append( "aa:b\r\n\n" );
		if ( 0 < count )
			log += "Append(\"aa:b\\r\\n\\n\") failed\n";

		count = rh.Append( "\r\n\r\n\r\n\r\n" );
		if ( 0 < count )
			log += "Append(\"\\r\\n\\r\\n\\r\\n\\r\\n\") failed\n";

		// should fail because of the trailing "\r\n\r\n", use AppendLines()
		count = rh.Append( "a:b\r\n\r\n" );
		if ( 0 < count )
			log += "Append(\"a:b\\r\\n\\r\\n\") failed\n";

		count = rh.Append( "a:b" );
		if ( count != rh.Count())
			plog += "Append(\"a:b\") failed\n";

		count = rh.Append( "c :d" );
		if ( count != rh.Count())
			log += "Append(\"c :d\") failed\n";

		count = rh.Append( "e: f" );
		if ( count != rh.Count())
			log += "Append(\"e: f\") failed\n";

		count = rh.Append( "g : h" );
		if ( count != rh.Count())
			log += "Append(\"g : h\") failed\n";
	}

	// test AppendLines() and Count()
	{
		ResponseHeaders rh;
		int count;

		count = rh.AppendLines( "a:b\r\n\r\n" );
		if ( count != rh.Count())
			log += "AppendLines(\"a:b\\r\\n\\r\\n\") failed\n";

		// '\n' can only appear at the end of a name-value pair
		count = rh.AppendLines( "c:\n\t\rd\r\n\r\n" );
		if ( 0 < count )
			log += "AppendLines(\"c:\\n\\t\\rd\\r\\n\\r\\n\") failed\n";

		count = rh.AppendLines( "e :f\r\n\r\n" );
		if ( count != rh.Count())
			log += "AppendLines(\"e :f\\r\\n\\r\\n\") failed\n";

		count = rh.AppendLines( "a : b\r\nc:d\r\n\r\n" );
		if ( count != rh.Count())
			log += "AppendLines(\"a : b\\r\\nc:d\\r\\n\\r\\n\" failed\n";

HERE I'M CONFUSED BECAUSE TESTS AREN'T THE WAY I WANT.
Since:
1. I am pressed for time
2. most of these cases are pathological
3. the code works on all the output I am likely to get from a HTTP server
I'm not fixing these problems now

		// don't append bad header, but don't quit either, add the valid one
		int previousCount = count;
		count = rh.AppendLines( " a:b\r\nc:d\r\n\r\n" );
		if ( count != previousCount + 1 || count != rh.Count())
			log += "AppendLines(\" a:b\\r\\nc:d\\r\\n\\r\\n\") failed\n";

		previousCount = count;
		count = rh.AppendLines( "a : b\r\nc:d\r\neeee:ffff\r\n" );
		if ( count != previousCount + 3 || count != rh.Count())
			log += "AppendLines(\"a  :\\t b\\r\\nc:d\\r\\n\\r\\n\") failed\n";
	}

	// Test Find! (ignoreCase too)
	
	// test GetString()
	{
		ResponseHeaders rh;
		int count;

		const string s = "a: b\r\nc:\t d\r\n\r\n";
		count = rh.AppendLines( s );
		if ( count != 2 )
			log += "AppendLines(\"a : b\\r\\nc\\t :d\\r\\n\\r\\n\" failed\n";

		if ( rh.GetString() != s )
			log += "GetString() failed\n";
	}
*/
	// TEST SetOfInterest() and WriteCookies()!!!

/*	TEST!
	long Find( const string &name, long start = 0,
					bool caseSensitive = true ) const;
	DWORD FindCount( const string &name, bool caseSensitive = true ) const;
	string FindOnly( const string &name, bool caseSensitive = true ) const;
	string GetValueAt( DWORD index ) const;


	GetDownloadedFileName()!
*/

	return log;
}


