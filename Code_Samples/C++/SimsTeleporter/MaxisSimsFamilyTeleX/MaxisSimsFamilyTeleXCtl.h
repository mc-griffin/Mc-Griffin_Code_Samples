#if !defined(AFX_MAXISSIMSFAMILYTELEXCTL_H__795892B3_EABD_4343_AA30_15AFB4EAFA1C__INCLUDED_)
#define AFX_MAXISSIMSFAMILYTELEXCTL_H__795892B3_EABD_4343_AA30_15AFB4EAFA1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MaxisSimsFamilyTeleXCtl.h : Declaration of the CMaxisSimsFamilyTeleXCtrl ActiveX Control class.

#include "SimsConfig.h"
#include "SimsTeleporter.h"


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXCtrl : See MaxisSimsFamilyTeleXCtl.cpp for implementation.

class CMaxisSimsFamilyTeleXCtrl : public COleControl
{
	DECLARE_DYNCREATE(CMaxisSimsFamilyTeleXCtrl)

// Constructor
public:
	CMaxisSimsFamilyTeleXCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMaxisSimsFamilyTeleXCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CMaxisSimsFamilyTeleXCtrl();

	DECLARE_OLECREATE_EX(CMaxisSimsFamilyTeleXCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CMaxisSimsFamilyTeleXCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CMaxisSimsFamilyTeleXCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CMaxisSimsFamilyTeleXCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CMaxisSimsFamilyTeleXCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CMaxisSimsFamilyTeleXCtrl)
	afx_msg BOOL Initialize(LPCTSTR serverName);
	afx_msg BOOL StartDownload(LPCTSTR scriptPath, LPCTSTR assetId);
	afx_msg BSTR GetPercentComplete();
	afx_msg BSTR GetOutcome();
	afx_msg BOOL HasAlbum();
	afx_msg BOOL StartUpload(LPCTSTR scriptPath, LPCTSTR subject, LPCTSTR neighNum);
	afx_msg BOOL IsSimsThere();
	afx_msg BOOL IsUnleashedThere();
	afx_msg BOOL IsDeluxeThere();
	afx_msg BOOL IsUploadAllowed();
	afx_msg BOOL IsDownloadAllowed();
	afx_msg BOOL IsSuperstarThere();
	afx_msg BOOL IsMakinMagicThere();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

// Event maps
	//{{AFX_EVENT(CMaxisSimsFamilyTeleXCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CMaxisSimsFamilyTeleXCtrl)
	dispidInitialize = 1L,
	dispidStartDownload = 2L,
	dispidGetPercentComplete = 3L,
	dispidGetOutcome = 4L,
	dispidHasAlbum = 5L,
	dispidStartUpload = 6L,
	dispidIsSimsThere = 7L,
	dispidIsUnleashedThere = 8L,
	dispidIsDeluxeThere = 9L,
	dispidIsUploadAllowed = 10L,
	dispidIsDownloadAllowed = 11L,
	dispidIsSuperstarThere = 12L,
	dispidIsMakinMagicThere = 13L,
	//}}AFX_DISP_ID
	};

protected:
	SimsTeleporter _teleporter;

	bool _hasAlbum;

	// used to display face
	HBITMAP _bitmap;
	static const WORD _width;
	static const WORD _height;
	HDC _DC;
	bool _created;

	// Cache of Transport Data, for an explanation, see the end of this file
	string _scriptPath;
	string _assetId;
	string _subject;
	string _filePath;
	string _simsPath;

	static void ChangeNameToMM( string *famName );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAXISSIMSFAMILYTELEXCTL_H__795892B3_EABD_4343_AA30_15AFB4EAFA1C__INCLUDED)


/*
The Cache of Transport Data contains data that must persist from before a
call to ( SimsTeleporter::StartDownload() or SimsTeleporter::StartUpload())
until the completion of a call to SimsTeleporter::FinishTransport().

The problem that necessitated the Cache of Transport Data

In my tests for MaxisSimsFamilyTeleX, I defined a global string variable in
the JavaScript code.  I passed a pointer (an address) to that global variable
to my ActiveX control via StartDownload().  When my control dereferenced
that pointer, the pointer was invalid and the control crashed.  The pointers
is accessible for some unknown period of time before it becomes unreliable.

Some notes:
* I would prefer to pass a copy of the JavaScript global string into the
ActiveX control, but as far as I know the only way to get a string into an
ActiveX control is to pass a  pointer to it.  There probably is a way to pass
a copy, but I bet it involves COM marshalling, which is a lot of work.
* This technique of passing a pointer has worked for every teleporter up till
now, but for some reason, it breaks in this case.
* Darn it, that JavaScript variable is global, so its not supposed to become
invalid during execution.

The solution is still to pass the pointer to the ActiveX control, but the
absolute first thing that the control does is make a local copy of the
string via the pointer.  After that, always use the local copy, not the
pointer.  This seems to avoid the problem, although I can't guarantee that
the problem is 100% gone.  I'll have to go back to our earlier teleporters
and make this adjustment to ensure that the problem doesn't later show up there.

I probably could have solved this problem without creating a cache if I just
changed all the parameters in SimsTeleporter::StartDownload() and
SimsTeleporter::StartUpload() from references to constant strings to strings.
*/


