#if !defined(AFX_MAXISSIMSFAMILYTELEXPPG_H__DD2F8490_AEB9_4A86_A192_8DF976DEF380__INCLUDED_)
#define AFX_MAXISSIMSFAMILYTELEXPPG_H__DD2F8490_AEB9_4A86_A192_8DF976DEF380__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MaxisSimsFamilyTeleXPpg.h : Declaration of the CMaxisSimsFamilyTeleXPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CMaxisSimsFamilyTeleXPropPage : See MaxisSimsFamilyTeleXPpg.cpp.cpp for implementation.

class CMaxisSimsFamilyTeleXPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CMaxisSimsFamilyTeleXPropPage)
	DECLARE_OLECREATE_EX(CMaxisSimsFamilyTeleXPropPage)

// Constructor
public:
	CMaxisSimsFamilyTeleXPropPage();

// Dialog Data
	//{{AFX_DATA(CMaxisSimsFamilyTeleXPropPage)
	enum { IDD = IDD_PROPPAGE_MAXISSIMSFAMILYTELEX };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CMaxisSimsFamilyTeleXPropPage)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAXISSIMSFAMILYTELEXPPG_H__DD2F8490_AEB9_4A86_A192_8DF976DEF380__INCLUDED)
