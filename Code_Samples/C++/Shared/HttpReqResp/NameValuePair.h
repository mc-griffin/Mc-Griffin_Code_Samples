// NameValuePair.h: interface for the NameValuePair class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NAMEVALUEPAIR_H__690E9A9C_D6F4_446E_A5EF_6C448977E207__INCLUDED_)
#define AFX_NAMEVALUEPAIR_H__690E9A9C_D6F4_446E_A5EF_6C448977E207__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct NameValuePair : protected pair< string, string > // by value semantics
{
	bool Set( const char *name, const char *value );
	bool Set( const string &name, const string &value );
	bool Set( const char *line );

	const string &Name() const { return first; }
	const string &Value() const { return second; }

	string GetString() const { return Name() + ": " + Value(); }

	bool IsSet() const { return _set; }

	NameValuePair() : _set(false) {}

	static bool IsValidString( const char *s, bool allowSpaces = false );

	NameValuePair( const NameValuePair &nv );
	NameValuePair &operator=( const NameValuePair &nv );

	string NameUp() const;
	string ValueUp() const;
	string NameLow() const;
	string ValueLow() const;

protected:
	bool _set;

public:
	static string Test();
};

#endif // !defined(AFX_NAMEVALUEPAIR_H__690E9A9C_D6F4_446E_A5EF_6C448977E207__INCLUDED_)
