// Utilities.cpp: implementation of the Utilities class.
//
//////////////////////////////////////////////////////////////////////

#include "Utilities.h"
#include <windows.h>
#include <assert.h>
#include <stdio.h> // I had to add this when I used this file for SimCityFile


bool GetSectionNames( const string &filePath, vector<string> *namesList )
{
	if ( filePath.empty() || MAX_PATH < filePath.length() || namesList == NULL )
		{ assert(false); return false; }

	const DWORD bufSize = 1024;
	char buf[ bufSize ] = "";
	const char defaultValue[] = "";

	DWORD r = GetPrivateProfileString( NULL, NULL, defaultValue,
				buf, bufSize - 1, filePath.c_str());
	if ( r < 1 || bufSize <= r )
		return false;

	int left = 0, right = 0;
	bool endOfValue = ( buf[right] == '\0' && buf[right+1] == '\0' );
	while ( right < bufSize - 1 && !endOfValue )
	{
		while ( right < bufSize - 1 && buf[right] != '\0' )
			++right;

		string s = buf + left;

		namesList->push_back( s );

		if ( right < bufSize - 1 )
		{
			++right;
			left = right;

			endOfValue = ( buf[right] == '\0' && buf[right+1] == '\0' );
		}
		else
			endOfValue = true;
	}

	return true;
}

// Add to namesList all the names in the specified section in an INI file
bool GetNamesInSection( const string &filePath, const string &sectionName,
					vector<string> *namesList )
{
	if ( sectionName.empty() || namesList == NULL )
		{ assert(false); return false; }

	const DWORD bufSize = 512;
	char buf[ bufSize ] = "";
	const char defaultValue[] = "";

	DWORD r = GetPrivateProfileString( sectionName.c_str(), NULL, defaultValue,
				buf, bufSize - 1, filePath.c_str());
	if ( r < 1 || bufSize <= r )
		return false;

	int left = 0, right = 0;
	bool endOfValue = ( buf[right] == '\0' && buf[right+1] == '\0' );
	while ( right < bufSize - 1 && !endOfValue )
	{
		while ( right < bufSize - 1 && buf[right] != '\0' )
			++right;

		string s = buf + left;

		namesList->push_back( s );

		if ( right < bufSize - 1 )
		{
			++right;
			left = right;

			endOfValue = ( buf[right] == '\0' && buf[right+1] == '\0' );
		}
		else
			endOfValue = true;
	}

	return true;
}


bool IsAllDigits( const string &s )
{
	for ( int i = 0; i < s.length(); ++i )
		if ( !isdigit( s[i]))
			return false;
	return true;
}


bool IsUnsignedLong( const string &s )
{
	if ( s.empty() || !IsAllDigits( s ))
		return false;

	// LONG_MAX is 4294967295, 10 digits
	if ( 10 < s.length())
		return false;

	if ( s.length() < 10 )
		return true;

	if ( '4' < s[0])
		return false;
	else if ( '4' == s[0])
	{
		if ( '2' < s[1])
			return false;
		else if ( '2' == s[1])
		{
			if ( '9' == s[2])
			{
				if ( '4' < s[3])
					return false;
				else if ( '4' == s[3])
				{
					if ( '9' == s[4])
					{
						if ( '6' < s[5])
							return false;
						else if ( '6' == s[5])
						{
							if ( '7' < s[6])
								return false;
							else if ( '7' == s[6])
							{
								if ( '2' < s[7])
									return false;
								else if ( '2' == s[7])
									if ( '9' == s[8])
										return s[9] < '6';
							}
						}
					}
				}
			}
		}
	}

	return true;
}


string NumberToString( unsigned long n )
{
	char buf[16] = "";
	sprintf( buf, "%u", n );
	assert( IsUnsignedLong( buf ));
	return buf;
}


// I was surprised that I couldn't find a Win32 API function to
// convert a string containing a hex value to a DWORD.
unsigned long AtoiHex( const string &hexStr )
{
	// "FFFFFFFF" is max
	if ( hexStr.empty() || 8 < hexStr.length()) { assert(false); return 0; }

	// n is the return value
	unsigned long n = 0;

	// power of 16 for current position
	unsigned long positionWeight = 1;

	for ( int i = hexStr.length() - 1; 0 <= i; --i )
	{
		if ( i < hexStr.length() - 1 ) positionWeight *= 16;

		unsigned long positionValue = 0;

		switch ( hexStr[i])
		{
			case '0': positionValue = 0; break;
			case '1': positionValue = 1; break;
			case '2': positionValue = 2; break;
			case '3': positionValue = 3; break;
			case '4': positionValue = 4; break;
			case '5': positionValue = 5; break;
			case '6': positionValue = 6; break;
			case '7': positionValue = 7; break;
			case '8': positionValue = 8; break;
			case '9': positionValue = 9; break;
			case 'a':
			case 'A': positionValue = 10; break;
			case 'b':
			case 'B': positionValue = 11; break;
			case 'c':
			case 'C': positionValue = 12; break;
			case 'd':
			case 'D': positionValue = 13; break;
			case 'e':
			case 'E': positionValue = 14; break;
			case 'f':
			case 'F': positionValue = 15; break;
			default:
				assert(false);
				return 0;
		}

		unsigned long product = positionValue * positionWeight;

		n += product;
	}

	return n;
}
/* Some tests
	if ( AtoiHex( "FfFfFfFf" ) != ULONG_MAX )
	if ( AtoiHex( "FfFf" ) != USHRT_MAX )
	if ( AtoiHex( "0" ) != 0 )
	if ( AtoiHex( "1" ) != 1 )
	if ( AtoiHex( "9" ) != 9 )
	if ( AtoiHex( "a" ) != 10 )
	if ( AtoiHex( "10" ) != 16 )
*/


string ToUpper( const string &s )
{
	string up;
	char buf[2];
	buf[ 1 ] = '\0';
	for ( unsigned long i = 0; i < s.length(); ++i )
	{
		buf[ 0 ] = toupper( s.at( i ));
		up += buf;
	}
	return up;
}


string ToLower( const string &s )
{
	string low;
	char buf[2];
	buf[ 1 ] = '\0';
	for ( unsigned long i = 0; i < s.length(); ++i )
	{
		buf[ 0 ] = tolower( s.at( i ));
		low += buf;
	}
	return low;
}


bool StringToBool( const string &s )
{
	if ( _stricmp( s.c_str(), "true" ) == 0 )
		return true;
	assert( _stricmp( s.c_str(), "false" ) == 0 );
	return false;
}

/*
// all parameters are in pixels
void CenterRectangleInScreen( int width, int height, short *left, short *top )
{
	if ( !left || !top ) { assert(false); return; }

	int screenWidth = GetSystemMetrics( SM_CXFULLSCREEN );
	int screenHeight = GetSystemMetrics( SM_CYFULLSCREEN );
	// I chose to use the SM_*FULLSCREEN parameters instead of SM_*SCREEN
	// because SM_*SCREEN causes the rectangle to appear further down the screen.

	assert( width <= screenWidth && height <= screenHeight );

	*left  = ( screenWidth - width ) / 2;
	*top = ( screenHeight - height ) / 2;
}
*/
