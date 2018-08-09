// RequestHeaders.cpp: implementation of the RequestHeaders class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RequestHeaders.h"
#include "Utilities.h"


string RequestHeaders::GetString() const
{
	// HTTP/1.1 requires the Host header
	assert( 0 < size());
	string s;
	const_iterator it = begin();
	while ( it != end())
	{
		s += (*it).GetString();
		s += "\r\n";
		++it;
	}
	// "\r\n" that delimits header and body
	s += "\r\n";
	assert( s.rfind( "\r\n\r\n" ) == s.size() - 4 );
	return s;
}


bool RequestHeaders::Append( const NameValuePair &p )
{
	if ( !p.IsSet()) { assert(false); return false; }
	push_back( p );
	return true;
}


DWORD RequestHeaders::Append( const vector< NameValuePair > &headerLines )
{
	DWORD count = 0;
	for ( DWORD i = 0; i < headerLines.size(); ++i )
	{
		bool append = Append( headerLines[ i ]);
		if ( append )
			++count;
	}
	return count;
}


// return value of -1 is an invalid input error
DWORD RequestHeaders::Find( const string &name, list<string> *matches,
								bool caseSensitive /*=false*/) const
{
	if ( name.empty() || matches == NULL ) { assert(false); return -1; }
	assert( matches->size() == 0 );

	string nameUp = caseSensitive ? "" : ToUpper( name );

	list< NameValuePair >::const_iterator it = begin();
	while ( it != end())
	{
		if ( caseSensitive && it->Name() == name )
			matches->push_back( it->Value());
		else if ( !caseSensitive && it->NameUp() == nameUp )
			matches->push_back( it->Value());

		++it;
	}

	return matches->size();
}


WORD RequestHeaders::Count() const
{
	return size();
}


bool RequestHeaders::AppendCookie( const string &serverName )
{
	if ( serverName.empty()) { assert(false); return false; }

	string cookieUrl = "http://";
	cookieUrl += serverName;

	unsigned long cookieValueLen = 0;

	// Since cookieValue is NULL initially, the first call to
	// InternetGetCookie will get the required buffer size needed to
	// download the cookie data.
	BOOL gotIt = InternetGetCookie( cookieUrl.c_str(), NULL, NULL, &cookieValueLen );
	if ( !gotIt || cookieValueLen == 0 )
	{
		assert( cookieValueLen == 0 );
		return false;
	}

	char *cookieValue = new char[ cookieValueLen + 1 ];

	gotIt = InternetGetCookie( cookieUrl.c_str(), NULL, cookieValue, &cookieValueLen );
	if ( !gotIt || cookieValue == NULL || cookieValueLen == 0 )
	{
		assert(false);
		return false;
	}

	string cookie = cookieValue;

	delete [] cookieValue;

	NameValuePair cookieHeader;
	bool set = cookieHeader.Set( "Cookie", cookie );
	if ( !set )
		{ assert(false); return false; }

	return Append( cookieHeader );
}


string RequestHeaders::Test()
{
	string log = "Test RequestHeaders\n";

	RequestHeaders rh;
	NameValuePair p1, p2;
	
	if ( rh.Find( "X", NULL ) != -1 )
		log += "Find() failed\n";

	p1.Set( "a", "b" );
	bool append = rh.Append( p1 );
	if ( !append || rh.GetString() != "a: b\r\n\r\n" ) 
		log += "Append() or GetString() failed\n";

	p2.Set( "c", "d" );
	append = rh.Append( p2 );
	if ( !append || rh.GetString() != "a: b\r\nc: d\r\n\r\n" ) 
		log += "Append() or GetString() failed\n";

	NameValuePair p3, p4;

	p3.Set( "w:x" );
	p4.Set( "y : z" );
	vector< NameValuePair > v;
	v.push_back( p3 );
	v.push_back( p4 );

	DWORD count = rh.Append( v );
	if ( count != 2 || rh.GetString() != "a: b\r\nc: d\r\nw: x\r\ny: z\r\n\r\n" )
		log += "Append() or GetString() failed\n";

	// C:\WINDOWS\"Temporary Internet Files" has the non-Netscape cookies
	// Cookie:default@dellnet.com/

	bool appendCookie = rh.AppendCookie( "thesims.ea.com" );
	if ( !appendCookie )
		log += "AppendCookie() failed or no thesims.ea.com cookie on this machine\n";

	list<string> matches;

	count = rh.Find( "a", &matches );
	if ( matches.size() != 1 || *matches.begin() != "b" )
		log += "Find() failed\n";
	matches.clear();

	count = rh.Find( "a", &matches );
	if ( matches.size() != 1 || *matches.begin() != "b" )
		log += "Find() failed\n";
	matches.clear();

	count = rh.Find( "c", &matches );
	if ( matches.size() != 1 || *matches.begin() != "d" )
		log += "Find() failed\n";
	matches.clear();

	count = rh.Find( "y", &matches );
	if ( matches.size() != 1 || *matches.begin() != "z" )
		log += "Find() failed\n";
	matches.clear();

	count = rh.Find( "Cookie", &matches );
	if ( matches.size() != 1 || matches.begin()->empty())
		log +="Find() failed or no thesims.ea.com cookie on this machine\n";
	matches.clear();

	return log;
}

