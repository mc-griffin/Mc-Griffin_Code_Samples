// MaxisSimCity4LotTeleXPpg.cpp : Implementation of the CMaxisSimCity4LotTeleXPropPage property page class.

#include "stdafx.h"
#include "MaxisSimCity4LotTeleX.h"
#include "MaxisSimCity4LotTeleXPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CMaxisSimCity4LotTeleXPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CMaxisSimCity4LotTeleXPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CMaxisSimCity4LotTeleXPropPage)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CMaxisSimCity4LotTeleXPropPage, "MAXISSIMCITY4LOTTELEX.MaxisSimCity4LotTeleXPropPage.1",
	0x5ecdeec, 0xcaac, 0x4cea, 0xb7, 0x1d, 0x80, 0, 0x72, 0x75, 0x4f, 0xaa)


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXPropPage::CMaxisSimCity4LotTeleXPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CMaxisSimCity4LotTeleXPropPage

BOOL CMaxisSimCity4LotTeleXPropPage::CMaxisSimCity4LotTeleXPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_MAXISSIMCITY4LOTTELEX_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXPropPage::CMaxisSimCity4LotTeleXPropPage - Constructor

CMaxisSimCity4LotTeleXPropPage::CMaxisSimCity4LotTeleXPropPage() :
	COlePropertyPage(IDD, IDS_MAXISSIMCITY4LOTTELEX_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CMaxisSimCity4LotTeleXPropPage)
	// NOTE: ClassWizard will add member initialization here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXPropPage::DoDataExchange - Moves data between page and properties

void CMaxisSimCity4LotTeleXPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CMaxisSimCity4LotTeleXPropPage)
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXPropPage message handlers
