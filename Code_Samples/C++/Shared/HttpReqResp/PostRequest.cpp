// PostRequest.cpp: implementation of the PostRequest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PostRequest.h"


PostRequest::PostRequest( const Server *const server, string boundary,
							bool useOldBody /*=false*/)
	: Request( server )
	, _boundary( boundary )
	, _body( _boundary, useOldBody )
{
	_line.SetMethod( RequestLine::s_post );
	assert( !boundary.empty());
}


int PostRequest::GetMinimalHeaders( vector< NameValuePair > *headers ) const
{
	if ( headers == NULL ) { assert(false); return HRR_INVALID_PARAMETER; }
	if ( _server == NULL || !_server->IsSet() || _boundary.empty())
	{
		assert(false);
		return HRR_INVALID_STATE;
	}

	string server = _server->GetName();
	string port = _server->GetPortStr();

	string serverPort = server + ":" + port;
	NameValuePair host;
	bool set = host.Set( "Host", serverPort ); assert( set );
	headers->push_back( host );

	string typeValue = "multipart/form-data; boundary=";
	typeValue += _boundary;

	// "Content-Type: multipart/form-data; boundary=------------GDM"
	NameValuePair type;
	set = type.Set( "Content-Type", typeValue.c_str()); assert( set );
	headers->push_back( type );

	char lengthValue[ 16 ] = "";
	sprintf( lengthValue, "%d", GetBodyLength());
	NameValuePair length;
	set = length.Set( "Content-Length", lengthValue ); assert( set );
	headers->push_back( length );

	return headers->size();
}


bool PostRequest::IsSet() const
{
	if ( _boundary.empty())
	{
		assert(false);
		return false;
	}

	if ( !Request::IsSet())
		return false;

	if ( _headers.Count() < 3 )
		return false;

	// I think that POSTs with empty bodies are legal and occassionally useful

	return true;
}


bool PostRequest::Generate( bool base64Encode /*=false*/)
{
	return _body.Generate( base64Encode );
}


const char *const PostRequest::GetBody() const
{
	return _body.GetBody();
}


DWORD PostRequest::GetBodyLength() const
{
	return _body.GetBodyLength();
}


string PostRequest::Test( bool testNetwork,
					const string &serverName, const string &serverAddress )
{
	if ( !testNetwork )
		return "Test PostRequest requires a network connection\n";

	string log = "Test PostRequest\n";

	// Test SetMinimalHeaders()
	{
		// fake a server so that a network connection isn't necessary
		struct FakeServer { string name; string add; WORD port; } fs;
		fs.name = "Server";
		fs.add = "123456789";
		fs.port = 80;

		string boundary = "X-X-X";
		
		PostRequest p((Server *) &fs, boundary );

		p.SetUrl( "sims" );

//		if ( !p._body.AppendActionUpload())
//			log += "ResponseBody::AppendActionUpload() failed\n";

		if ( p.SetMinimalHeaders() != 3 )
			log += "SetMinimalHeaders() failed\n";

		if ( !p.IsSet())
			log += "IsSet() or SetMinimalHeaders() failed\n";
		
		string headers = "Host: ";
		headers += fs.name + ":" + "80\r\n";
		headers += "Content-Type: multipart/form-data; boundary=";
		headers += boundary + "\r\n";
		headers += "Content-Length: ";
		{
			char buf[16];
			sprintf( buf, "%d\r\n\r\n", p.GetBodyLength());
			headers += buf;
		}

		if ( p._headers.GetString() != headers )
			log += "SetMinimalHeaders() failed\n";
	}

	// GetMinimalHeaders() with an invalid server and NULL input
	{
		Server s; // don't initialize, make GetMinimalHeader() fail
		PostRequest p( &s, "boundary" );

		int count = p.GetMinimalHeaders( NULL );
		if ( count != HRR_INVALID_PARAMETER )
			log += "GetMinimalHeaders() failed\n";
		
		vector< NameValuePair > h;
		count = p.GetMinimalHeaders( &h ); // fail due to empty server
		if ( count != HRR_INVALID_STATE )
			log += "GetMinimalHeaders() failed\n";

		if ( p.IsSet())
			log += "IsSet() failed\n";
	}

	{
		const string boundary = "------T-E-S-T--B-O-U-N-D-A-R-Y-";
		const string endingBoundary = string("--") + boundary + "--";
		const string bodyEnding = string("\r\n") + endingBoundary;

		Server s;
		if ( !s.SetNameAddress( serverName.c_str()) || !s.IsSet())
			log += "s.SetNameAddress() failed.\n";

		PostRequest p( &s, boundary );

		p.SetUrl( "sims" );
		
		NameValuePair p1, p2;
		p1.Set( "a : b" );
		p._headers.Append( p1 );
		p2.Set( "c", "d" );
		p._headers.Append( p2 );

		string req = "POST sims HTTP/1.1\r\n";
		req += "a: b\r\nc: d\r\n\r\n";

		if ( p.GetHead() != req )
			log += "GetHead() failed\n";

		vector< NameValuePair > h;
		int count = p.GetMinimalHeaders( &h );
		if ( count != 3 )
			log += "GetMinimalHeaders() failed\n";

		string min = "Host: ";
		min += serverName + ":" + "80";
		if ( h[0].GetString() != min )
			log += "GetMinimalHeaders() failed\n";
		min += "\r\n";

		min += "Content-Type: multipart/form-data; boundary=";
		min += boundary + "\r\n";

		min += "Content-Length: ";
		{
			char buf[16];
			sprintf( buf, "%d\r\n\r\n", p.GetBodyLength());
			min += buf;
		}

		p._headers.Append( h );

		req.resize( req.length() - 2 );
		req += min;

		if ( p.GetHead() != req )
			log += "GetHead() or Append() failed\n";

		list<string> matches;
		DWORD found = p._headers.Find( "Content-Length", &matches, true );
		if ( found == -1 || matches.size() != 1 || matches.begin()->empty())
			log += "Find() failed\n";
	}

	return log;
}


#if 0
string PostRequest::Test( bool testNetwork,
					const string &serverName, const string &serverAddress )
{
	if ( !testNetwork )
		return "Test PostRequest requires a network connection\n";

	string log = "Test PostRequest\n";

	// Test SetMinimalHeaders()
	{
		// fake a server so that a network connection isn't necessary
		struct FakeServer { string name; string add; WORD port; } fs;
		fs.name = "Server";
		fs.add = "123456789";
		fs.port = 80;

		string boundary = "X-X-X";
		
		PostRequest p((Server *) &fs, boundary, true );

		p.SetUrl( "sims" );

		if ( !p._body.AppendActionUpload())
			log += "ResponseBody::AppendActionUpload() failed\n";

		if ( p.SetMinimalHeaders() != 3 )
			log += "SetMinimalHeaders() failed\n";

		if ( !p.IsSet())
			log += "IsSet() or SetMinimalHeaders() failed\n";
		
		string headers = "Host: ";
		headers += fs.name + ":" + "80\r\n";
		headers += "Content-Type: multipart/form-data; boundary=";
		headers += boundary + "\r\n";
		headers += "Content-Length: ";
		{
			char buf[16];
			sprintf( buf, "%d\r\n\r\n", p._body.GetString().length());
			headers += buf;
		}

		if ( p._headers.GetString() != headers )
			log += "SetMinimalHeaders() failed\n";
	}

	// GetMinimalHeaders() with an invalid server and NULL input
	{
		Server s; // don't initialize, make GetMinimalHeader() fail
		PostRequest p( &s, "boundary", false );

		int count = p.GetMinimalHeaders( NULL );
		if ( count != HRR_INVALID_PARAMETER )
			log += "GetMinimalHeaders() failed\n";
		
		vector< NameValuePair > h;
		count = p.GetMinimalHeaders( &h ); // fail due to empty server
		if ( count != HRR_INVALID_STATE )
			log += "GetMinimalHeaders() failed\n";

		if ( p.IsSet())
			log += "IsSet() failed\n";
	}

	{
		const string boundary = "------T-E-S-T--B-O-U-N-D-A-R-Y-";
		const string endingBoundary = string("--") + boundary + "--";
		const string bodyEnding = string("\r\n") + endingBoundary;

		Server s;
		if ( !s.SetNameAddress( serverName.c_str()) || !s.IsSet())
			log += "s.SetNameAddress() failed.\n";

		PostRequest p( &s, boundary, false );

		p.SetUrl( "sims" );
		
		NameValuePair p1, p2;
		p1.Set( "a : b" );
		p._headers.Append( p1 );
		p2.Set( "c", "d" );
		p._headers.Append( p2 );

		string req = "POST sims HTTP/1.1\r\n";
		req += "a: b\r\nc: d\r\n\r\n";

		if ( p.GetHead() != req )
			log += "GetHead() failed\n";

		vector< NameValuePair > h;
		int count = p.GetMinimalHeaders( &h );
		if ( count != 3 )
			log += "GetMinimalHeaders() failed\n";

		string min = "Host: ";
		min += serverName + ":" + "80";
		if ( h[0].GetString() != min )
			log += "GetMinimalHeaders() failed\n";
		min += "\r\n";

		min += "Content-Type: multipart/form-data; boundary=";
		min += boundary + "\r\n";

		min += "Content-Length: ";
		{
			char buf[16];
			sprintf( buf, "%d\r\n\r\n", p._body.GetString().length());
			min += buf;
		}

		p._headers.Append( h );

		req.resize( req.length() - 2 );
		req += min;

		if ( p.GetHead() != req )
			log += "GetHead() or Append() failed\n";

		list<string> matches;
		DWORD found = p._headers.Find( "Content-Length", &matches, true );
		if ( found == -1 || matches.size() != 1 || matches.begin()->empty())
			log += "Find() failed\n";
	}

	return log;
}
#endif // 0


