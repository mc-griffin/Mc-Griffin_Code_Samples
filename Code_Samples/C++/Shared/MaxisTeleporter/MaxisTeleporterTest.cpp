// MaxisTeleporterTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"


BOOL CALLBACK TestProc( HWND dlg, UINT msg, WPARAM w, LPARAM l );


bool s_testNetwork = true;
//string s_serverName = "thesims.ea.com";
//string s_serverAddress = "159.153.194.3";
//static string s_serverName = "int.thesims.max.ad.ea.com";
//static string s_serverAddress = "10.24.5.85";
//static string s_serverName = "thesims.simbeta.com";
//static string s_serverAddress = "159.153.194.7"; //10.24.4.59
//static string s_serverName = "gmcclellan.thesims.max.ad.ea.com";
//static string s_serverAddress = "10.24.5.85";
static string s_serverName = "nmolon.thesims.max.ad.ea.com";
static string s_serverAddress = "10.24.5.85";

HINSTANCE s_inst = NULL;
HWND s_log = NULL;


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	s_inst = hInstance;
	Logger::GetReference().Start( "SOFTWARE\\Maxis\\The Sims", "MaxisTeleporterTest.txt" );
	DialogBox( hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, TestProc );

	return 0;
}


BOOL CALLBACK TestProc( HWND dlg, UINT msg, WPARAM w, LPARAM l )
{
	static bool initialized = false;

	if ( msg == WM_INITDIALOG )
	{
		s_log = GetDlgItem( dlg, IDC_LOG ); assert( s_log );

		{
			initialized = false;

			string tests = MaxisTeleporter::Test( s_testNetwork,
											s_serverName, s_inst );

			DWORD left = 0, right = 0;
			while ( left < tests.length() )
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
				int add = SendMessage( s_log, LB_ADDSTRING, 0,
													(LPARAM) test.c_str());
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

		return TRUE;
	}

	if ( msg != WM_COMMAND )
		return FALSE;

	WORD cmd = LOWORD( w );
	switch ( cmd )
	{
		case IDOK: // carriage returns
		case IDCANCEL:
			EndDialog( dlg, cmd );
			return TRUE;
	}

	return FALSE;
}

