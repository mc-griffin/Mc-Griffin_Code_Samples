// XmlDoc.cpp: implementation of the XmlDoc class.
//
//////////////////////////////////////////////////////////////////////

#include "XmlDoc.h"
#include "FileUtils.h"
#include <assert.h>
#include <stdio.h> // I had to add this when I used this file for SimCityFile


/*
http://support.microsoft.com/support/kb/articles/Q251/3/54.ASP
     &amp;		&
     &lt;		<
     &gt;		>
     &quote;	"
     &apos;		' 
*/
bool XmlDoc::IsCharToEncode( char c )
{
	switch ( c )
	{
		case '&':
		case '<':
		case '>':
		case '"':
		case '\'':
			return true;
	}
	return false;
}


string XmlDoc::EncodeChar( char c )
{
	switch ( c )
	{
		case '&': return string( "&amp;" );
		case '<': return string( "&lt;" );
		case '>': return string( "&gt;" );
		case '"': return string( "&quot;" );
		case '\'': return string( "&apos;" );
	}

	assert(false);

	char bad[4] = "";
	sprintf( bad, "%c", c );

	return string( bad );
}


string XmlDoc::GetXmlEncoded( const string &s )
{
	string encoded;
	for ( DWORD i = 0; i < s.length(); ++i )
	{
		char c = s.at( i );
		if ( IsCharToEncode( c ))
			encoded += EncodeChar( c );
		else
		{
			char theChar[4] = "";
			sprintf( theChar, "%c", c );
			encoded += string( theChar );
		}
	}
	return encoded;
}


bool XmlDoc::Test()
{
	printf( "Test XmlDoc\n" );
	bool passed = true;

	// test static methods

	// IsCharToEncode()
	for ( char c = 32; c < 127; ++c )
		if ( IsCharToEncode( c ))
			if ( c != '&' && c != '<' && c != '>' && c != '"' && c != '\'' )
			{
				printf( "IsCharToEncode() failed\n" );
				passed = false;
			}

	// EncodeChar
	// I had to change the literal strings, for example "&amp;", to strings,
	// for example string( "&amp;" ), when I used this file for SimCityFile
	if ( EncodeChar( '&' ) != string( "&amp;" ) || EncodeChar( '<' ) != string( "&lt;" )
			|| EncodeChar( '>' ) != string( "&gt;" ) || EncodeChar( '"' ) != string( "&quot;" )
				|| EncodeChar( '\'' ) != string( "&apos;" ))
	{
		printf( "EncodeChar() failed\n" );
		passed = false;
	}

	// GetXmlEncoded()
	char before[] = "a&b<c>d\"e'f";
	char after[] = "a&amp;b&lt;c&gt;d&quot;e&apos;f";
	if ( GetXmlEncoded( before ) != string( after ))
	{
		printf( "GetXmlEncoded() failed\n" );
		passed = false;
	}

	return passed;

#if 0
	// Template of tests to do in any derived class:
	// (This code is not executed here because the behavior of
	// SaveFile() depends on how GetXmlDoc() is implemented.)

	const char fileName[] = "__xmlDoc_test";

	// assumptions
	assert( xmlDoc.IsSet());
	assert( !FileExists( fileName ));

	if ( !xmlDoc.SaveFile( fileName ) || !FileExists( fileName ))
	{
		printf( "SaveFile() failed\n" );
		passed = false;
	}

	if ( !xmlDoc.IsSet() || xmlDoc.GetFilePath() != fileName )
	{
		printf( "IsSet() failed\n" );
		passed = false;
	}

	if ( !xmlDoc.DeleteTheFile() || FileExists( fileName ))
	{
		printf( "DeleteTheFile() failed\n" );
		passed = false;
	}
#endif
}



