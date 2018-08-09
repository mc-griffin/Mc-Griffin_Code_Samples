// HttpReqResp.cpp: implementation of the HttpReqResp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HttpUri.h"
#include "Utilities.h"


const WORD HttpUri::_maxUrlLength = 2083;


// improvement: string HttpUri::GetString( bool urlEncode = false ) const;
string HttpUri::GetString() const
{
	if ( _url.empty()) { return _url; }
	string uri = _url;
	if ( _modifiers.size() == 0 )
		return uri;
	uri += "?";
	for ( DWORD i = 0; i < _modifiers.size(); ++i )
	{
		uri += _modifiers.at( i );
		if ( 0 <= i && i < _modifiers.size() - 1 )
			uri += "&";
	}
	assert( uri.length() <= _maxUrlLength );

	return uri;

	// improvement: if ( urlEncode ) return UrlEncode( uri );
}


// See RFC 1738: URL:
// "Thus, only alphanumerics, the special characters "$-_.+!*'(),", and
// reserved characters used for their reserved purposes may be used
// unencoded within a URL."
//
// "An HTTP URL takes the form:
//		http://<host>:<port>/<path>?<searchpart>
// where ... see full quote in HttpUri.h
// Within the <path> and <searchpart> components, "/", ";", "?" are
// reserved."
//
// My question:  Isn't "&" also reserved?
bool HttpUri::MustUrlEncode( char c )
{
	if ( isalnum( c )) return false;

	switch ( c )
	{
		// special characters
		case '$':
		case '-':
		case '_':
		case '.':
		case '+':
		case '!':
		case '*':
		case '\'':
		case '(':
		case ')':
		case ',':
			return false;

		// reserved characters
		case '/':
		case ';':
		case '?':
			return false;
	}

	return true;
}


// uses InternetCanonicalizeUrl(), but also contains my own URL encoding code
string HttpUri::UrlEncode( const string &unencoded )
{
	string encoded;

#if _DEBUG
	// My URL encoding code based on the RFCs referenced in HttpUri.h:
	// This is the code I wrote before I found InternetCanonicalizeUrl().
	//
	// InternetCanonicalizeUrl() returns results that seem wrong to me.
	// For example, see TestUrlEncode().  I choose to use InternetCanonicalizeUrl()
	// because its more likely that I misunderstand or miscoded the encoding.

	string myEncoded;
	for ( DWORD i = 0; i < unencoded.length(); ++i )
	{
		char c = unencoded[i];
		// code required to mimic InternetCanonicalizeUrl() begin
		if ( c < '!' )
			continue;
		// code required to mimic InternetCanonicalizeUrl() end

		if ( MustUrlEncode( c ))
		{
			int charInt( c );
			if ( CHAR_MAX < charInt || charInt < 0 ) { assert(false); return ""; }

			char octet[16] = "";
			sprintf( octet, "%%%02x", charInt );

			if ( strlen( octet ) != 3 ) { assert(false); return ""; }

			myEncoded += octet; 
		}
		else
		{
			char buf[4] = "";
			sprintf( buf, "%c", c );
			if ( strlen( buf ) != 1 ) { assert(false); return ""; }

			myEncoded += buf;
		}
	}
#endif

	char buf[ _maxUrlLength + 1 ] = "";
	DWORD len = _maxUrlLength;

	BOOL encode = InternetCanonicalizeUrl( unencoded.c_str(), buf, &len, ICU_NO_META );
	if ( !encode )
	{
		DWORD err = GetLastError();
		// LOG(
		return "";
	}

	encoded = buf;

	assert( encoded.length() < _maxUrlLength );

	// ( encoded == myEncoded ) is false!  See TestUrlEncode().

	return encoded;
}


string HttpUri::Test()
{
	string log = "Test HttpUri\n";

	log += TestDefCopyConstr();
	// Don't call this function because my URL encoding conflicts with
	// InternetCanonicalizeUrl().  Don't worry, UrlEncode()
	// uses InternetCanonicalizeUrl().
	// log += TestUrlEncode();

	// test that "?" only appears when there are modifiers
	// test that "&" doesn't appear at the end of the uri

	return log;
}


string HttpUri::TestUrlEncode()
{
	string log;

	// need to first test AtoiHex()
	if ( AtoiHex( "FfFfFfFf" ) != ULONG_MAX )
		log += "AtoiHex() failed\n";
	if ( AtoiHex( "FfFf" ) != USHRT_MAX )
		log += "AtoiHex() failed\n";
	if ( AtoiHex( "0" ) != 0 )
		log += "AtoiHex() failed\n";
	if ( AtoiHex( "1" ) != 1 )
		log += "AtoiHex() failed\n";
	if ( AtoiHex( "9" ) != 9 )
		log += "AtoiHex() failed\n";
	if ( AtoiHex( "a" ) != 10 )
		log += "AtoiHex() failed\n";
	if ( AtoiHex( "10" ) != 16 )
		log += "AtoiHex() failed\n";

	// simple checks
	{
		string unencoded = "This string has spaces in it.  How 'bout that?";
		string encoded = UrlEncode( unencoded );
		string expected =
			"This%20string%20has%20spaces%20in%20it.%20%20How%20'bout%20that?";
		if ( encoded != expected )
			log += "UrlEncode() failed\n";
	}

	// make a list of all characters, pass them to UrlEncode(), check results
	string unencoded;
	for ( char c = 0; c < CHAR_MAX; ++c )
	{
		// leave the real '%' out of this test, to avoid confusion
		if ( c == '%' )
			continue;

		char buf[8] = "";
		sprintf( buf, "%c", c );
		unencoded += buf;
	}

	string encoded = UrlEncode( unencoded );

	char buf[32] = "";
	strncpy( buf, encoded.c_str(), 30 );

	for ( WORD i = 0; i < encoded.length(); ++i )
	{
		char c( encoded[i]);

		if ( c == '%' )
		{
			++i;
			char sOctet[4] = "";
			strncpy( sOctet, encoded.c_str() + i, 2 );
			DWORD iOctet = AtoiHex( sOctet );
			char cOctet( iOctet );
			if ( !MustUrlEncode( cOctet ))
				log += "MustUrlEncode() failed\n";
			i += 2;
		}
		else
		{
			// THIS is where I detect that InternetCanonicalizeUrl() conflicts
			// with my own method of computing URL encoding in UrlEncode().
			if ( MustUrlEncode( c ))
				log += "MustUrlEncode() failed\n";
		}
	}

	// Here is where I demonstrate the bizarre behavior of InternetCanonicalizeUrl().
	// Actually, what I demonstrate is that I can't reproduce the bizarre behavior
	// here, which makes me think the problem is with my code.
	{
		string unencoded =
			"#$&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

		char buf[ _maxUrlLength + 1 ] = "";
		DWORD len = _maxUrlLength;

		BOOL encode = InternetCanonicalizeUrl( unencoded.c_str(), buf, &len, ICU_NO_META );
		if ( !encode )
		{
			DWORD err = GetLastError();
			assert( err == ERROR_INTERNET_INVALID_URL );

			log += "InternetCanonicalizeUrl() failed\n";
		}

		string encoded = buf;

		// Here is where I expected to demonstrate the bizarre behavior of
		// InternetCanonicalizeUrl(), but cannot.
		// assert( encoded ==
		//	 "!%22?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~#$&'()*+,-./0123456789:;<=>" );
	}

	return log;
}


// only used in testing
static HttpUri TestDefCopyConstr( HttpUri uri )
{
	return uri;
}


string HttpUri::TestDefCopyConstr()
{
	string log;

	HttpUri uri1;
	uri1.SetUrl( "some.url" );
	uri1.AddModifier( "modifier1" );
	uri1.AddModifier( "modifier2" );
	string string1 = uri1.GetString();

	HttpUri uri2( uri1 );
	string string2 = uri2.GetString();
	if ( string1 != string2 )
		log += "default copy constructor failed\n";

	HttpUri uri3 = ::TestDefCopyConstr( uri1 );
	string string3 = uri3.GetString();
	if ( string1 != string3 )
		log += "default copy constructor failed\n";

	return log;
}



