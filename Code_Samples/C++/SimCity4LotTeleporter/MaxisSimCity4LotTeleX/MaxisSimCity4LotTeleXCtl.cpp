// MaxisSimCity4LotTeleXCtl.cpp : Implementation of the CMaxisSimCity4LotTeleXCtrl ActiveX Control class.

#include "stdafx.h"
#include "MaxisSimCity4LotTeleX.h"
#include "MaxisSimCity4LotTeleXCtl.h"
#include "MaxisSimCity4LotTeleXPpg.h"
#include "SafeScripting.h"
#include "FileUtils.h"
#include "Utilities.h"
#include "SimCity4Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const string s_title = "Maxis SimCity4Lot Teleporter";
// should match the dimensions of IDB_FACE bitmap
const WORD CMaxisSimCity4LotTeleXCtrl::_width( 50 );
const WORD CMaxisSimCity4LotTeleXCtrl::_height( 50 );

IMPLEMENT_DYNCREATE(CMaxisSimCity4LotTeleXCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CMaxisSimCity4LotTeleXCtrl, COleControl)
	//{{AFX_MSG_MAP(CMaxisSimCity4LotTeleXCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CMaxisSimCity4LotTeleXCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CMaxisSimCity4LotTeleXCtrl)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "Initialize", Initialize, VT_BOOL, VTS_BSTR)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "StartUpload", StartUpload, VT_BOOL, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "StartDownload", StartDownload, VT_BOOL, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetPercentComplete", GetPercentComplete, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetOutcome", GetOutcome, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "Reset", Reset, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "IsSimCity4There", IsSimCity4There, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetSimCity4PluginPath", GetSimCity4PluginPath, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "SelectDir", SelectDir, VT_BOOL, VTS_BSTR)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetFirstDir", GetFirstDir, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetNextDir", GetNextDir, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetFirstFile", GetFirstFile, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetNextFile", GetNextFile, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "SelectLot", SelectLot, VT_BOOL, VTS_BSTR)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetName", GetName, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetZoneCompatibility", GetZoneCompatibility, VT_I4, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetWealthCompatibility", GetWealthCompatibility, VT_I4, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetUserLotType", GetUserLotType, VT_I4, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetWidth", GetWidth, VT_I4, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetDepth", GetDepth, VT_I4, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetPropCount", GetPropCount, VT_I4, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetNumDependantPlugins", GetNumDependantPlugins, VT_I4, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetPurposeCompatibility", GetPurposeCompatibility, VT_I4, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetPlugin", GetPlugin, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetGroup", GetGroup, VT_I4, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetGrowthStage", GetGrowthStage, VT_I4, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetThumbnailPath", GetThumbnailPath, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "GetErrorCode", GetErrorCode, VT_I4, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "SetTitle", SetTitle, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "SetDescription", SetDescription, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "IsReasonableCharacterSet", IsReasonableCharacterSet, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "IsRushHourThere", IsRushHourThere, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CMaxisSimCity4LotTeleXCtrl, "IsDeluxeThere", IsDeluxeThere, VT_BOOL, VTS_NONE)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CMaxisSimCity4LotTeleXCtrl, COleControl)
	//{{AFX_EVENT_MAP(CMaxisSimCity4LotTeleXCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CMaxisSimCity4LotTeleXCtrl, 1)
	PROPPAGEID(CMaxisSimCity4LotTeleXPropPage::guid)
END_PROPPAGEIDS(CMaxisSimCity4LotTeleXCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CMaxisSimCity4LotTeleXCtrl, "MAXISSIMCITY4LOTTELEX.MaxisSimCity4LotTeleXCtrl.1",
	0xbc18e6df, 0xbe57, 0x4580, 0x93, 0xe8, 0xf2, 0x28, 0xf9, 0xa1, 0x33, 0xaa)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CMaxisSimCity4LotTeleXCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DMaxisSimCity4LotTeleX =
		{ 0xea3d4bd3, 0x9c2a, 0x49bf, { 0x8c, 0x6a, 0xfc, 0x84, 0x45, 0x6, 0x8, 0x6c } };
const IID BASED_CODE IID_DMaxisSimCity4LotTeleXEvents =
		{ 0x6ebf07c1, 0x7f0e, 0x4ed8, { 0xb9, 0x4, 0xe6, 0xd5, 0x28, 0xd9, 0x28, 0x2f } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwMaxisSimCity4LotTeleXOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CMaxisSimCity4LotTeleXCtrl, IDS_MAXISSIMCITY4LOTTELEX, _dwMaxisSimCity4LotTeleXOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXCtrl::CMaxisSimCity4LotTeleXCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CMaxisSimCity4LotTeleXCtrl

BOOL CMaxisSimCity4LotTeleXCtrl::CMaxisSimCity4LotTeleXCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.
	if ( bRegister )
	{
		BOOL regResult =  AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_MAXISSIMCITY4LOTTELEX,
			IDB_MAXISSIMCITY4LOTTELEX,
			afxRegInsertable | afxRegApartmentThreading,
			_dwMaxisSimCity4LotTeleXOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);

		// Borrowed from original teleporter:

		// mark as safe for scripting -- failure OK
		HRESULT hr = CreateComponentCategory(CATID_SafeForScripting, 
					L"Controls that are safely scriptable");
		
		if (SUCCEEDED(hr))
			// only register if category exists
			RegisterCLSIDInCategory(m_clsid, CATID_SafeForScripting);
			// don't care if this call fails
		
		// mark as safe for data initialization
		hr = CreateComponentCategory(CATID_SafeForInitializing, 
					L"Controls safely initializable from persistent data");
		
		if (SUCCEEDED(hr))
			// only register if category exists
			RegisterCLSIDInCategory(m_clsid, CATID_SafeForInitializing);
			// don't care if this call fails

		return regResult;
	}
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
	
	/*
	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_MAXISSIMCITY4LOTTELEX,
			IDB_MAXISSIMCITY4LOTTELEX,
			afxRegApartmentThreading,
			_dwMaxisSimCity4LotTeleXOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
		*/
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXCtrl::CMaxisSimCity4LotTeleXCtrl - Constructor

CMaxisSimCity4LotTeleXCtrl::CMaxisSimCity4LotTeleXCtrl()
{
	try {
		InitializeIIDs(&IID_DMaxisSimCity4LotTeleX, &IID_DMaxisSimCity4LotTeleXEvents);
		Logger::Start( "SOFTWARE\\Maxis\\SimCity 4", "MaxisSimCity4LotTeleX.txt" );		
		_bitmap = NULL;
		_DC = NULL;
		_created = false;
	}
	catch ( ... ) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::constructor:  unhandled exception!" );
	}	
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXCtrl::~CMaxisSimCity4LotTeleXCtrl - Destructor

CMaxisSimCity4LotTeleXCtrl::~CMaxisSimCity4LotTeleXCtrl()
{
	try {
		// wrap up the open city, if it exists
		_lot.Close();
		if ( (! _thumbnailPath.empty()) && FileExists(_thumbnailPath))
			DeleteTheFile(_thumbnailPath);
	}
	catch ( ... ) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::destructor:  unhandled exception!" );
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXCtrl::OnDraw - Drawing function

void CMaxisSimCity4LotTeleXCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	try {
		if ( !_created )
		return;

		HDC hDC = pdc->m_hDC;
		if ( hDC == NULL )
			return;

		if ( _DC && _bitmap )
		{
			HBITMAP hOldBitmap = (HBITMAP) ::SelectObject( _DC, _bitmap);

			CRect rect;
			GetClientRect( &rect );

			::StretchBlt( hDC, 0, 0, rect.Width(), rect.Height(),
							_DC, 0, 0, _width, _height, SRCCOPY );

			::SelectObject( _DC, hOldBitmap );
		}
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::OnDraw:  unhandled exception!" );
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXCtrl::DoPropExchange - Persistence support

void CMaxisSimCity4LotTeleXCtrl::DoPropExchange(CPropExchange* pPX)
{
	try {
		ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
		COleControl::DoPropExchange(pPX);
	}
	catch ( ... ) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::DoPropExchange:  unhandled exception!" );
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXCtrl::OnResetState - Reset control to default state

void CMaxisSimCity4LotTeleXCtrl::OnResetState()
{
	try {
	
		COleControl::OnResetState();  // Resets defaults found in DoPropExchange
		
		_lotTeleporter.Reset();

		_xmlPath.erase();
		_thumbnailPath.erase();
		_title.erase();
		_description.erase();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::OnResetState:  unhandled exception!" );
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXCtrl message handlers

BOOL CMaxisSimCity4LotTeleXCtrl::Initialize(LPCTSTR serverName) 
{
	try {
		bool success = _server.SetNameAddress( serverName );
		if ( success ) {
			success = _lotTeleporter.Initialize( &_server );
		}
		return success;
	}
 	catch ( Exception &e ) {
 		e.TellUser( s_title );
 		return FALSE;
 	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::Initialize:  unhandled exception!" );
		return FALSE;
	}
}

BOOL CMaxisSimCity4LotTeleXCtrl::StartUpload(LPCTSTR scriptPath, LPCTSTR lotPath) 
{
	try {
		// check input parameters, all other checks done in _cityTeleporter.StartUploadCity()
		if ( scriptPath == NULL || lotPath == NULL )
		{ assert(false); return FALSE; }
   
		if (! FileExists(lotPath) )
		{ assert(false); return FALSE; }

		if (! _lot.Init(lotPath) ) 
		{ assert(false); return FALSE; }

      _lot.SetTitle(_title.c_str());
      _lot.SetDescription(_description.c_str());

		// create XML file
		if (! _lot.CreateXMLFile()) // extracts "lotName".xml to tmp folder
		{ assert(false); return FALSE; }

		// extract thumbnail
		if (! _lot.ExtractLotThumbnail()) // extracts "lotName".png to tmp folder
		{ assert(false); return FALSE; }

		return _lotTeleporter.StartUpload( scriptPath, lotPath );
	}
	catch ( ... ) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::StartUpload:  unhandled exception!" );
		return FALSE;
	}
}

BOOL CMaxisSimCity4LotTeleXCtrl::StartDownload(LPCTSTR scriptPath, LPCTSTR assetId) 
{
	try {
		if (! (scriptPath && assetId ))
		{
			LOG( "CMaxisSimCity4LotTeleXCtrl::StartDownloadLot:  a NULL parameter!" );
			assert(false);
			return FALSE;
		}


		return _lotTeleporter.StartDownload( scriptPath, assetId );
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::StartDownloadCity:  unhandled exception!" );
		return FALSE;
	}
}

BSTR CMaxisSimCity4LotTeleXCtrl::GetPercentComplete() 
{
	try {
		CString percent = _lotTeleporter.GetPercentComplete().c_str();
		return percent.AllocSysString();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetPercentComplete:  unhandled exception!" );
		return NULL;
	}
}

BSTR CMaxisSimCity4LotTeleXCtrl::GetOutcome() 
{
	try {
		CString outcome;

		// APP if download directory doesn't exist, config will return error
		SimCity4Config config;
		bool init = config.Initialize();
		if ( !init )
		{
			assert(false);
			return outcome.AllocSysString();
		}

      // downloaded lots belong in plugin directory
		string downloadDirectory = config.GetPluginPath();
		// shave off ending backslash to prevent an assert in teleporter
		if ( HasEndingBackSlash( downloadDirectory ))
			downloadDirectory.resize( downloadDirectory.size() - 1 );

        // don't allow renaming of files (no saveAs)
		SfpConflictAlert conflictAlert;
		conflictAlert.SetTitle( s_title );
		conflictAlert.SetParent( this->GetSafeHwnd());

		string code = _lotTeleporter.FinishTransport( downloadDirectory, conflictAlert );
		outcome = MaxisTeleporter1::GetOutcomePhrase( code ).c_str();

		// APP delete xmlPath and thumbnailPath files that got created in tmp/
		if ( (! _xmlPath.empty()) && FileExists(_xmlPath))
			DeleteTheFile(_xmlPath);
		if ( (! _thumbnailPath.empty()) && FileExists(_thumbnailPath))
			DeleteTheFile(_thumbnailPath);

		return outcome.AllocSysString();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetOutcome:  unhandled exception!" );
		return NULL;
	}
}

void CMaxisSimCity4LotTeleXCtrl::Reset() 
{
	try {
		OnResetState();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::Reset:  unhandled exception!" );
	}

}

BOOL CMaxisSimCity4LotTeleXCtrl::IsSimCity4There() 
{
	try {
		SimCity4Config sim;
		if ( !sim.Initialize())
			return FALSE;
		return sim.IsSimCity4There();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::IsSimCity4There:  unhandled exception!" );
		return FALSE;
	}
}

BSTR CMaxisSimCity4LotTeleXCtrl::GetSimCity4PluginPath() 
{
	try {
		CString strResult;
		SimCity4Config sim;
		if ( !sim.Initialize())
			return strResult.AllocSysString();
		strResult = MakeUnixPath( sim.GetPluginPath()).c_str();
		return strResult.AllocSysString();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetSimCity4PluginPath:  unhandled exception!" );
		return NULL;
	}
	return NULL;
}

BOOL CMaxisSimCity4LotTeleXCtrl::SelectDir(LPCTSTR dirPath) 
{
	try {
		if ( !_lotBrowser.SelectDir( MakeWinPath( dirPath )))
			return FALSE;
		return TRUE;
	} 
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::SelectDir:  unhandled exception!" );
		return FALSE;
	}
	
	return FALSE;
}

BSTR CMaxisSimCity4LotTeleXCtrl::GetFirstDir() 
{
	try {
		CString strResult = MakeUnixPath( _lotBrowser.GetFirstDir()).c_str();
		return strResult.AllocSysString();
	} 
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetFirstDir:  unhandled exception!" );
		return NULL;
	}
}

BSTR CMaxisSimCity4LotTeleXCtrl::GetNextDir() 
{
	try {
		CString strResult = MakeUnixPath( _lotBrowser.GetNextDir()).c_str();
		return strResult.AllocSysString();
	} 
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetNextDir:  unhandled exception!" );
		return NULL;
	}
}

BSTR CMaxisSimCity4LotTeleXCtrl::GetFirstFile() 
{
	try {
		CString strResult = MakeUnixPath( _lotBrowser.GetFirstFile()).c_str();
		return strResult.AllocSysString();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetFirstFile:  unhandled exception!" );
		return NULL;
	}
}

BSTR CMaxisSimCity4LotTeleXCtrl::GetNextFile() 
{
	try {
		CString strResult = MakeUnixPath( _lotBrowser.GetNextFile()).c_str();
		return strResult.AllocSysString();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetNextFile:  unhandled exception!" );
		return NULL;
	}
}

BOOL CMaxisSimCity4LotTeleXCtrl::SelectLot(LPCTSTR lotPath) 
{
	try {
		// first wrap up the previous lot, if it exists
		_lot.Close();
		if ( (! _thumbnailPath.empty()) && FileExists(_thumbnailPath))
			DeleteTheFile(_thumbnailPath);

		// now open the new lot
		bool success = _lot.Init( (MakeWinPath( lotPath )).c_str() );
		if (success) {
			_lot.ExtractLotThumbnail();
			_thumbnailPath = _lot.GetThumbnailPath();		
		} 

		return success;
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::SelectLot:  unhandled exception!" );
		return FALSE;
	}
}

BSTR CMaxisSimCity4LotTeleXCtrl::GetName() 
{
	try {
		CString strResult = _lot.GetName();
		return strResult.AllocSysString();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetName:  unhandled exception!" );
		return NULL;
	}
}

long CMaxisSimCity4LotTeleXCtrl::GetZoneCompatibility() 
{
	try {
		return _lot.GetZoneCompatibility();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetZoneCompatibility:  unhandled exception!" );
		return 0;
	}
}

long CMaxisSimCity4LotTeleXCtrl::GetWealthCompatibility() 
{
	try {
		return _lot.GetWealthCompatibility();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetZoneCompatibility:  unhandled exception!" );
		return 0;
	}
}

long CMaxisSimCity4LotTeleXCtrl::GetUserLotType() 
{
	try {
		return _lot.GetUserLotType();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetUserLotType:  unhandled exception!" );
		return 0;
	}
}

long CMaxisSimCity4LotTeleXCtrl::GetWidth() 
{
	try {
		return _lot.GetWidth();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetWidth:  unhandled exception!" );
		return 0;
	}
}

long CMaxisSimCity4LotTeleXCtrl::GetDepth() 
{
	try {
		return _lot.GetDepth();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetDepth:  unhandled exception!" );
		return 0;
	}
}

long CMaxisSimCity4LotTeleXCtrl::GetPropCount() 
{
	try {
		return _lot.GetPropCount();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetUserLotType:  unhandled exception!" );
		return 0;
	}
}

long CMaxisSimCity4LotTeleXCtrl::GetNumDependantPlugins() 
{
	try {
		return _lot.GetNumDependantPlugins();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetNumDependantPlugins:  unhandled exception!" );
		return 0;
	}
}

long CMaxisSimCity4LotTeleXCtrl::GetPurposeCompatibility() 
{
	try {
		return _lot.GetPurposeCompatibility();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetPurposeCompatibility:  unhandled exception!" );
		return 0;
	}
}

BSTR CMaxisSimCity4LotTeleXCtrl::GetPlugin(long index) 
{
	try {
		CString strResult = _lot.GetPlugin(index);
		return strResult.AllocSysString();

	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetPlugin:  unhandled exception!" );
		return 0;
	}
}

long CMaxisSimCity4LotTeleXCtrl::GetGroup() 
{
	try {
		return _lot.GetGroup();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetGroup:  unhandled exception!" );
		return 0;
	}
}

long CMaxisSimCity4LotTeleXCtrl::GetGrowthStage() 
{
	try {
		return _lot.GetGrowthStage();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetGrowthStage:  unhandled exception!" );
		return 0;
	}
}

BSTR CMaxisSimCity4LotTeleXCtrl::GetThumbnailPath() 
{
	try {
		CString strResult = _lot.GetThumbnailPath();
		return strResult.AllocSysString();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetThumbnailPath:  unhandled exception!" );
		return NULL;
	}
}

long CMaxisSimCity4LotTeleXCtrl::GetErrorCode() 
{
	try {
		return _lot.GetErrCode();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetErrorCode:  unhandled exception!" );
		return 0;
	}
}

void CMaxisSimCity4LotTeleXCtrl::SetTitle(LPCTSTR title) 
{
	try {
		_title = title;
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::Set:  unhandled exception!" );
	}
}

void CMaxisSimCity4LotTeleXCtrl::SetDescription(LPCTSTR description) 
{
	try {
      _description = description;
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetGrowthStage:  unhandled exception!" );
	}
}

int CMaxisSimCity4LotTeleXCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	try {
		if ( COleControl::OnCreate(lpCreateStruct) == -1 )
			return -1;
		
		HDC	tempDC = NULL;
		_bitmap = ::LoadBitmap( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_FACE));
		if ( _bitmap == NULL )
		{
			assert(false);
			return -1;
		}

		HDC hDC = ::GetDC( GetSafeHwnd());
		if ( hDC == NULL )
		{
			assert(false);
			return -1;
		}

		_DC = ::CreateCompatibleDC( hDC );
		if ( _DC == NULL )
		{
			assert(false);
			return -1;
		}

		::ReleaseDC( GetSafeHwnd(), hDC );

		_created = true;
		
		return 0;
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::OnCreate:  unhandled exception!" );
		return 1;
	}
}

void CMaxisSimCity4LotTeleXCtrl::OnDestroy() 
{
	try {
		_created = false;

		if ( _bitmap )
			::DeleteObject( _bitmap );

		if ( _DC )
			::DeleteObject( _DC );

		COleControl::OnDestroy();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::OnDestroy:  unhandled exception!" );
	}
	
}

BOOL CMaxisSimCity4LotTeleXCtrl::IsReasonableCharacterSet() 
{
	try {
		SimCity4Config sim;
		if ( sim.Initialize() && sim.IsReasonableCharSet() )
			return TRUE;
		else
			return FALSE;
			
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::GetSimCity4PluginPath:  unhandled exception!" );
		return FALSE;
	}
}

BOOL CMaxisSimCity4LotTeleXCtrl::IsRushHourThere() 
{
	try {
		SimCity4Config sim;
		if ( !sim.Initialize())
			return FALSE;
		return sim.IsRushHourThere();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::IsRushHourThere:  unhandled exception!" );
		return FALSE;
	}
}

BOOL CMaxisSimCity4LotTeleXCtrl::IsDeluxeThere() 
{
	try {
		SimCity4Config sim;
		if ( !sim.Initialize())
			return FALSE;
		return sim.IsDeluxeThere();
	}
	catch (...) {
		LOG( "CMaxisSimCity4LotTeleXCtrl::IsDeluxeThere:  unhandled exception!" );
		return FALSE;
	}
}
