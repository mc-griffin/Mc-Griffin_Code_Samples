// MaxisSimsFamilyTeleXCtl.cpp : Implementation of the CMaxisSimsFamilyTeleXCtrl ActiveX Control class.

#include "stdafx.h"
#include "MaxisSimsFamilyTeleX.h"
#include "MaxisSimsFamilyTeleXCtl.h"
#include "MaxisSimsFamilyTeleXPpg.h"
#include "SafeScripting.h"
#include "FileUtils.h"
#include "Utilities.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// should match the dimensions of vacation.bmp
const WORD CMaxisSimsFamilyTeleXCtrl::_width( 50 );
const WORD CMaxisSimsFamilyTeleXCtrl::_height( 50 );

static const string s_title = "Maxis Sims Family Teleporter";


IMPLEMENT_DYNCREATE(CMaxisSimsFamilyTeleXCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CMaxisSimsFamilyTeleXCtrl, COleControl)
	//{{AFX_MSG_MAP(CMaxisSimsFamilyTeleXCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CMaxisSimsFamilyTeleXCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CMaxisSimsFamilyTeleXCtrl)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "Initialize", Initialize, VT_BOOL, VTS_BSTR)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "StartDownload", StartDownload, VT_BOOL, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "GetPercentComplete", GetPercentComplete, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "GetOutcome", GetOutcome, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "HasAlbum", HasAlbum, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "StartUpload", StartUpload, VT_BOOL, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "IsSimsThere", IsSimsThere, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "IsUnleashedThere", IsUnleashedThere, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "IsDeluxeThere", IsDeluxeThere, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "IsUploadAllowed", IsUploadAllowed, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "IsDownloadAllowed", IsDownloadAllowed, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "IsSuperstarThere", IsSuperstarThere, VT_BOOL, VTS_NONE)
	DISP_FUNCTION(CMaxisSimsFamilyTeleXCtrl, "IsMakinMagicThere", IsMakinMagicThere, VT_BOOL, VTS_NONE)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CMaxisSimsFamilyTeleXCtrl, COleControl)
	//{{AFX_EVENT_MAP(CMaxisSimsFamilyTeleXCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CMaxisSimsFamilyTeleXCtrl, 1)
	PROPPAGEID(CMaxisSimsFamilyTeleXPropPage::guid)
END_PROPPAGEIDS(CMaxisSimsFamilyTeleXCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CMaxisSimsFamilyTeleXCtrl, "MAXISSIMSFAMILYTELEX.MaxisSimsFamilyTeleXCtrl.1",
	0xa44b714b, 0xee0f, 0x453e, 0x93, 0, 0xa6, 0x9b, 0x32, 0x1f, 0xef, 0x6c)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CMaxisSimsFamilyTeleXCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DMaxisSimsFamilyTeleX =
		{ 0x6b2bb468, 0xdf1a, 0x4731, { 0xa4, 0x4e, 0x79, 0xfa, 0xeb, 0x30, 0xde, 0xcc } };
const IID BASED_CODE IID_DMaxisSimsFamilyTeleXEvents =
		{ 0xfb268452, 0xfb16, 0x4cd0, { 0xb4, 0x67, 0x5d, 0x32, 0x9d, 0x87, 0xcd, 0xf0 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwMaxisSimsFamilyTeleXOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CMaxisSimsFamilyTeleXCtrl, IDS_MAXISSIMSFAMILYTELEX, _dwMaxisSimsFamilyTeleXOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXCtrl::CMaxisSimsFamilyTeleXCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CMaxisSimsFamilyTeleXCtrl

BOOL CMaxisSimsFamilyTeleXCtrl::CMaxisSimsFamilyTeleXCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
	{
		BOOL regResult = AfxOleRegisterControlClass(
								AfxGetInstanceHandle(),
								m_clsid,
								m_lpszProgID,
								IDS_MAXISSIMSFAMILYTELEX,
								IDS_MAXISSIMSFAMILYTELEX,
								afxRegApartmentThreading,
								_dwMaxisSimsFamilyTeleXOleMisc,
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
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXCtrl::CMaxisSimsFamilyTeleXCtrl - Constructor

CMaxisSimsFamilyTeleXCtrl::CMaxisSimsFamilyTeleXCtrl()
{
	InitializeIIDs(&IID_DMaxisSimsFamilyTeleX, &IID_DMaxisSimsFamilyTeleXEvents);

	Logger::Start( "SOFTWARE\\Maxis\\The Sims", "MaxisSimsFamilyTeleX.txt" );

	_hasAlbum = false;

	_bitmap = NULL;
	_DC = NULL;
	_created = false;
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXCtrl::~CMaxisSimsFamilyTeleXCtrl - Destructor

CMaxisSimsFamilyTeleXCtrl::~CMaxisSimsFamilyTeleXCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXCtrl::OnDraw - Drawing function

void CMaxisSimsFamilyTeleXCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	if ( !_created )
		return;

	HDC hDC = pdc->m_hDC;
	if ( hDC == NULL )
		return;

	if ( _DC && _bitmap )
	{
		HBITMAP hOldBitmap = (HBITMAP) ::SelectObject( _DC, _bitmap );

		CRect rect;
		GetClientRect( &rect );

		::StretchBlt( hDC, 0, 0, rect.Width(), rect.Height(),
					_DC,
					0, 0, _width, _height,
					SRCCOPY );

		::SelectObject( _DC, hOldBitmap );
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXCtrl::DoPropExchange - Persistence support

void CMaxisSimsFamilyTeleXCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXCtrl::OnResetState - Reset control to default state

void CMaxisSimsFamilyTeleXCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	_teleporter.Reset();
	_hasAlbum = false;

	// local copies of parameters from javascript
	_scriptPath.erase();
	_assetId.erase();
	_simsPath.erase();
	_subject.erase();
	_filePath.erase();
}


int CMaxisSimsFamilyTeleXCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (COleControl::OnCreate(lpCreateStruct) == -1)
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


void CMaxisSimsFamilyTeleXCtrl::OnDestroy() 
{
	_created = false;

	if ( _bitmap )
		::DeleteObject( _bitmap );

	if ( _DC )
		::DeleteObject( _DC );

	COleControl::OnDestroy();
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXCtrl message handlers


BOOL CMaxisSimsFamilyTeleXCtrl::Initialize(LPCTSTR serverName) 
{
	return _teleporter.Initialize( serverName );
}


BOOL CMaxisSimsFamilyTeleXCtrl::StartUpload(LPCTSTR scriptPath,
						LPCTSTR subject, LPCTSTR neighNumIn) 
{
	// "subject" is the name the game uses for the name of the pub file, minus
	// the file extension.
	// neighNumIn is for future uses, for example, to allow a user to upload
	// without going through the game.
	// If neighNumIn is empty, the registry is consulted.

	// check input parameters, all other checks done in _teleporter.StartUpload()
	if ( subject == NULL || subject[0] == '\0' )
		{ assert(false); return FALSE; }

	SimsConfig sims;
	if ( !sims.Initialize())
		{ assert(false); return FALSE; }

	/* don't prevent users with Sims+noEPs from teleporting
	if ( sims.GetNeighNum().empty() || sims.GetNeighPath().empty())
	{
		Exception e( "No SIMS_CURRENT_NEIGHBORHOOD_* found in registry",
				"You must click the \"View Web Pages\" button in the Sims game\n"
				"before you can use the Sims Family Exchange." );
		e.TellUser( s_title );
		return FALSE;
	}
	*/

	// set userData
	string userData;
	{
		string neighNum;
		if ( neighNumIn == NULL || !IsUnsignedLong( neighNumIn ))
			neighNum = sims.GetNeighNum();
		else
			neighNum = neighNumIn;

		if ( neighNum.empty() || neighNum == "0" || neighNum == "1" )
		{
			// neighbNum of 1 means "UserData".
			// As far as I know, the game never sets SIMS_CURRENT_NEIGHBORHOOD_NUM
			// to "0".
			assert( neighNum != "0" );

			userData = "UserData";
		}
		else
			userData = "UserData" + neighNum;
	}

//	The following code is wrong because subject may have replaced spaces with underscores.
//	For example, "SKU6 NO 3_2.FAM" yields a subject of SKU6_NO_3_2.
//	const string filePath = sims.GetSimsPath() + "\\" + userData + "\\Export\\"
//								+ subject + ".fam";

	// set _hasAlbum and _filePath and make sure PUB file exists
	try
	{
		// set pubFilePath;
		string pubFilePath = sims.GetSimsPath() + "\\" + userData + "\\Export\\"
							+ subject + "." + PubFile::_fileExt;

		PubFile pub;
		if ( pub.Initialize( pubFilePath ))
		{
			_hasAlbum = pub.HasAlbum();
			
			string famName = pub.GetFamFileName();
			if ( famName.empty())
				throw	"Before you can teleport this family, you must restart "
						"the game and save this family.\n"
						"FAM file not found (1)";

			if ( pub.IsFamilyInMakinMagicLot())
				ChangeNameToMM( &famName );

			string famPath = sims.GetSimsPath() + "\\" + userData + "\\Export\\"
								+ famName;

			if ( !FileExists( famPath ))
				throw	"Before you can teleport this family, you must restart "
						"the game and save this family.\n"
						"FAM file not found (2)";

			_filePath = famPath;
		}
		else
			throw "PUB file not found";
	}
	catch ( string err )
	{
		Exception e( "CMaxisSimsFamilyTeleXCtrl::StartUpload", err.c_str());
		e.TellUser( s_title );
		return FALSE;
	}
	catch ( LPCSTR err )
	{
		Exception e( "CMaxisSimsFamilyTeleXCtrl::StartUpload", err );
		e.TellUser( s_title );
		return FALSE;
	}
	catch (...)
	{
		LOG( "Unexpected exception in CMaxisSimsFamilyTeleXCtrl::StartUpload" );
		assert(false);
		return FALSE;
	}

	_scriptPath = scriptPath;
	_subject = subject;
	assert( FileExists( _filePath ));

	return _teleporter.StartUpload( _scriptPath, _filePath, _subject );
}


BOOL CMaxisSimsFamilyTeleXCtrl::StartDownload(LPCTSTR scriptPath, LPCTSTR assetId) 
{
	if ( scriptPath && assetId )
	{
		_scriptPath = scriptPath;
		_assetId = assetId;
	}
	else
	{
		LOG( "CMaxisSimsFamilyTeleXCtrl::StartDownload:  a NULL parameter!" );
		assert(false);
		return FALSE;
	}

	// set _simPath
	SimsConfig sims;
	if ( !sims.Initialize()) { assert(false); return FALSE; }
	_simsPath = sims.GetSimsPath();

	return _teleporter.StartDownload( _scriptPath, _assetId, _simsPath );
}


BSTR CMaxisSimsFamilyTeleXCtrl::GetPercentComplete() 
{
	CString percent = _teleporter.GetPercentComplete().c_str();
	return percent.AllocSysString();
}


BSTR CMaxisSimsFamilyTeleXCtrl::GetOutcome() 
{
	CString outcome = "internal error";

	SimsConfig config;
	bool init = config.Initialize();
	if ( !init )
	{
		assert(false);
		return outcome.AllocSysString();
	}
	string simsPath = config.GetSimsPath();

	string code = _teleporter.FinishTransport( simsPath );
	outcome = MaxisTeleporter1::GetOutcomePhrase( code ).c_str();

	return outcome.AllocSysString();
}


BOOL CMaxisSimsFamilyTeleXCtrl::HasAlbum() 
{
	return _hasAlbum;
}


BOOL CMaxisSimsFamilyTeleXCtrl::IsSimsThere() 
{
	SimsConfig sims;
	if ( !sims.Initialize())
		{ assert(false); return FALSE; }
	return sims.IsSimsThere();
}



BOOL CMaxisSimsFamilyTeleXCtrl::IsUnleashedThere() 
{
	SimsConfig sims;
	if ( !sims.Initialize())
		{ assert(false); return FALSE; }
	return sims.IsUnleashedThere();
}

BOOL CMaxisSimsFamilyTeleXCtrl::IsDeluxeThere() 
{
	SimsConfig sims;
	if ( !sims.Initialize())
		{ assert(false); return FALSE; }
	return sims.IsDeluxeThere();
}

BOOL CMaxisSimsFamilyTeleXCtrl::IsUploadAllowed() 
{
	SimsConfig sims;
	if ( !sims.Initialize())
		{ assert(false); return FALSE; }
	return sims.IsUploadAllowed();
}

BOOL CMaxisSimsFamilyTeleXCtrl::IsDownloadAllowed() 
{
	SimsConfig sims;
	if ( !sims.Initialize())
		{ assert(false); return FALSE; }
	return sims.IsDownloadAllowed();
}


BOOL CMaxisSimsFamilyTeleXCtrl::IsSuperstarThere() 
{
	SimsConfig sims;
	if ( !sims.Initialize())
		{ assert(false); return FALSE; }
	return sims.IsSuperstarThere();
}


// for example, family.fam -> family.mtf, "MM" stand for Makin' Magic
void CMaxisSimsFamilyTeleXCtrl::ChangeNameToMM( string *famName )
{
	if ( famName == NULL || MAX_PATH <= famName->size()) { assert(false); return; }
	DWORD dot = famName->rfind( "." );
	if ( dot == string::npos || dot == 0 ) { assert(false); return; }
	string ext = ToLower( famName->substr( dot + 1 ));
	if ( ext != "fam" ) { assert(false); return; }
	string name = famName->substr( 0, dot );
	*famName = name + ".mtf";
}


BOOL CMaxisSimsFamilyTeleXCtrl::IsMakinMagicThere() 
{
	SimsConfig sims;
	if ( !sims.Initialize())
		{ assert(false); return FALSE; }
	return sims.IsMakinMagicThere();
}
