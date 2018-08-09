// FormMimeRequestBody.cpp: implementation of the FormMimeRequestBody class.
//
//////////////////////////////////////////////////////////////////////

#include "FormMimeRequestBody.h"
#include "FileUtils.h"
#include "Logger.h"
#include "Base64Coder.h"


// FormMimeRequestBody


FormMimeRequestBody::FormMimeRequestBody( const string &boundary )
	: _boundary( boundary )
	, _body( NULL )
	, _bodyLen( 0 )
	, _base64Encoded( false )
{
	assert( 0 < boundary.length());
}


FormMimeRequestBody::~FormMimeRequestBody()
{	
	if ( _body == NULL )
	{
		for ( DWORD i = 0; i < _bodyParts.size(); ++i )
		{
			FormMimeEntry *e = _bodyParts[i];
			delete e;
		}
	}
	else
		delete [] _body;
}


bool FormMimeRequestBody::AppendVariable( LPCSTR name, LPCSTR value )
{
	if ( name == NULL || value == NULL || _boundary.empty()) { assert(false); return false; }
	if ( _body != NULL ) { assert(false); return false; }
	FormMimeVariable *v = new FormMimeVariable( _boundary );
	bool set = v->Set( name, value );
	if ( !set )
	{
		assert(false);
		if ( v != NULL )
			delete v;
		return false;
	}
	_bodyParts.push_back( v );
	return true;
}


bool FormMimeRequestBody::AppendFile( LPCSTR filePath, LPCSTR name )
{
	if ( filePath == NULL || name == NULL || _boundary.empty()) { assert(false); return false; }
	if ( _body != NULL ) { assert(false); return false; }
	FormMimeFile *f = new FormMimeFile( _boundary );
	bool set = f->Set( filePath, name );
	if ( !set )
	{
		assert(false);
		if ( f != NULL )
			delete f;
		return false;
	}
	_bodyParts.push_back( f );
	return true;
}


bool FormMimeRequestBody::AppendAsFile( LPCSTR data, DWORD dataLen,
											LPCSTR name, LPCSTR filePath )
{
	// dataLen can be zero
	if ( data == NULL || name == NULL || filePath == NULL || _boundary.empty())
		{ assert(false); return false; }
	if ( _body != NULL ) { assert(false); return false; }
	FormMimeFile *f = new FormMimeFile( _boundary );
	bool set = f->Set( data, dataLen, name, filePath );
	if ( !set )
	{
		assert(false);
		if ( f != NULL )
			delete f;
		return false;
	}
	_bodyParts.push_back( f );
	return true;
}


bool FormMimeRequestBody::Generate( bool base64Encode /*= false*/)
{
	if ( _body != NULL ) { assert(false); return false; }

	// compute the body length and set _bodyLen
	DWORD len = 0;
	for ( DWORD i = 0; i < _bodyParts.size(); ++i )
		len += _bodyParts[i]->GetLength();
	if ( 0 < _bodyParts.size())
	{
		len += 2; // "--"
		len += _boundary.length();
		len += 2; // "--"
	}
	_bodyLen = len;

	// If I don't increase the size of the allocated memory by 1, I get
	// a crash bug in ~FormMimeRequestBody
	_body = new char[ _bodyLen + 1 ];
	char *b = _body;

	for ( i = 0; i < _bodyParts.size(); ++i )
	{
		bool outcome = _bodyParts[i]->GetEntry( b, _bodyLen - ( b - _body ));
		assert(outcome);
		b += _bodyParts[i]->GetLength();
	}
	if ( 0 < _bodyParts.size())
	{
		strcpy( b, "--" );
		b += 2;
		strcpy( b, _boundary.c_str());
		b += _boundary.length();
		strcpy( b, "--" );
		b += 2;
	}

	assert( b - _body == GetBodyLength());

	// deallocate _bodyParts
	for ( i = 0; i < _bodyParts.size(); ++i )
	{
		FormMimeEntry *e = _bodyParts[i];
		delete e;
		_bodyParts[i] = NULL;
	}
	_bodyParts.erase( _bodyParts.begin(), _bodyParts.end());

	if ( base64Encode )
	{
		string encoded;
		Base64Coder coder;
		coder.Encode((BYTE *) _body, _bodyLen );
		encoded = coder.EncodedMessage();

		_bodyLen = encoded.size();
		delete [] _body;
		_body = new char[ _bodyLen ];
		strcpy( _body, encoded.c_str());
	}
	_base64Encoded = base64Encode;

	return true;
}


// FormMimeFile


FormMimeFile::FormMimeFile( const string &boundary )
	: FormMimeEntry( boundary ), _fileData( NULL ), _fileLen( 0 )
{}


FormMimeFile::~FormMimeFile()
{
	if ( _fileData != NULL )
		delete [] _fileData;
}


bool FormMimeFile::Set( LPCSTR filePath, LPCSTR name )
{
	if ( filePath == NULL || MAX_PATH <= strlen( filePath )) { assert(false); return false; }

	// Open filePath.  If file can't be opened, try copying it to the temp
	// directory and opening it there.  If the file can be opened in the temp
	// directory, return the file path in tempFilePath.
	HANDLE hFile = INVALID_HANDLE_VALUE;
	string tempFilePath; // will be empty unless a temp file was created
	try
	{
		hFile = OpenTheFile( string(filePath), &tempFilePath );
	}
	catch(...)
	{
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			BOOL closed = CloseHandle( hFile ); assert( closed );
			hFile = INVALID_HANDLE_VALUE;
		}
	}
	if ( hFile == INVALID_HANDLE_VALUE ) { assert(false); return false; }
	
	_name = name;
	_filePath = filePath;
	_contentType = GetContentType( _filePath );
	_fileLen = GetFileSize( hFile, NULL );

	if ( 0 < _fileLen )
	{
		_fileData = new char[ _fileLen ];

		DWORD bytesRead = 0;
		BOOL readed = ReadFile( hFile, _fileData, _fileLen, &bytesRead, NULL );
		if ( !readed || _fileLen != bytesRead ) { assert(false); return false; }
	}

	BOOL closed = CloseHandle( hFile ); assert( closed );

	if ( !tempFilePath.empty() && FileExists( tempFilePath ))
		DeleteTheFile( tempFilePath );

	return true;
}


bool FormMimeFile::Set( LPCSTR data, DWORD dataLen, LPCSTR name, LPCSTR filePath )
{
	// dataLen can be zero
	if ( data == NULL || name == NULL ) { assert(false); return false; }

	if ( 0 < dataLen )
	{
		_fileData = new char[ dataLen ];
		memcpy( _fileData, data, dataLen );
	}
	_name = name;
	_fileLen = dataLen;
	_filePath = filePath;
	_contentType = GetContentType( _filePath );
	return true;
}


bool FormMimeFile::GetEntry( char *buf, DWORD bufLen ) const
{
	if ( buf == NULL || bufLen == 0 ) { assert(false); return false; }

	if ( bufLen < GetLength()) { assert(false); return false; }
	assert( GetLength() <= bufLen );

	const char *bufIn = buf;

	const char crlf[] = "\r\n";
	const char space[] = " ";

	strcpy( buf, "--" );
	buf += 2;
	strcpy( buf, _boundary.c_str());
	buf += _boundary.length();
	strcpy( buf, crlf );
	buf += 2;
	strcpy( buf, _formData.c_str());
	buf += _formData.length();
	strcpy( buf, space );
	buf += 1;
	strcpy( buf, "name=\"" );
	buf += 6;
	strcpy( buf, _name.c_str());
	buf += _name.length();
	strcpy( buf, "\"; " );
	buf += 3;
	strcpy( buf, "filename=\"" );
	buf += 10;
	strcpy( buf, _filePath.c_str());
	buf += _filePath.length();
	strcpy( buf, "\"" );
	buf += 1;
	strcpy( buf, crlf );
	buf += 2;
	strcpy( buf, "Content-Type: " );
	buf += 14;
	strcpy( buf, _contentType.c_str());
	buf += _contentType.length();
	strcpy( buf, "\r\n\r\n" );
	buf += 4;
	memcpy( buf, _fileData, _fileLen );
	buf += _fileLen;
	strcpy( buf, crlf );
	buf += 2;

	assert( buf - bufIn == GetLength());

	return true;
}


DWORD FormMimeFile::GetLength() const
{
	const DWORD crlf = 2; // strlen("\r\n")
	const DWORD space = 1; // strlen(" ")

	DWORD size = 2; // "--"
	size += _boundary.length();
	size += crlf;
	size += ( _formData.length() + space );
	size += string( "name=\"" ).length();
	size += _name.length();
	size += 3; // 1 for "\"" + 1 for ";" + 1 for space
	size += string( "filename=\"" ).length();
	size += _filePath.length();
	size += ( 1 + crlf ); // 1 for "\""
	size += string( "Content-Type: " ).length();
	size += _contentType.length();
	size += ( crlf + crlf );
	size += _fileLen;
	size += crlf;
	return size;
}


// Note: each new file extension for a Maxis product requires a new entry here.
// This is not an ideal solution, but since I have been told we won't be
// building too many more teleporters, I think this is okay.

string FormMimeFile::GetContentType( string fileName )
{
	int i = fileName.rfind( '.' );
	if ( i == string::npos ) { assert(false); return ""; }
	++i; // skip '.'
	string fileExt = fileName.substr( i );
	
	if ( fileExt == string( "sc4" ))
		return "application/octet-stream";
	if ( fileExt == string( "xml" ))
		return "text/xml";
	if ( fileExt == string( "png" ))
		return "image/x-png";
	if ( fileExt == string( "jpg" ))
		return "image/jpeg";
	if ( fileExt == string( "mtl" ))
		return "application/octet-stream";

	assert(false);
	return "";
}


// FormMimeVariable

FormMimeVariable::FormMimeVariable( const string &boundary )
	: FormMimeEntry( boundary )
{}


bool FormMimeVariable::Set( LPCSTR name, LPCSTR value )
{
	if ( name == NULL || value == NULL ) { assert(false); return false; }
	_name = name;
	_value = value;
	return true;
}


bool FormMimeVariable::GetEntry( char *buf, DWORD bufLen ) const
{
	if ( buf == NULL || bufLen == 0 ) { assert(false); return false; }

	if ( bufLen < GetLength()) { assert(false); return false; }
	assert( GetLength() <= bufLen );

	const char *bufIn = buf;

	const char crlf[] = "\r\n";
	const char space[] = " ";

	strcpy( buf, "--" );
	buf += 2;
	strcpy( buf, _boundary.c_str());
	buf += _boundary.length();
	strcpy( buf, crlf );
	buf += 2;
	strcpy( buf, _formData.c_str());
	buf += _formData.length();
	strcpy( buf, space );
	buf += 1;
	strcpy( buf, "name=\"" );
	buf += 6;
	strcpy( buf, _name.c_str());
	buf += _name.length();
	strcpy( buf, "\"" );
	buf += 1;
	strcpy( buf, "\r\n\r\n" );
	buf += 4;
	strcpy( buf, _value.c_str());;
	buf += _value.length();
	strcpy( buf, crlf );
	buf += 2;

	assert( buf - bufIn == GetLength());

	return true;
}


DWORD FormMimeVariable::GetLength() const
{
	const DWORD crlf = 2; // strlen("\r\n")
	const DWORD space = 1; // strlen(" ")

	DWORD size = 2; // for "--"
	size += _boundary.length();
	size += crlf;
	size += ( _formData.length() + space );
	size += string( "name=\"" ).length();
	size += _name.length();
	size += 1; // 1 for "\""
	size += ( crlf + crlf );
	size += _value.length();
	size += crlf;
	return size;
}


// FormMimeEntry

const string FormMimeEntry::_formData = "Content-Disposition: form-data;";


FormMimeEntry::FormMimeEntry( const string &boundary ) : _boundary( boundary )
{
	assert( 0 < boundary.length());
}


string FormMimeRequestBody::Test()
{
	string log = "FormMimeRequestBody::Test()\n";

	FormMimeRequestBody body( "---x-x-" );

	if ( !body.AppendVariable( "asset_type", "SCS" ))
	{
		log += "FormMimeRequestBody::Set() failed\n";
		return false;
	}

	if ( !body.AppendAsFile( "<?xml version=\"1.0\"?>", 21, "xml", "test.xml" ))
	{
		log += "FormMimeRequestBody::Set() failed\n";
		return false;
	}

assert(false);

	return log;
}


/* An example of the FormMime protocol

POST / HTTP/1.1
Host: simcity.max.ad.ea.com:10000
Content-Length: 569807
Cookie: Apache=10.24.26.90.3001058825430652
-----------------------------7d322e3a7054a
Content-Disposition: form-data; name="uploadFileName[xml]"; filename="C:\Documents and Settings\nmolon\My Documents\Amar\Maxisland.xml"
Content-Type: text/xml

<RegionRecord>
	<City>
	</City>
</RegionRecord>

-----------------------------7d322e3a7054a
Content-Disposition: form-data; name="uploadFileName[gamesave]"; filename="C:\Documents and Settings\nmolon\My Documents\Amar\Maxisland.sc4"
Content-Type: application/octet-stream

DBPÚ..ãü
-----------------------------7d322e3a7054a
Content-Disposition: form-data; name="uploadFileName[thumb]"; filename="C:\Documents and Settings\nmolon\My Documents\Amar\City - Maxisland.png"
Content-Type: image/x-png

‰PNG.Jñÿ
-----------------------------7d322e3a7054a
Content-Disposition: form-data; name="asset_type"

SCS
-----------------------------7d322e3a7054a--

*/

