// HttpUri.h: interface for the HttpUri class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPURI_H__BC5FC1B3_EA3C_424C_BC0B_499DE2621A39__INCLUDED_)
#define AFX_HTTPURI_H__BC5FC1B3_EA3C_424C_BC0B_499DE2621A39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>
#include<vector>


// My use of the term "URI" in "HttpUri" is incorrect.
// See the RFC notes below.


class HttpUri
{
public:
	void SetUrl( const string &url ) { _url = url; }

	void AddModifier( const string &url )
	{
		if ( url.empty()) { assert(false); return; }
		_modifiers.push_back( url );
	}

	string GetString() const;
	HttpUri() : _nextMod( 0 ) {}

	// I am using the default copy constructor and that has worked fine.
	// The compiler will force you to write an equality operator, if you use one.

	static string UrlEncode( const string &unencoded );
	static bool MustUrlEncode( char c );

	static const WORD _maxUrlLength;

	static string Test();
	static string TestDefCopyConstr();
	static string TestUrlEncode();

protected:
	string _url;
	vector<string> _modifiers;
	DWORD _nextMod;
};


/* RFC 1738: URL

BNF for HTTP URL:
httpurl        = "http://" hostport [ "/" hpath [ "?" search ]]
hpath          = hsegment *[ "/" hsegment ]
hsegment       = *[ uchar | ";" | ":" | "@" | "&" | "=" ]
search         = *[ uchar | ";" | ":" | "@" | "&" | "=" ]
hostport       = host [ ":" port ]

"... octets may be encoded by a character triplet consisting
of the character "%" followed by the two hexadecimal digits (from
"0123456789ABCDEF") which forming the hexadecimal value of the octet.
(The characters "abcdef" may also be used in hexadecimal encodings.)"

"Thus, only alphanumerics, the special characters "$-_.+!*'(),", and
reserved characters used for their reserved purposes may be used
unencoded within a URL."

Reserver characters:
"An HTTP URL takes the form:

	http://<host>:<port>/<path>?<searchpart>

where <host> and <port> are as described in Section 3.1. If :<port>
is omitted, the port defaults to 80.  No user name or password is
allowed.  <path> is an HTTP selector, and <searchpart> is a query
string. The <path> is optional, as is the <searchpart> and its
preceding "?". If neither <path> nor <searchpart> is present, the "/"
may also be omitted.

Within the <path> and <searchpart> components, "/", ";", "?" are
reserved.  The "/" character may be used within HTTP to designate a
hierarchical structure."

No mention of URIs in this document.
*/

/* RFC 2396: URI

Updates: 1808, 1738

"Uniform Resource Identifiers (URI) provide a simple and extensible
means for identifying a resource."

Why my use of "URI" is incorrect:
"A URI can be further classified as a locator, a name, or both.  The
term "Uniform Resource Locator" (URL) refers to the subset of URI
that identify resources via a representation of their primary access
mechanism (e.g., their network "location"), rather than identifying
the resource by name or by some other attribute(s) of that resource."

"An escaped octet is encoded as a character triplet, consisting of the
percent character "%" followed by the two hexadecimal digits
representing the octet code. For example, "%20" is the escaped
encoding for the US-ASCII space character."
*/

/* Maximum URL length

MSDN: INFO: Maximum URL Length Is 2,083 Characters in Internet Explorer (Q208427):

"Internet Explorer has a maximum uniform resource locator (URL) length of 2,083
characters, with a maximum path length of 2,048 characters. This limit applies
to both POST and GET request URLs. 

If you are using the GET method, you are limited to a maximum of 2,048 characters
(minus the number of characters in the actual path, of course). 

POST, however, is not limited by the size of the URL for submitting name/value
pairs, because they are transferred in the header and not the URL. 

RFC 2616, Hypertext Transfer Protocol -- HTTP/1.1 , does not specify any
requirement for URL length."
*/


// MSDN mentions an UrlEscape() function in <shlwapi.h>, but its not there!


#endif // !defined(AFX_HTTPURI_H__BC5FC1B3_EA3C_424C_BC0B_499DE2621A39__INCLUDED_)
