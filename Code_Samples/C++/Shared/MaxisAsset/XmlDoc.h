// XmlDoc.h: interface for the XmlDoc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLDOC_H__01A6E3F1_EFD1_46FC_8510_1F9504CA86E1__INCLUDED_)
#define AFX_XMLDOC_H__01A6E3F1_EFD1_46FC_8510_1F9504CA86E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <windows.h>

// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>

using namespace std;


class XmlDoc
{
public:
	virtual string GetXmlDoc() const = 0;

	static bool IsCharToEncode( char c );
	static string EncodeChar( char c );
	static string GetXmlEncoded( const string &s );

	static bool Test();
};

#endif // !defined(AFX_XMLDOC_H__01A6E3F1_EFD1_46FC_8510_1F9504CA86E1__INCLUDED_)
