// RequestHeaders.h: interface for the RequestHeaders class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REQUESTHEADERS_H__1A3E7BA2_816F_4271_B449_9EA81F4D670E__INCLUDED_)
#define AFX_REQUESTHEADERS_H__1A3E7BA2_816F_4271_B449_9EA81F4D670E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "NameValuePair.h"
#include "Error.h"

// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <list>
#include <vector>


struct RequestHeaders : protected list< NameValuePair >
{
	bool Append( const NameValuePair &nv );
	DWORD Append( const vector< NameValuePair > &headerLines );
	bool AppendCookie( const string &serverName );

	string GetString() const;

	DWORD Find( const string &name, list<string> *found, bool caseSensitive = false ) const;

	WORD Count() const;

	static string Test();
};

#endif // !defined(AFX_REQUESTHEADERS_H__1A3E7BA2_816F_4271_B449_9EA81F4D670E__INCLUDED_)
