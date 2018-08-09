// RequestLine.cpp: implementation of the RequestLine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RequestLine.h"


const char RequestLine::s_get[] = "GET";
const char RequestLine::s_put[] = "PUT";
const char RequestLine::s_post[] = "POST";
const char RequestLine::s_head[] = "HEAD";
const char RequestLine::s_http1_1[] = "HTTP/1.1";


string RequestLine::GetString() const
{
	if ( _method.empty() || _url.empty()) { assert(false); return string(); }

	return _method + " " + _url + " " + s_http1_1; // no trailing "\r\n"
}


bool RequestLine::SetUrl( const string &u )
{
	if ( u.empty()) { assert(false); return false; }

	_url = u;
	return true;
}


bool RequestLine::SetMethod( const string &m )
{
	if ( m.compare( s_get ) == 0 )
	{
		_method = s_get;
		return true;
	}
	else if ( m.compare( s_put ) == 0 )
	{
		_method = s_put;
		return true;
	}
	else if ( m.compare( s_post ) == 0 )
	{
		_method = s_post;
		return true;
	}
	else if ( m.compare( s_head ) == 0 )
	{
		_method = s_head;
		return true;
	}

	assert(false);
	return false;
}


bool RequestLine::IsSet() const
{
	return !_method.empty() && !_url.empty();
}


string RequestLine::Test()
{
	string log = "Test RequestLine\n";

	RequestLine get;
	get.SetMethod( RequestLine::s_get );
	get.SetUrl( "xxx" );

	if ( !get.IsSet())
		log += "IsSet() failed\n";

	if ( get.GetString().compare( "GET xxx HTTP/1.1" ) != 0 )
		log += "RequestLine failed\n";

	return log;
}



