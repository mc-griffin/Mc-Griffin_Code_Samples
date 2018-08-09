// HttpReqRespTestMfcDlg.h : header file
//

#if !defined(AFX_HTTPREQRESPTESTMFCDLG_H__D3B40E84_287F_439B_81B5_8002611BA94E__INCLUDED_)
#define AFX_HTTPREQRESPTESTMFCDLG_H__D3B40E84_287F_439B_81B5_8002611BA94E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CHttpReqRespTestMfcDlg dialog

class CHttpReqRespTestMfcDlg : public CDialog
{
// Construction
public:
	CHttpReqRespTestMfcDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CHttpReqRespTestMfcDlg)
	enum { IDD = IDD_HTTPREQRESPTESTMFC_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHttpReqRespTestMfcDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CHttpReqRespTestMfcDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HTTPREQRESPTESTMFCDLG_H__D3B40E84_287F_439B_81B5_8002611BA94E__INCLUDED_)
