// HeadRequest.cpp: implementation of the HeadRequest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HeadRequest.h"


HeadRequest::HeadRequest( const Server *const server )
	: Request( server )
{
	_line.SetMethod( RequestLine::s_head );
}


int HeadRequest::GetMinimalHeaders( vector< NameValuePair > *headers ) const
{
	if ( headers == NULL ) { assert(false); return HRR_INVALID_PARAMETER; }
	if ( _server != NULL && !_server->IsSet())
		{ assert(false); return HRR_INVALID_STATE; }

	string server = _server->GetName();
	string port = _server->GetPortStr();

	string serverPort = server + ":" + port;

	NameValuePair host;
	bool set = host.Set( "Host", serverPort ); assert( set );

	headers->push_back( host );

	return headers->size();
}


string HeadRequest::Test( bool testNetwork,
					const string &serverName, const string &serverAddress )
{
	string log = "Test HeadRequest\n";
/*
	// GetMinimalHeaders() with an invalid server and NULL input
	{
		Server s; // don't initialize, make GetMinimalHeader() fail
		HeadRequest g( s );

		if ( g.IsSet())
			log += "HeadRequest() or IsSet() failed\n";

		int count = g.GetMinimalHeaders( NULL );
		if ( count != HRR_INVALID_PARAMETER )
			log += "GetMinimalHeaders() failed\n";

		vector< NameValuePair > h;
		count = g.GetMinimalHeaders( &h ); // fail due to empty server
		if ( count != HRR_INVALID_STATE )
			log += "GetMinimalHeaders() failed\n";

		if ( g.IsSet())
			log += "IsSet() failed\n";
	}
*/
	// GetMinimalHeaders() with a (faked) valid server
	{
		// fake a server so that a network connection isn't necessary
		struct FakeServer { string name; string add; WORD port; } fs;
		fs.name = serverName;
		fs.add = serverAddress;
		fs.port = 80;
		
		HeadRequest g((Server *) &fs );

		g.SetUrl( "sims" );
		
		NameValuePair p1, p2;
		p1.Set( "a : b" );
		g._headers.Append( p1 );
		p2.Set( "c", "d" );
		g._headers.Append( p2 );

		if ( !g.IsSet())
			log += "IsSet() failed.\n";

		string req = "HEAD sims HTTP/1.1\r\n";
		req += "a: b\r\nc: d\r\n\r\n";

		if ( g.GetHead() != req )
			log += "GetHead() failed\n";

		vector< NameValuePair > h;
		int count = g.GetMinimalHeaders( &h );
		if ( count != 1 )
			log += "GetMinimalHeaders() failed\n";

		string min = "Host: ";
		min += serverName + ":" + "80";
		if ( h[0].GetString() != min )
			log += "GetMinimalHeaders() failed\n";

		g._headers.Append( h );

		req.resize( req.length() - 2 );
		req += min + "\r\n\r\n";

		if ( g.GetHead() != req )
			log += "GetHead() or Append() failed";
	}

	// Test SetMinimalHeaders()
	{
		// fake a server so that a network connection isn't necessary
		struct FakeServer { string name; string add; WORD port; } fs;
		fs.name = "Server";
		fs.add = "123456789";
		fs.port = 80;
		
		HeadRequest g((Server *) &fs );

		g.SetUrl( "sims" );
		
		if ( g.SetMinimalHeaders() != 1 )
			log += "SetMinimalHeaders() failed\n";

		string host = "Host: ";
		host += fs.name + ":" + "80\r\n\r\n";
		if ( g._headers.GetString() != host )
			log += "GetString() failed\n";

		// search should be case insensitive
		list<string> matches;
		DWORD found = g._headers.Find( "hOST", &matches );
		if ( found == -1 || matches.size() != 1 || matches.begin()->empty())
			log += "IncludeCookie() or Find() failed\n";
	}
	
	if ( testNetwork )
	{
		Server s;
		if ( s.SetNameAddress( serverName.c_str()) == false )
			log += "s.SetNameAddress() failed.\n";

		if ( s.GetAddress() != serverAddress )
			log += "SetNameAddress() or GetAddress() failed.\n";
		
		HeadRequest g( &s );

		string url = "/teleport/hotdate/dl_manager.php"
						"?asset_id=46&asset_type=NPC&action=getIFF"
							"&login_key=3ba24b49f9a29a735c35e3bd614f74aa";

		g.SetUrl( url );

		NameValuePair p1, p2;
		p1.Set( "a : b" );
		g._headers.Append( p1 );
		p2.Set( "c", "d" );
		g._headers.Append( p2 );

		if ( !g.IsSet())
			log += "IsSet() failed.\n";

		string req = "HEAD " + url + " HTTP/1.1\r\n";
		req += "a: b\r\nc: d\r\n\r\n";

		if ( g.GetHead() != req )
			log += "GetHead() failed";

		vector< NameValuePair > h;
		int count = g.GetMinimalHeaders( &h );
		if ( count != 1 )
			log += "GetMinimalHeaders() failed\n";

		string min = "Host: ";
		min += serverName + ":" + "80";
		if ( h[0].GetString() != min )
			log += "GetMinimalHeaders() failed\n";

		g._headers.Append( h );

		req.resize( req.length() - 2 );
		req += min + "\r\n\r\n";

		if ( g.GetHead() != req )
			log += "GetHead() or Append() failed";
	}
	
	return log;
}

