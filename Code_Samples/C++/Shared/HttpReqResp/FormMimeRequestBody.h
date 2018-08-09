// FormMimeRequestBody.h: interface for the FormMimeRequestBody class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMMIMEREQUESTBODY_H__F91B2E8C_C02A_473D_8F10_C566D4A99D93__INCLUDED_)
#define AFX_FORMMIMEREQUESTBODY_H__F91B2E8C_C02A_473D_8F10_C566D4A99D93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// The name "FormMime" is meant to suggest a Form POST with MIME variables.
// Noel and I discovered this protocol by sniffing the contents sent by
// a PHP page in a browser.  I can't find a description of this protocol anywhere, so I
// named it myself.


// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>
#include <vector>
using namespace std;

#include <windows.h>
#include <assert.h>


// Dynamic Memory Management
//
// - _bodyParts in Append*(), Generate() deallocates _bodyParts and allocates _body.
// - _fileData allocated in FormMimeFile::Set(LPCSTR,LPCSTR) and
//   FormMimeFile::Set(LPCSTR,DWORD,LPCSTR,LPCSTR).


// FormMimeEntry is an abstract class.  Polymorphism is used so we can
// create lists of FormMimeVariable and FormMimeFile objects.  These objects
// are accessed by the virtual functions.

class FormMimeEntry
{
protected:
	string _name;

public:
	virtual bool GetEntry( char *buf, DWORD bufLen ) const = 0;
	virtual DWORD GetLength() const = 0;

	const string _boundary;
	static const string _formData;

	FormMimeEntry( const string &boundary );
	virtual ~FormMimeEntry()
		{ /* destructor is virtual because subclasses need to release memory */	}
};


class FormMimeVariable : public FormMimeEntry
{
protected:
	string _value; // value of FormMimeEntry._name

public:
	bool Set( LPCSTR name, LPCSTR value );

	virtual bool GetEntry( char *buf, DWORD bufLen ) const;
	virtual DWORD GetLength() const;

	FormMimeVariable( const string &boundary );
};


class FormMimeFile : public FormMimeEntry
{
protected:
	string _contentType;
	string _filePath; // can I just end file name?
	char *_fileData;
	DWORD _fileLen;

public:
	bool Set( LPCSTR filePath, LPCSTR name );
	bool Set( LPCSTR data, DWORD dataLen, LPCSTR name, LPCSTR filePath );

	virtual bool GetEntry( char *buf, DWORD bufLen ) const;
	virtual DWORD GetLength() const;

	FormMimeFile( const string &boundary );
	virtual ~FormMimeFile();

	static string GetContentType( string fileName );

private:
	FormMimeFile( const FormMimeFile& );
	FormMimeFile &operator=( const FormMimeFile& );
};


// 2 states:  appending and generating.  The initial state is appending
// in which calls to Append*() add to _bodyParts.  One call to Generate()
// causes a state transition to generating.  This call sets _body, _bodyLen,
// and _base64Encoded.  Memory is allocated for _body.  _bodyParts is
// deallocated

class FormMimeRequestBody
{
protected:
	vector<FormMimeEntry *> _bodyParts;
	char *_body;
	DWORD _bodyLen;
	bool _base64Encoded;

public:
	bool AppendVariable( LPCSTR name, LPCSTR value );
	bool AppendFile( LPCSTR filePath, LPCSTR name );
	bool AppendAsFile( LPCSTR data, DWORD dataLen, LPCSTR name, LPCSTR filePath );

	bool Generate( bool base64Encode = false);

	const char *const GetBody() const { return _body; }
	DWORD GetBodyLength() const { return _bodyLen; }

	FormMimeRequestBody( const string &boundary );
	virtual ~FormMimeRequestBody();

	const string _boundary;

private:
	FormMimeRequestBody( const FormMimeRequestBody& );
	FormMimeRequestBody &operator=( const FormMimeRequestBody& );

public:
	static string Test();
};


#endif // !defined(AFX_FORMMIMEREQUESTBODY_H__F91B2E8C_C02A_473D_8F10_C566D4A99D93__INCLUDED_)
