// PostRequest.h: interface for the PostRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POSTREQUEST_H__635B587C_329D_4880_953E_4026C00AC7E3__INCLUDED_)
#define AFX_POSTREQUEST_H__635B587C_329D_4880_953E_4026C00AC7E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Request.h"
#include "RequestBody.h"


class PostRequest : public Request  
{
public:
	PostRequest( const Server *const server, string boundary, bool useOldBody = false );
	virtual ~PostRequest() {}

	virtual int GetMinimalHeaders( vector< NameValuePair > *headers ) const;

	virtual bool IsSet() const;

	virtual bool Generate( bool base64Encode = false );
	virtual const char *const GetBody() const;
	virtual DWORD GetBodyLength() const;

	const string _boundary;
	RequestBody _body;

public:
	static string Test( bool testNetwork,
						const string &serverName, const string &serverAddress );
};

#endif // !defined(AFX_POSTREQUEST_H__635B587C_329D_4880_953E_4026C00AC7E3__INCLUDED_)
