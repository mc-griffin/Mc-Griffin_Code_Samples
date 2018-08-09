// RequestLine.h: interface for the RequestLine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REQUESTLINE_H__534A4966_C448_4DAC_B3DE_AFE32B5F9819__INCLUDED_)
#define AFX_REQUESTLINE_H__534A4966_C448_4DAC_B3DE_AFE32B5F9819__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class RequestLine  
{
protected:
	string _method;
	string _url;

public:
	string GetString() const;

	string GetMethod() const { return _method; }
	bool SetMethod( const string &m );

	string GetUrl() const { return _url; }
	bool SetUrl( const string &u );

	bool IsSet() const;

	static const char s_get[];
	static const char s_put[];
	static const char s_post[];
	static const char s_head[];
	static const char s_http1_1[];

	static string Test();
};

#endif // !defined(AFX_REQUESTLINE_H__534A4966_C448_4DAC_B3DE_AFE32B5F9819__INCLUDED_)
