// RequestBody.cpp: implementation of the RequestBody class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RequestBody.h"
#include "FileUtils.h"


RequestBody::RequestBody( string boundary, bool useOldBody /*=false*/)
	: _fileCount( 0 )
	, _body( boundary )
	, _oldBody( boundary )
	, _useOldBody( useOldBody )
{}


bool RequestBody::Append( const string &name, const string &data )
{
	if ( name.empty() || data.empty()) { assert(false); return false; }

	if ( _useOldBody )
		return _oldBody.Append( name.c_str(), data.c_str());
	else
		return _body.AppendVariable( name.c_str(), data.c_str());
}


bool RequestBody::AppendFile( const string &tmpType, const string &filePath,
								const string &tmpName /*=string()*/)
{
	// input validation, set fileName, hFile, and fileSize
	string fileName;
	try
	{
		if ( tmpType.empty() || filePath.empty()
				|| MAX_PATH < filePath.length() || HasEndingBackSlash( filePath ))
			throw "invalid input 1";

		DWORD rightBackSlash = filePath.rfind( "\\" );
		if ( rightBackSlash == string::npos )
			throw "invalid input 3";

		fileName = filePath.substr( rightBackSlash + 1 );
		if ( fileName.empty())
			throw "invalid input 4";
	}
	catch ( string s )
	{
		string error = "RequestBody::AppendFile: " + s;
		LOG( error );
		assert(false);
		return false;
	}

	bool outcome = false;
	if ( _useOldBody )
	{
		if ( tmpName.empty())
			outcome = _oldBody.AppendFile( filePath.c_str(), tmpType.c_str());
		else
			outcome = _oldBody.AppendFile(
							filePath.c_str(), tmpType.c_str(), tmpName.c_str());
	}
	else
	{
		// If we retrofit the Sims Family exchange to use the new body format,
		// we'll need to pass in the tmpName.
		outcome = _body.AppendFile( filePath.c_str(), tmpType.c_str());
	}

	++_fileCount;
	return outcome;
}


bool RequestBody::AppendAsFile( LPCSTR data, DWORD dataLen,
						const string &tmpType, const string &name )
{
	bool outcome = false;
	if ( _useOldBody )
	{
		string sData = data;
		if ( sData.size() != dataLen )
		{
			// need to write a new RequestBodyOld::AppendAsFile()
			// which handles binary data
			assert(false);
			return false;
		}
		outcome = _oldBody.AppendAsFile( sData.c_str(), tmpType.c_str(), name.c_str());
	}
	else
		outcome = _body.AppendAsFile( data, dataLen, tmpType.c_str(), name.c_str());
	++_fileCount;
	return outcome;
}


bool RequestBody::AppendAsFile( const string &data,
						const string &tmpType, const string &name )
{
	return AppendAsFile( data.c_str(), data.size(), tmpType, name );
}


bool RequestBody::AppendAsFileEncode( LPCSTR data, DWORD dataLen,
						const string &tmpType, const string &name )
{
	bool outcome = false;
	if ( _useOldBody )
	{
		outcome = _oldBody.AppendAsFileEncode( data, dataLen, tmpType.c_str(), name.c_str());
	}
	else
	{
		// You should never need to encode in the new format
		assert(false);
		return false;
	}

	++_fileCount;
	return outcome;
}


bool RequestBody::AppendAsFileEncode( const string &data,
						const string &tmpType, const string &name )
{
	return AppendAsFileEncode( data.c_str(), data.size(), tmpType, name );
}


// AppendActionUpload() is only used by old body
bool RequestBody::AppendActionUpload()
{
	if ( !_useOldBody ) { assert(false); return 0; }
	return _oldBody.AppendActionUpload();
}


bool RequestBody::Generate( bool base64Encode /*=false*/)
{
	if ( _useOldBody )
		return _oldBody.Generate();
	else
		return _body.Generate( base64Encode );
}


const char *const RequestBody::GetBody() const
{
	if ( _useOldBody )
		return _oldBody.GetBody();
	else
		return _body.GetBody();
}


DWORD RequestBody::GetBodyLength() const
{
	if ( _useOldBody )
		return _oldBody.GetBodyLength();
	else
		return _body.GetBodyLength();
}


void RequestBody::Set( LPCSTR b )
{
	if ( _useOldBody )
		_oldBody.Set( b );
	else
	{
		// You need to write FormMimRequestBody::Set().
	}
}



string RequestBody::Test()
{
	string log = "Test RequestBody\n";

	log += TestUseCases();
	log += TestErrors();

	return log;
}


string RequestBody::TestErrors()
{
	string log;

	// boundary and body
	{
		const string boundary = "------T-E-S-T--B-O-U-N-D-A-R-Y-";

		RequestBody b( boundary, false );

		if ( b.GetBodyLength() != 0 )
			log += "GetContentLength() failed\n";

		if ( b.GetBody() == NULL )
			log += "GetBody() failed\n";

		if ( b.Append( "", "" ) || b.Append( "a", "" ) || b.Append( "", "b" ))
			log += "Append(n,v) failed\n";
	}

	{
		const string boundary = "------T-E-S-T--B-O-U-N-D-A-R-Y-";

		RequestBody b( boundary, false );

		if ( b.AppendFile( "", "" ))
			log += "AppendFile() failed\n";

		if ( b.AppendFile( "x", "__THIS_FILE_SHOULD\\NOT_EXIST__" ))
			log += "AppendFile() failed\n";
	}

	return log;
}


string RequestBody::TestUseCases()
{
	string log;

	const string boundary = "------T-E-S-T--B-O-U-N-D-A-R-Y-";
	const string endingBoundary = string("--") + boundary + "--";
	const string bodyEnding = string("\r\n") + endingBoundary;

	RequestBody b( boundary, true );

	string body1 = "--";
	body1 += boundary + "\r\nContent-Disposition: form-data; name=\"a\"\r\n\r\nb\r\n";

	if ( !b.Append( "a", "b" ))
		log += "Append(n,v) failed\n";

	if ( b.GetBodyLength() != body1.length())
		log += "GetBodyLength() failed\n";

	if ( !b.AppendActionUpload())
		log += "AppendActionUpload() failed\n";

	if ( b.GetFileCount() != 0 )
		log += "GetFileCount() failed\n";
	
	string body2 = "--";
	body2 += boundary + "\r\nContent-Disposition: form-data; name=\"action\"\r\n\r\nupload\r\n";

	if ( b.GetBodyLength() != body1.length() + body2.length())
		log += "GetBodyLength() failed\n";

	if ( !b.Append( "c", "d" ))
		log += "Append() failed\n";

	if ( !b.AppendActionUpload())
		log += "AppendActionUpload() failed\n";

	string body3 = "--";
	body3 += boundary + "\r\nContent-Disposition: form-data; name=\"c\"\r\n\r\nd\r\n";
	body3 += "--";
	body3 += boundary + "\r\nContent-Disposition: form-data; name=\"action\"\r\n\r\nupload\r\n";

	if ( b.GetBodyLength() != body1.length() + body2.length() + body3.length())
		log += "GetBodyLength() failed\n";

	// AppendFile
	char dir[MAX_PATH+1] = "";
	GetCurrentDirectory( MAX_PATH, dir );
	string filePath = dir;
	filePath += "\\request_body_test.txt";

	if ( !b.AppendFile( "test", filePath ))
		log += "AppendFile() failed\n";

	if ( b.GetFileCount() != 1 )
		log += "GetFileCount() failed\n";
	
	// contents of request_body_test.txt: "test RequestBody!"
	string encoded = "dGVzdCBSZXF1ZXN0Qm9keSE="; // contents base64 encoded
	string body4 = "--";
	body4 += boundary;
	body4 += "\r\nContent-Disposition: form-data; name=\"tmpName[]\"\r\n\r\n";
	body4 += "request_body_test.txt\r\n";
	body4 += "--";
	body4 += boundary;
	body4 += "\r\nContent-Disposition: form-data; name=\"tmpType[]\"\r\n\r\n";
	body4 += "test\r\n";
	body4 += "--";
	body4 += boundary;
	body4 += "\r\nContent-Disposition: form-data; name=\"fileData[]\"\r\n\r\n";
	body4 += encoded;
	body4 += "\r\n";

	if ( !b.AppendAsFile( "test.xyz", "test for AppendAsFile", "tmpTypo" ))
		log += "AppendAsFile() failed\n";

	if ( b.GetFileCount() != 2 )
		log += "GetFileCount() failed\n";
	
	encoded = "dGVzdCBmb3IgQXBwZW5kQXNGaWxl";
	string body5 = "--";
	body5 += boundary;
	body5 += "\r\nContent-Disposition: form-data; name=\"tmpName[]\"\r\n\r\n";
	body5 += "test.xyz\r\n";
	body5 += "--";
	body5 += boundary;
	body5 += "\r\nContent-Disposition: form-data; name=\"tmpType[]\"\r\n\r\n";
	body5 += "tmpTypo\r\n";
	body5 += "--";
	body5 += boundary;
	body5 += "\r\nContent-Disposition: form-data; name=\"fileData[]\"\r\n\r\n";
	body5 += encoded;
	body5 += "\r\n";

	return log;
}

