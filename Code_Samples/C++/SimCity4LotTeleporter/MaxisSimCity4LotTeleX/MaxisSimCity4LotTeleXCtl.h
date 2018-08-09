#if !defined(AFX_MAXISSIMCITY4LOTTELEXCTL_H__8B7BFC0D_0E08_49C2_89C1_0AA777316B7A__INCLUDED_)
#define AFX_MAXISSIMCITY4LOTTELEXCTL_H__8B7BFC0D_0E08_49C2_89C1_0AA777316B7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MaxisSimCity4LotTeleXCtl.h : Declaration of the CMaxisSimCity4LotTeleXCtrl ActiveX Control class.

#include <SimCity4LotTeleporter.h>
#include <SimCity4LotBrowser.h>
#include <SimCity4Lot.h>

/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXCtrl : See MaxisSimCity4LotTeleXCtl.cpp for implementation.

class CMaxisSimCity4LotTeleXCtrl : public COleControl
{
	DECLARE_DYNCREATE(CMaxisSimCity4LotTeleXCtrl)

// Constructor
public:
	CMaxisSimCity4LotTeleXCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMaxisSimCity4LotTeleXCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CMaxisSimCity4LotTeleXCtrl();

	DECLARE_OLECREATE_EX(CMaxisSimCity4LotTeleXCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CMaxisSimCity4LotTeleXCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CMaxisSimCity4LotTeleXCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CMaxisSimCity4LotTeleXCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CMaxisSimCity4LotTeleXCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CMaxisSimCity4LotTeleXCtrl)
	afx_msg BOOL Initialize(LPCTSTR serverName);
	afx_msg BOOL StartUpload(LPCTSTR scriptPath, LPCTSTR lotPath);
	afx_msg BOOL StartDownload(LPCTSTR scriptPath, LPCTSTR assetId);
	afx_msg BSTR GetPercentComplete();
	afx_msg BSTR GetOutcome();
	afx_msg void Reset();
	afx_msg BOOL IsSimCity4There();
	afx_msg BSTR GetSimCity4PluginPath();
	afx_msg BOOL SelectDir(LPCTSTR dirPath);
	afx_msg BSTR GetFirstDir();
	afx_msg BSTR GetNextDir();
	afx_msg BSTR GetFirstFile();
	afx_msg BSTR GetNextFile();
	afx_msg BOOL SelectLot(LPCTSTR lotPath);
	afx_msg BSTR GetName();
	afx_msg long GetZoneCompatibility();
	afx_msg long GetWealthCompatibility();
	afx_msg long GetUserLotType();
	afx_msg long GetWidth();
	afx_msg long GetDepth();
	afx_msg long GetPropCount();
	afx_msg long GetNumDependantPlugins();
	afx_msg long GetPurposeCompatibility();
	afx_msg BSTR GetPlugin(long index);
	afx_msg long GetGroup();
	afx_msg long GetGrowthStage();
	afx_msg BSTR GetThumbnailPath();
	afx_msg long GetErrorCode();
   afx_msg void SetTitle (LPCTSTR title);
   afx_msg void SetDescription (LPCTSTR description);
	afx_msg BOOL IsReasonableCharacterSet();
	afx_msg BOOL IsRushHourThere();
	afx_msg BOOL IsDeluxeThere();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

// Event maps
	//{{AFX_EVENT(CMaxisSimCity4LotTeleXCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CMaxisSimCity4LotTeleXCtrl)
	dispidInitialize = 1L,
	dispidStartUpload = 2L,
	dispidStartDownload = 3L,
	dispidGetPercentComplete = 4L,
	dispidGetOutcome = 5L,
	dispidReset = 6L,
	dispidIsSimCity4There = 7L,
	dispidGetSimCity4PluginPath = 8L,
	dispidSelectDir = 9L,
	dispidGetFirstDir = 10L,
	dispidGetNextDir = 11L,
	dispidGetFirstFile = 12L,
	dispidGetNextFile = 13L,
	dispidSelectLot = 14L,
	dispidGetName = 15L,
	dispidGetZoneCompatibility = 16L,
	dispidGetWealthCompatibility = 17L,
	dispidGetUserLotType = 18L,
	dispidGetWidth = 19L,
	dispidGetDepth = 20L,
	dispidGetPropCount = 21L,
	dispidGetNumDependantPlugins = 22L,
	dispidGetPurposeCompatibility = 23L,
	dispidGetPlugin = 24L,
	dispidGetGroup = 25L,
	dispidGetGrowthStage = 26L,
	dispidGetThumbnailPath = 27L,
	dispidGetErrorCode = 28L,
	dispidSetTitle = 29L,
	dispidSetDescription = 30L,
	dispidIsReasonableCharacterSet = 31L,
	dispidIsRushHourThere = 32L,
	dispidIsDeluxeThere = 33L,
	//}}AFX_DISP_ID
	};
protected:
	SimCity4LotBrowser _lotBrowser;
	SimCity4Lot _lot;
	SimCity4LotTeleporter _lotTeleporter;
	Server _server;

	// APP we cache these paths so we can delete after successful upload
	string _thumbnailPath;
	string _xmlPath; 
	// these should be set before StartUpload is called
	string _title;
	string _description;

	// used to display face
	HBITMAP _bitmap;
	static const WORD _width;
	static const WORD _height;
	HDC _DC;
	bool _created;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAXISSIMCITY4LOTTELEXCTL_H__8B7BFC0D_0E08_49C2_89C1_0AA777316B7A__INCLUDED)
