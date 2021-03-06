// HttpReqRespTestMfcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HttpReqRespTestMfc.h"
#include "HttpReqRespTestMfcDlg.h"


#include "HttpReqResp.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static bool s_testNetwork = true;
static bool s_testUi = false;
//static string s_serverName = "int.thesims.max.ad.ea.com";
//static string s_serverAddress = "10.24.5.85";
//static string s_serverName = "thesims.ea.com";
//static string s_serverAddress = "159.153.194.3";
//static string s_serverName = "thesims.simbeta.com";
//static string s_serverAddress = "159.153.194.7"; //10.24.4.59
//static string s_serverName = "gmcclellan.thesims.max.ad.ea.com";
//static string s_serverAddress = "10.24.5.85";
static string s_serverName = "thesims.max.ad.ea.com";
static string s_serverAddress = "10.24.5.85";


HWND s_log = NULL;


string Tests()
{
	DWORD error = GetLastError();
	if ( error != 0 )
		SetLastError( 0 );

	Logger::Start( "SOFTWARE\\Maxis\\The Sims", "HttpReqRespTest.txt" );

	string log;

/*	log += RequestLine::Test(); // passed
	log += NameValuePair::Test(); // passed
	log += RequestHeaders::Test();
	log += RequestBody::Test();
	log += Exception::Test( s_testUi );
	log += Server::Test( s_testNetwork, s_testUi, s_serverName, s_serverAddress );
	log += HeadRequest::Test( s_testNetwork, s_serverName, s_serverAddress );
	log += GetRequest::Test( s_testNetwork, s_serverName, s_serverAddress );
	log += PostRequest::Test( s_testNetwork, s_serverName, s_serverAddress );
	log += ResponseHeaders::Test( s_serverName, s_serverAddress );
	log += Response::Test( s_serverName, s_serverAddress );
	log += Logger::Test();
*/	log += HttpReqResp::Test( s_testUi, s_testNetwork, s_serverName, s_serverAddress );

	return log;
}


/////////////////////////////////////////////////////////////////////////////
// CHttpReqRespTestMfcDlg dialog

CHttpReqRespTestMfcDlg::CHttpReqRespTestMfcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHttpReqRespTestMfcDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHttpReqRespTestMfcDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHttpReqRespTestMfcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHttpReqRespTestMfcDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHttpReqRespTestMfcDlg, CDialog)
	//{{AFX_MSG_MAP(CHttpReqRespTestMfcDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHttpReqRespTestMfcDlg message handlers

BOOL CHttpReqRespTestMfcDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	{
		s_log = ::GetDlgItem( this->GetSafeHwnd(), IDC_LOG ); assert( s_log );

		{
			string tests = Tests();

			DWORD left = 0, right = 0;
			while ( right < tests.length() )
			{
				// scan test text
				while ( right < tests.length())
				{
					char c = tests.at( right );
					if ( c == '\n' || c == '\r' )
						break;

					++right;
				}
				
				string test = tests.substr( left, right - left );
				int add = ::SendMessage( s_log, LB_ADDSTRING, 0, (LPARAM) test.c_str());
				assert( add != LB_ERR );

				// skip over '\n' and '\r'
				while ( right < tests.length())
				{
					char c = tests.at( right );
					if ( c != '\n' && c != '\r' )
						break;

					++right;
				}
				left = right;
			}
		}
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHttpReqRespTestMfcDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHttpReqRespTestMfcDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
