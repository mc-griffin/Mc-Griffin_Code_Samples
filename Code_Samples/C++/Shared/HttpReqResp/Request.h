// Request.h: interface for the Request class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REQUEST_H__9A5CBE39_107F_40D4_984D_4DD8D010AB21__INCLUDED_)
#define AFX_REQUEST_H__9A5CBE39_107F_40D4_984D_4DD8D010AB21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace std;

#include "Server.h"
#include "RequestLine.h"
#include "RequestHeaders.h"


class Request  
{
public:
	Request( const Server *const server );
	virtual ~Request() {}

	bool SetUrl( const string &url );
	string GetUrl() const;
	string GetMethod() const;

	RequestHeaders _headers;

	const Server *const _server;

	int SetMinimalHeaders();
	virtual int GetMinimalHeaders( vector< NameValuePair > *headers ) const = 0;

	virtual bool Generate( bool base64Encode = false );
	virtual const char *const GetBody() const;
	virtual DWORD GetBodyLength() const;

	virtual bool IsSet() const;

	string GetHead() const;

protected:
	// protected because user cannot set _line._method
	RequestLine _line;	
};

//	Test Data
#if 0
1/16/2002 15:23:26.827      GET / HTTP/1.1
Accept: application/vnd.ms-excel, application/msword, application/vnd.ms-powerpoint, */*
Accept -Language: en -au
Accept -Encoding: gzip , deflate
User-Agent: Mozilla/4.0 (compatible; MSIE 4.01; Windows 98)
Host: localhost
Connection: Keep -Alive

1/16/2002 15:23:42.640      GET /us HTTP/1.1
Accept: application/vnd.ms-excel, application/msword, application/vnd.ms-powerpoint, */*
Accept -Language: en -au
Accept -Encoding: gzip , deflate
User-Agent: Mozilla/4.0 (compatible; MSIE 4.01; Windows 98)
Host: localhost
Connection: Keep -Alive

1/16/2002 15:23:50.359      GET /us/ HTTP/1.1
Accept: application/vnd.ms-excel, application/msword, application/vnd.ms-powerpoint, */*
Accept -Language: en -au
Accept -Encoding: gzip , deflate
User-Agent: Mozilla/4.0 (compatible; MSIE 4.01; Windows 98)
Host: localhost
Connection: Keep -Alive

*/
#endif

#endif // !defined(AFX_REQUEST_H__9A5CBE39_107F_40D4_984D_4DD8D010AB21__INCLUDED_)
