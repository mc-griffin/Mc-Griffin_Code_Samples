// Utilities.h: interface for the Utilities class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UTILITIES_H__FE80896C_631F_4533_B915_280221E900F4__INCLUDED_)
#define AFX_UTILITIES_H__FE80896C_631F_4533_B915_280221E900F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>
#include <vector>
using namespace std;


extern bool IsAllDigits( const string &s );
extern bool IsUnsignedLong( const string &s );
extern string NumberToString( unsigned long n );
extern unsigned long AtoiHex( const string &hexString );
extern string ToUpper( const string &s );
extern string ToLower( const string &s );
extern bool StringToBool( const string &s );

// INI file utils
extern bool GetSectionNames( const string &filePath, vector<string> *namesList );
extern bool GetNamesInSection( const string &filePath, const string &sectionName,
									vector<string> *namesList );


#endif // !defined(AFX_UTILITIES_H__FE80896C_631F_4533_B915_280221E900F4__INCLUDED_)
