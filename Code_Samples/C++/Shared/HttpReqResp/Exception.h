// Exception.h: interface for the Exception class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXCEPTION_H__1C4B6C3A_1951_4B8E_8351_AED1C2811370__INCLUDED_)
#define AFX_EXCEPTION_H__1C4B6C3A_1951_4B8E_8351_AED1C2811370__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class Exception  
{
protected:
	string _technical; // programmer description of problem, write to log
	string _friendly; // description of problem for user
	DWORD _lastError; // GetLastError()
	string _lastErrorPhrase; // FormatMessage(_lastError)

public:
	Exception( const string &technical, string friendly = string());
	virtual ~Exception() {}

	virtual string GetExplanation( const string &title,
										string transactionId = string()) const;
	string GetTechnicalPhrase() const;
	string GetFriendlyPhrase() const;
	DWORD GetTheLastError() const; // "The" to prevent name conflict with Win32
	string GetLastErrorPhrase() const;
	string GetLastErrorAndPhrase() const;

	int TellUser( const string &title, string transactionId = string(),
					HWND wnd = NULL ) const;

	static string ExplainExtendedError( DWORD lastError );


protected:
	bool IsSet() const;

// s_nullLastErrorPhrase is useful because subclasses of
// Exception can tell if constructor wasn't able to fill _lastErrorPhrase.
//static const string s_nullLastErrorPhrase;

public:
	static string Test( bool testUi );
};


class WsaStartupException : public Exception
{
protected:
	DWORD _wsaLastError;

public:
	WsaStartupException( const string &technical, string friendly = string());

	virtual string GetExplanation( const string &title,
						string transactionId = string()) const;
};


class SocketException : public Exception
{
protected:
	DWORD _wsaLastError;

public:
	SocketException( const string &technical, string friendly = string());

	virtual string GetExplanation( const string &title,
						string transactionId = string()) const;
};

#endif // !defined(AFX_EXCEPTION_H__1C4B6C3A_1951_4B8E_8351_AED1C2811370__INCLUDED_)
