// HeadRequest.h: interface for the HeadRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HEADREQUEST_H__76F61D39_8BA9_4BCC_B62A_0FDE39C7710C__INCLUDED_)
#define AFX_HEADREQUEST_H__76F61D39_8BA9_4BCC_B62A_0FDE39C7710C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Request.h"


class HeadRequest : public Request  
{
public:
	HeadRequest( const Server *const server );
	virtual ~HeadRequest() {}

	int GetMinimalHeaders( vector< NameValuePair > *headers ) const;

	static string Test( bool testNetwork,
						const string &serverName, const string &serverAddress );
};

#endif // !defined(AFX_HEADREQUEST_H__76F61D39_8BA9_4BCC_B62A_0FDE39C7710C__INCLUDED_)
