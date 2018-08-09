#if !defined(AFX_MAXISSIMSFAMILYTELEX_H__EA53350A_DF8A_4BC3_A93D_BB8EBDC4CD36__INCLUDED_)
#define AFX_MAXISSIMSFAMILYTELEX_H__EA53350A_DF8A_4BC3_A93D_BB8EBDC4CD36__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MaxisSimsFamilyTeleX.h : main header file for MAXISSIMSFAMILYTELEX.DLL

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXApp : See MaxisSimsFamilyTeleX.cpp for implementation.

class CMaxisSimsFamilyTeleXApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAXISSIMSFAMILYTELEX_H__EA53350A_DF8A_4BC3_A93D_BB8EBDC4CD36__INCLUDED)
