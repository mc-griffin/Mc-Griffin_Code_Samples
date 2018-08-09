// SimsTeleporter.h: interface for the SimsTeleporter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMSTELEPORTER_H__EE46E844_7A18_428D_9A7A_74CA007D741A__INCLUDED_)
#define AFX_SIMSTELEPORTER_H__EE46E844_7A18_428D_9A7A_74CA007D741A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <windows.h>
#include <assert.h>

#include "MaxisTeleporter.h"
#include "HttpUri.h"
#include "PubFile.h"


class SimsUploader : public MaxisTeleporter
{
public:
	SimsUploader( const string &title, const string &boundary,
					DWORD maxUploadSize );

	bool StartUpload( const string &scriptPath, const string &filePath,
						const string &subject );

	void Reset() { MaxisTeleporter::Reset(); _subject.erase(); }

protected:
	virtual bool SetReqBodyWithAsset( const string &famFilePath );
	bool AppendFilesImpliedInPubFile( RequestBody &body, const PubFile &pubFile );

	string _subject;

public:
	static bool IsMakinMagicFamily( const string &filePath );
};


class PklGetter : HttpReqResp
{
public:
	PklGetter( const string &title ) : HttpReqResp( title ), _gotPkl( false ) {}

	string GetPkl( const Server &server, const HttpUri &uri );

	bool GotPkl() const { return _gotPkl; }

	void Reset() { HttpReqResp::Reset(); _gotPkl = false; }

protected:
	bool _gotPkl;
};


class SimsDownloader
{
public:
	SimsDownloader( const string &title );
	bool Initialize( const Server *server );

	bool StartDownload( const string &scriptPath, const string &assetId,
						 const string &simsPath );
	string GetPercentComplete() const;
	string FinishTransport();
	bool WasSuccessful() const;
	bool WasCanceled() const;

	void Reset();

	const string _title;

protected:
	bool DownloadFamily( const string &scriptPath, const string &assetId,
							const string &simsPath );
	bool DownloadFile( const string &relativeFilePath, const HttpUri &uri,
							const string &simsPath );
	static unsigned int WINAPI DownloadThread( LPVOID param );

	PklGetter _getPkl;

	DWORD _filesToDownloadCount; // excluding PKL file
	DWORD _downloadedFilesCount;
	MaxisTeleporter *_currentDownloader;

	DWORD _totalBytesUnencoded; // we get this from the PKL file
	DWORD _totalBytes; // _totalBytesUnencoded base64 encoded
	DWORD _bytesReceived;

	bool _sendDone;
	bool _receiveDone;

	HANDLE _downloadThread;
	unsigned int _threadId; // for logging

	// used to set percent done to 100 in the case of an error or cancel
	bool _errorOut;
	bool _canceled;

	const Server *_server;
};


class SimsTeleporter // : public MaxisTeleporter1
{
public:
	SimsTeleporter();

	// begin MaxisTeleporter1
	bool Initialize( const string &serverName );

	bool StartDownload( const string &scriptPath, const string &assetId,
							const string &simsInstallPath );
	bool StartUpload( const string &scriptPath, const string &filePath,
							const string &subject );

	string GetPercentComplete() const;

	virtual string FinishTransport( const string &simsPath );

	void Reset();
	// end MaxisTeleporter1

	static string Test( bool testNetwork, string serverName, HINSTANCE inst );
	static string TestUpload( string serverName );
	static string TestDownload( string serverName, HINSTANCE inst );
	static void TestErrors( string serverName );

protected:
	enum Direction { Up = 1, Down };
	Direction _direction;

	SimsUploader _uploader;
	SimsDownloader _downloader;

	Server _server;

public:
	static bool IsLegalFamilyName( const string &familyName,
					DWORD *indexOfIllegalChar = NULL );
	static bool IsDefaultFamilyName( const string &fileName );
};


#endif // !defined(AFX_SIMSTELEPORTER_H__EE46E844_7A18_428D_9A7A_74CA007D741A__INCLUDED_)
