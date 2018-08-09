// RequestBody.h: interface for the RequestBody class.
//
// If a boundary is not given to the constructor, all methods that
// attempt to add to the body fail.  Note we could change this so
// that if a boundary is not given to the constructor, then one is
// generated when an attempt is made to add to the body.  An example of
// a generated boundary is "--------" + hex representation of time that
// boundary is generated.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REQUESTBODY_H__560198E3_D747_4FA0_BE36_2B025BF6CF2A__INCLUDED_)
#define AFX_REQUESTBODY_H__560198E3_D747_4FA0_BE36_2B025BF6CF2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "FormMimeRequestBody.h"
#include "RequestBodyOld.h"


class RequestBody  
{
protected:
	FormMimeRequestBody _body;
	// _oldBody is used for teleporters released before August 2003:
	// the old HTTP file format with base64 encoding
	RequestBodyOld _oldBody; 

	// _fileCount is the number of files appended via
	// AppendFile() and AppendAsFile()
	WORD _fileCount;

public:
	RequestBody( string boundary, bool useOldBody = false );

	bool Append( const string &name, const string &data );
	bool AppendFile( const string &tmpType, const string &filePath,
						const string &tmpName = string());
	bool AppendAsFile( LPCSTR data, DWORD dataLen,
						const string &tmpType, const string &name );
	bool AppendAsFile( const string &data,
						const string &tmpType, const string &name );
	bool AppendAsFileEncode( LPCSTR data, DWORD dataLen,
						const string &tmpType, const string &name );
	bool AppendAsFileEncode( const string &data,
						const string &tmpType, const string &name );
	WORD GetFileCount() const { return _fileCount; }

	bool Generate( bool base64Encode = false );
	const char *const GetBody() const;
	DWORD GetBodyLength() const;

	static string Test();
	static string TestErrors();
	static string TestUseCases();

	const bool _useOldBody;
	
	// AppendActionUpload() is only used by old body
	bool AppendActionUpload();

	// for testing
	void Set( LPCSTR b );
};

#endif // !defined(AFX_REQUESTBODY_H__560198E3_D747_4FA0_BE36_2B025BF6CF2A__INCLUDED_)
