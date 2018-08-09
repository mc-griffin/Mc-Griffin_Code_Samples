// MaxisSimsFamilyTeleXPpg.cpp : Implementation of the CMaxisSimsFamilyTeleXPropPage property page class.

#include "stdafx.h"
#include "MaxisSimsFamilyTeleX.h"
#include "MaxisSimsFamilyTeleXPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CMaxisSimsFamilyTeleXPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CMaxisSimsFamilyTeleXPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CMaxisSimsFamilyTeleXPropPage)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CMaxisSimsFamilyTeleXPropPage, "MAXISSIMSFAMILYTELEX.MaxisSimsFamilyTeleXPropPage.1",
	0x5e71ca35, 0x8537, 0x4159, 0xba, 0x24, 0x39, 0x55, 0x35, 0xb1, 0xca, 0xd9)


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXPropPage::CMaxisSimsFamilyTeleXPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CMaxisSimsFamilyTeleXPropPage

BOOL CMaxisSimsFamilyTeleXPropPage::CMaxisSimsFamilyTeleXPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_MAXISSIMSFAMILYTELEX_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXPropPage::CMaxisSimsFamilyTeleXPropPage - Constructor

CMaxisSimsFamilyTeleXPropPage::CMaxisSimsFamilyTeleXPropPage() :
	COlePropertyPage(IDD, IDS_MAXISSIMSFAMILYTELEX_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CMaxisSimsFamilyTeleXPropPage)
	// NOTE: ClassWizard will add member initialization here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXPropPage::DoDataExchange - Moves data between page and properties

void CMaxisSimsFamilyTeleXPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CMaxisSimsFamilyTeleXPropPage)
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXPropPage message handlers
