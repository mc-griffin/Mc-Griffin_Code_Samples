// HttpReqRespTestMfc.h : main header file for the HTTPREQRESPTESTMFC application
//

#if !defined(AFX_HTTPREQRESPTESTMFC_H__1DD56B1D_0278_47A7_8A5C_462A76FB54E0__INCLUDED_)
#define AFX_HTTPREQRESPTESTMFC_H__1DD56B1D_0278_47A7_8A5C_462A76FB54E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CHttpReqRespTestMfcApp:
// See HttpReqRespTestMfc.cpp for the implementation of this class
//

class CHttpReqRespTestMfcApp : public CWinApp
{
public:
	CHttpReqRespTestMfcApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHttpReqRespTestMfcApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CHttpReqRespTestMfcApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HTTPREQRESPTESTMFC_H__1DD56B1D_0278_47A7_8A5C_462A76FB54E0__INCLUDED_)
