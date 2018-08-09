// SimsTeleporterTest.h : main header file for the SIMSTELEPORTERTEST application
//

#if !defined(AFX_SIMSTELEPORTERTEST_H__31FBBA46_FB57_46DF_8C6A_5B1DAE559C5A__INCLUDED_)
#define AFX_SIMSTELEPORTERTEST_H__31FBBA46_FB57_46DF_8C6A_5B1DAE559C5A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSimsTeleporterTestApp:
// See SimsTeleporterTest.cpp for the implementation of this class
//

class CSimsTeleporterTestApp : public CWinApp
{
public:
	CSimsTeleporterTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimsTeleporterTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSimsTeleporterTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMSTELEPORTERTEST_H__31FBBA46_FB57_46DF_8C6A_5B1DAE559C5A__INCLUDED_)
