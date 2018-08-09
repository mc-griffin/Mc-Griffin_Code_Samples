#if !defined(AFX_MAXISSIMCITY4LOTTELEXPPG_H__22D58B7D_E6CF_4B87_9312_AA3A8A69E61A__INCLUDED_)
#define AFX_MAXISSIMCITY4LOTTELEXPPG_H__22D58B7D_E6CF_4B87_9312_AA3A8A69E61A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MaxisSimCity4LotTeleXPpg.h : Declaration of the CMaxisSimCity4LotTeleXPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CMaxisSimCity4LotTeleXPropPage : See MaxisSimCity4LotTeleXPpg.cpp.cpp for implementation.

class CMaxisSimCity4LotTeleXPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CMaxisSimCity4LotTeleXPropPage)
	DECLARE_OLECREATE_EX(CMaxisSimCity4LotTeleXPropPage)

// Constructor
public:
	CMaxisSimCity4LotTeleXPropPage();

// Dialog Data
	//{{AFX_DATA(CMaxisSimCity4LotTeleXPropPage)
	enum { IDD = IDD_PROPPAGE_MAXISSIMCITY4LOTTELEX };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CMaxisSimCity4LotTeleXPropPage)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAXISSIMCITY4LOTTELEXPPG_H__22D58B7D_E6CF_4B87_9312_AA3A8A69E61A__INCLUDED)
