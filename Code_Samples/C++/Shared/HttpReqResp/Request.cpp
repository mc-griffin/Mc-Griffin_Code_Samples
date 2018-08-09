// Request.cpp: implementation of the Request class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Request.h"


Request::Request( const Server *const server )
	: _server( server )
{
	assert( _server != NULL && _server->IsSet());
}


bool Request::SetUrl( const string &url )
{
	return _line.SetUrl( url );
}


string Request::GetHead() const
{
	// _body is not included
	// return _line.GetString() + "\r\n" + _headers.GetString(); seems to cause exception
	string s = _line.GetString();
	s += "\r\n";
	s += _headers.GetString();
	return s;
}


bool Request::IsSet() const
{
	return _server != NULL && _server->IsSet() && _line.IsSet() && 0 < _headers.Count();
}


string Request::GetUrl() const
{
	return _line.GetUrl();
}


string Request::GetMethod() const
{
	return _line.GetMethod();
}


int Request::SetMinimalHeaders()
{
	vector<NameValuePair> headers;

	// polymorphism
	int count1 = GetMinimalHeaders( &headers );
	if ( count1 < 1 ) { assert(false); return count1; }

	int count2 = _headers.Append( headers );
	assert( count1 == count2 );
	return count2;
}


bool Request::Generate( bool /*base64Encode=false*/)
{
	return true;
}


const char *const Request::GetBody() const
{
	return NULL;
}


DWORD Request::GetBodyLength() const
{
	return 0;
}

