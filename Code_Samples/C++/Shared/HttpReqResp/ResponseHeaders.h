// ResponseHeaders.h: interface for the ResponseHeaders class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESPONSEHEADERS_H__8686EE3A_9493_40D1_BA70_A9B4210D886A__INCLUDED_)
#define AFX_RESPONSEHEADERS_H__8686EE3A_9493_40D1_BA70_A9B4210D886A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NameValuePair.h"
#include "Error.h"

// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <vector>


// This class is potentially vulnerable to thread problems.
// However, I am convinced that these classes are thread safe now (May 23, 2002).
//
// The thread safety dangers can be organized by data type.  There
// are 2 data types used by this class that are vulnerable to thread safety
// problems: string and vector, both STL.
//
// 1. vector thread safety
// ResponseHeaders is a vector.  The danger is that while the main thread is
// querying the response headers, the worker (transport) thread is altering
// those headers.  Specifically, if an HttpReqResp launches an HTTP transaction
// with 2 threads (HRR_DUAL),
// * the worker thread receives the HTTP response and fills
//   HttpReqResp::_response and HttpReqResp::._response._headers, and
// * the main thread peppers HttpReqResp::_response and its _headers with
//   GetPercent*() calls, which call GetExpectedContentLength() or Find*().
//
// There are 2 reasons why I think that the current use of vectors is thread safe.
// * Although ResponseHeaders has new headers appended to the end of it,
//   no headers are deleted.
// * I access the vector elements with an index, not with an iterator.
//   Thus, I don't need to worry that an iterator object becomes out-of-sync
//   with the vector when that vector is changed.
//
// ASSUMPTION: vector push_back() is thread safe under these conditions:
//   thread1 can execute "v.push_back(x);" at the same time that thread2
//   accesses v as follows:
//		for ( i=0; i < v.size(); ++i ) { assess v[i]; }
//   and thread2 will find v either with the value it had before or after
//   the push_back().
//   This assumption is invoked in Find*().
//
//	2. string thread safety
//  To facilitate this thread safety analysis, I introduced a cache of variables.
//  Ironically, I may have made the code vulnerable to thread safety problems
//  when those variables are accessed or changed.  But I think I'm safe because:
//  ASSUMPTION: string assignment is thread safe under these conditions:
//    thread1 can execute "s1 = s2;" at the same time that thread2 accesses
//    the value of s1 and thread2 will find that s1 is either s2 or the
//    value s1 had before the assignment statement.
//    This assumption is invoked in UpdateCache().
//
// I NEED TO WRITE TESTS for these assumptions, but I don't have time now.
//
//
// The worst case is that the main thread is stepping through the vector using
// indices, while the worker thread is adding headers to the end of the vector.
// The worst that happens in this case is that the main thread may not notice
// the newest headers appended to the vector.  But that's fine, because the
// threads do not interfere with eachother.


// The next step in ensuring thread safety is probably to use a MUTEX
// on Append( const NameValuePair &nv ) and Find*().  While one thread is
// appending, the other thread cannot find, and vice versa.  But that's not
// necessary now.



class ResponseHeaders : vector< NameValuePair >
{
public:
	int AppendLines( const char *headers );
	DWORD Append( const NameValuePair &nv );
	DWORD Append( const char *line );

	string GetString() const;
	WORD Count() const;

	DWORD Find( const string &name, DWORD start = 0,
					bool caseSensitive = false ) const;
	DWORD FindCount( const string &name, bool caseSensitive = false ) const;
	string FindOnly( const string &name, bool caseSensitive = false ) const;
	// caseSensitive defaults to false since most HTTP is case insensitive

	string GetValueAt( DWORD index ) const;

	DWORD SetCookies( const string &serverName ) const;

	// accessors to cache of variables of interest
	string GetDownloadedFileName() const;
	bool GetContentLength( string *len ) const;
	// string GetTransferEncoding() const;

	void Reset();

	static string Test( const string &serverName, const string &serverAddress );

	// convenience functions
	int AppendLines( const string &headers );
	DWORD Append( const string &line );

protected:
	bool UpdateCache( const NameValuePair &nv );
	static string ExtractDownloadedFileName( const string &contentDisposition );

	// cache of variables of interest:
	string _downloadedFileName; // from Content-Disposition
	string _contentLength; // from Content-Length
	string _transferEncoding; // from Transfer-Encoding
	//
	// The cache was created to minimize the calls to Find*().
	// Many calls to Find*() made analyzing threading/reentrant issues difficult.

	static DWORD GetLongestLineLength( const string &headers );
};

#endif // !defined(AFX_RESPONSEHEADERS_H__8686EE3A_9493_40D1_BA70_A9B4210D886A__INCLUDED_)
