// SimCity4LotTeleporterTestDlg.h : header file
//

#if !defined(AFX_SIMCITY4LOTTELEPORTERTESTDLG_H__E7C8A982_B22D_4B56_9B25_4DD1E43ACB99__INCLUDED_)
#define AFX_SIMCITY4LOTTELEPORTERTESTDLG_H__E7C8A982_B22D_4B56_9B25_4DD1E43ACB99__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSimCity4LotTeleporterTestDlg dialog

class CSimCity4LotTeleporterTestDlg : public CDialog
{
// Construction
public:
	CSimCity4LotTeleporterTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSimCity4LotTeleporterTestDlg)
	enum { IDD = IDD_SIMCITY4LOTTELEPORTERTEST_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimCity4LotTeleporterTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSimCity4LotTeleporterTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMCITY4LOTTELEPORTERTESTDLG_H__E7C8A982_B22D_4B56_9B25_4DD1E43ACB99__INCLUDED_)
