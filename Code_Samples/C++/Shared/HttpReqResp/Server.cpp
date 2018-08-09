// Server.cpp: implementation of the Server class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Server.h"


string Server::GetString() const
{
	if ( !IsSet()) { assert(false); return ""; }
	string s = _name;
	s += "(";
	s += _address;
	s += "):";
	s += GetPortStr();
	return s;
}


string Server::GetName() const
{
	assert( !_name.empty());
	return _name;
}


string Server::GetAddress() const
{
	assert( !_address.empty());
	return _address;
}


string Server::GetPortStr() const
{
	if ( _port < 1 ) { assert(false); return ""; }

	char port[8] = "";
	sprintf( port, "%d", _port );
	return port;
}


bool Server::SetNameAddress( LPCSTR name )
// throw WsaStartupException, Exception
{
	if ( name == NULL || *name == '\0' ) { assert(false); return false; }

	_name.erase();
	_address = GetAddressByName( name );

	if ( _address.empty()) { assert(false); return false; }

	_name = name;
	return true;
}


bool Server::SetNameAddress( const string &name )
{
	return SetNameAddress( name.c_str());
}


string Server::GetAddressByName( const string &name )
// throw WsaStartupException, Exception
{
	if ( name.empty()) { assert(false); return string(); }

	// in case winsock isn't up, start it (but shut it down before returning)
 	int rc;
    WSADATA wsdata;
	rc = WSAStartup( MAKEWORD(1,1), &wsdata );
	if ( rc != 0 )
		throw WsaStartupException(
				"WSAStartup(version 1.1) failed in Server::GetAddressByName()",
				"Winsock service is not working on your machine." );

	

	struct hostent *host = gethostbyname( name.c_str());
	if ( host == NULL )
	{
		rc = WSACleanup(); assert( rc == 0 );
		throw Exception(
				"gethostbyname() failed in Server::GetAddressByName(), 1",
				"Either you are not connected to the Internet or " 
				"your DNS service is not working." );
	}

#if 0
	// should we ever need to try the alias URLs
	string alts;
	char **p = host->h_aliases;
	while ( *p != NULL )
	{
		alts += *p;
		alts += " ";
		++p;
	}
#endif

	if ( host->h_addr_list == NULL )
	{
		rc = WSACleanup(); assert( rc == 0 );
		throw Exception(
				"gethostbyname() failed in Server::GetAddressByName(), 2",
				"Either you are not connected to the Internet or " 
				"your DNS service is not working." );
	}

	assert( host->h_length == sizeof(DWORD));

	struct in_addr netOrder = {0};
	memmove( &netOrder, host->h_addr_list[0], sizeof( netOrder ));
	char FAR *address = inet_ntoa( netOrder );

	rc = WSACleanup(); assert( rc == 0 );

	if ( address == 0 )
		throw Exception(
				"gethostbyname() failed in Server::GetAddressByName(), 3",
				"Either you are not connected to the Internet or " 
				"your DNS service is not working." );

	return string( address );
}


bool Server::IsSet() const
{
	return !_name.empty() && !_address.empty() && 0 < _port;
}


string Server::GetCookie() const
{
	if ( !IsSet()) { assert(false); return ""; }
	return GetCookie( _name );
}


string Server::GetCookie( const string &serverName )
{
	if ( serverName.empty()) { assert(false); return ""; }

	string cookieUrl = "http://";
	cookieUrl += serverName;

	unsigned long cookieValueLen = 0;

	// Since cookieValue is NULL initially, the first call to
	// InternetGetCookie will get the required buffer size needed to
	// download the cookie data.
	BOOL gotIt = InternetGetCookie( cookieUrl.c_str(), NULL, NULL, &cookieValueLen );
	if ( !gotIt || cookieValueLen == 0 )
	{
		assert(false);
		return "";
	}

	char *cookieValue = new char[ cookieValueLen + 1 ];

	gotIt = InternetGetCookie( cookieUrl.c_str(), NULL, cookieValue, &cookieValueLen );
	if ( !gotIt || cookieValue == NULL || cookieValueLen == 0 )
	{
		delete [] cookieValue;
		assert(false);
		return "";
	}

	string cookie = cookieValue;

	delete [] cookieValue;

	return cookie;
}



string Server::Test( bool testNetwork, bool testUi,
						const string &serverName, const string &serverAddress )
{
	string log;

	if ( !testNetwork )
	{
		log = "Test Server requires network connection\n";
		return log;
	}

	log = "Test Server\n";

	Server s;
		
	if ( !s.SetNameAddress( serverName.c_str()))
		log += "SetNameAddress() failed\n";

	if ( !s.IsSet())
		log += "IsSet() failed\n";

	if ( s.GetAddress() != serverAddress.c_str())
		log += "SetNameAddress() or GetAddress() failed\n";

	try
	{
		if ( s.SetNameAddress( "xxx" ))
			log += "SetNameAddress() failed";
	}
	catch ( const Exception &e )
	{
		if ( testUi )
			e.TellUser( "Server::Test()" );
	}
	catch (...)
	{
		log += "Unexpected exception thrown\n";
	}

	return log;
}



