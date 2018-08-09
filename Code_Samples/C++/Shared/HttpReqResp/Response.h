// Response.h: interface for the Response class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESPONSE_H__3EC6B47F_FABC_4E2E_8AC4_125424E83B01__INCLUDED_)
#define AFX_RESPONSE_H__3EC6B47F_FABC_4E2E_8AC4_125424E83B01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ResponseHeaders.h"


// The class ResponseHeaders is potentially vulnerable to thread problems, and
// since the Response class contains ResponseHeaders, Response is also vulnerable.
// However, I am convinced that ResponseHeaders is thread safe now (May 23, 2002).
// Read ResponseHeaders.h for my explanation.


class Response  
{
protected:
	WORD _statusCode;
	string _phrase;
	string _version; // HTTP version

	char *_body;
	DWORD _bodyLength;
	DWORD _bodyBufLength; // _body may be bigger than _bodyLength

public:
	ResponseHeaders _headers;

	Response() : _statusCode(0), _body(NULL), _bodyLength(0), _bodyBufLength(0)
		{}
	~Response();

	bool IsSet() const;
	DWORD SetHead( LPCSTR head, DWORD len = 0 );

	string GetCodeAndPhrase() const;

	bool GetExpectedContentLength( DWORD *expectedLen ) const;
	DWORD GetActualContentLength() const;

	// 2 ways to add a body:
	// 1. First call AllocateBody(), which allocates a single block for
	//    the body.  Then Call AppendToBody() to add to that single block.
	// 2. Just call AppendToBody().  Each call to AppendToBody() will
	//    cause an allocation of a new block and a deallocation of the old.
	// Note:  Method  1 is approximately 300 times faster than 2.
	bool AllocateBody( DWORD bodyBufLen );
	bool AppendToBody( LPCSTR bodyPart, DWORD len );

	const char *const GetBody() const;
	bool GetBody( char *buf, DWORD bufLen ) const;
	DWORD GetBodyLength() const { return _bodyLength; }

	bool SaveBodyAsFile( const string &filePath, bool base64decode = true ) const;

	void Reset();

	static bool EnsureCRLF( LPCSTR unix, string *windows );

	static string Test( const string &serverName, const string &serverAddress );

protected:
	static bool AtEndOfHead( LPCSTR buffer, DWORD index );
};

#endif // !defined(AFX_RESPONSE_H__3EC6B47F_FABC_4E2E_8AC4_125424E83B01__INCLUDED_)
