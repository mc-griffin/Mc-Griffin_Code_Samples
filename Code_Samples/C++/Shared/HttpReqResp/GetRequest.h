// GetRequest.h: interface for the GetRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GETREQUEST_H__9E0615BF_57EC_40A5_B682_55B9BB4289EF__INCLUDED_)
#define AFX_GETREQUEST_H__9E0615BF_57EC_40A5_B682_55B9BB4289EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Request.h"


class GetRequest : public Request  
{
public:
	GetRequest( const Server *const server );
	virtual ~GetRequest() {}

	int GetMinimalHeaders( vector< NameValuePair > *headers ) const;

	static string Test( bool testNetwork,
						const string &serverName, const string &serverAddress );
};

#endif // !defined(AFX_GETREQUEST_H__9E0615BF_57EC_40A5_B682_55B9BB4289EF__INCLUDED_)
