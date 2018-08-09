// NameValuePair.cpp: implementation of the NameValuePair class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NameValuePair.h"
#include "Utilities.h"


NameValuePair::NameValuePair( const NameValuePair &nv )
{
	first = nv.Name();
	second = nv.Value();
	_set = true;
}


NameValuePair &NameValuePair::operator=( const NameValuePair &nv )
{
	if ( this == &nv ) return *this;
	first = nv.Name();
	second = nv.Value();
	_set = true;
	return *this;
}


bool NameValuePair::Set( const char *name, const char *value )
{
	_set = false;

	if ( !IsValidString( name ) || !IsValidString( value, true ))
	{
		assert(false);
		return false;
	}
/*
	// note: if name and value aren't translated into strings, we crash!
	NameValuePair nv = (NameValuePair &) make_pair( string( name ), string( value ));

	*((NameValuePair *) this) = nv;

	_set = true;
*/
	first = name;
	second = value;
	_set = true;

	return true;
}


bool NameValuePair::Set( const string &name, const string &value )
{
	_set = false;

	if ( !IsValidString( name.c_str()) || !IsValidString( value.c_str(), true ))
	{
		assert(false);
		return false;
	}
/*
	NameValuePair nv = (NameValuePair &) make_pair( name, value );

	*((NameValuePair *) this) = nv;

	_set = true;
*/

	first = name;
	second = value;
	_set = true;

	return true;
}


// BNF: name space* : space* value is legal, but spaces before name or after value are not
bool NameValuePair::Set( const char *line )
{
	_set = false;

	if ( line == NULL || *line == '\0' ) { assert(false); return false; }

	// find colon
	char *colon = strchr( line, ':' );
	if ( colon == NULL ) { assert(false); return false; }
	assert( *colon == ':' );

	// skip any spaces before colon
	char *p = colon - 1;
	while ( line < p && isspace( *p ))
		--p;

	char *endName = p + 1;

	int nameLength = endName - line;
	char *szName = new char[ nameLength + 1 ];

	strncpy( szName, line, nameLength );
	szName[ nameLength ] = '\0';

	// step over ':' and spaces
	char *szValue = colon + 1;
	while ( *szValue != '\0' && isspace( *szValue ))
		++szValue;

	string name = szName;
	string value = szValue;

	delete [] szName;

	return Set( name, value );
}


string NameValuePair::NameUp() const
{
	return ToUpper( Name());
}


string NameValuePair::ValueUp() const
{
	return ToUpper( Value());
}


string NameValuePair::NameLow() const
{
	return ToLower( Name());
}


string NameValuePair::ValueLow() const
{
	return ToLower( Value());
}


bool NameValuePair::IsValidString( const char *s, bool allowSpace /*=false*/)
{
	if ( s == NULL || s[0] == '\0' )
		return false;

	while ( *s != '\0' )
	{
		if ( !isgraph( *s ))
			if ( !allowSpace || *s != ' ' )
				return false;
		++s;
	}

	return true;
}


string NameValuePair::Test()
{
	string log = "Test NameValuePair\n";

	// test IsValidString()
	{
		if ( NameValuePair::IsValidString( NULL ))
			log += "NameValuePair(NULL) failed\n";

		if ( NameValuePair::IsValidString( "" ))
			log += "IsValidString(\"\") failed\n";

		if ( NameValuePair::IsValidString( " " ))
			log += "IsValidString(\" \") failed\n";

		if ( !NameValuePair::IsValidString( " ", true ))
			log += "IsValidString(\" \",true) failed\n";

		if ( !NameValuePair::IsValidString( ":" ))
			log += "IsValidString(\":\") failed\n";
	}

	// test Set()
	{
		NameValuePair nv;

		if ( !nv.Set( "a:b" ))
			log += "Set(\"a:b\") failed\n";

		if ( !nv.IsSet())
			log += "IsSet() failed\n";
// UNcomment
//		if ( nv.Set( " a:b" ))
//			log += "Set(\" a:b\") failed\n";
//
//		if ( nv.IsSet())
//			log += "IsSet() failed\n";

		if ( !nv.Set( "a :b" ))
			log += "Set(\"a :b\") failed\n";

		if ( !nv.Set( "a: b" ))
			log += "Set(\"a: b\") failed\n";

		if ( !nv.Set( "a:b " )) // spaces are allowed in values
			log += "Set(\"a:b \") failed\n";

		if ( nv.NameUp() != "A" || nv.ValueUp() != "B " )
			log += "NameUp() or ValueUp() failed\n";

		if ( !nv.Set( "XYZ:12A" ))
			log += "Set(\"XYZ:12A\") failed\n";

		if ( nv.NameLow() != "xyz" || nv.ValueLow() != "12a" )
			log += "NameLow() or ValueLow() failed\n";

		if ( ToUpper( "azAZ09!+" ) != "AZAZ09!+" )
			log += "ToUpper() failed\n";

		if ( ToLower( "azAZ09!+" ) != "azaz09!+" )
			log += "ToLower() failed\n";
	}

	{
		string a = "a";
		const string &aa = a;
		string b = "b";
		const string &bb = b;

		NameValuePair nv;
		if ( !nv.Set( aa, bb ))
			log += "Set() failed\n";

		a += "aa";
		b += "bbb";

		NameValuePair nv2;
		if ( !nv.Set( aa, bb ))
			log += "Set() failed\n";

		if ( nv2.Name() == a || nv2.Value() == b )
			log += "STL breaking down\n";
	}

	return log;
}

