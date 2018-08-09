// Server.h: interface for the Server class.
//
// This class requires wsock32.lib.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVER_H__6F641AB2_FB7D_438B_85A0_B0DB6C752669__INCLUDED_)
#define AFX_SERVER_H__6F641AB2_FB7D_438B_85A0_B0DB6C752669__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Exception.h"
#include <wininet.h> // for INTERNET_PORT


class Server  
{
public:
	Server() : _port( 80 ) {}

	bool SetNameAddress( LPCSTR name ); // throw WsaStartupException, Exception

	string GetString() const;
	string GetName() const;
	string GetAddress() const;
	string GetPortStr() const;
	INTERNET_PORT GetPort() const { return _port; }

	static string GetAddressByName( const string &serverName );
	// throw WsaStartupException, Exception

	string GetCookie() const;
	static string GetCookie( const string &serverName );
	// could add SetCookie( serverName, data ) for completeness

	bool IsSet() const;

protected:
	string _name;
	string _address;
	INTERNET_PORT _port;

public:
	static string Test( bool testNetwork, bool testUi,
						const string &serverName, const string &serverAddress );

	// convenience
	bool SetNameAddress( const string &name );
};

#endif // !defined(AFX_SERVER_H__6F641AB2_FB7D_438B_85A0_B0DB6C752669__INCLUDED_)
