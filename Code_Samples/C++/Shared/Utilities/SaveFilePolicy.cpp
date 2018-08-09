// SaveFilePolicy.cpp: implementation of the SaveFilePolicy class.
//
//////////////////////////////////////////////////////////////////////

#include "SaveFilePolicy.h"
#include "FileUtils.h"
#include "Utilities.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static const WORD s_maxTitleLen = 128; // 128 by fiat


// SfpSaveAs


void SfpSaveAs::SetInst( HINSTANCE inst )
{
	if ( inst == NULL ) { assert(false); return; }
	_inst = inst;
}


void SfpSaveAs::SetParent( HWND parent )
{
	if ( parent == NULL ) { assert(false); return; }
	_parent = parent;
}


void SfpSaveAs::SetTitle( const string &title )
{
	if ( title.length() == 0 || s_maxTitleLen < title.length())
		{ assert(false); return; }
	_title = title;
}


SaveFilePolicy::Outcome SfpSaveAs::Apply()
{
	if ( _title.empty() || _inst == NULL )
	{
		_outcome = SFP_ERROR;
		assert(false);
		return SFP_ERROR;
	}

	// display Save As dialog box

	_selectedFilePath = SaveAsSelectFilePath( _filePath,
												_title, _inst, _parent );
	if ( _selectedFilePath.empty())
	{
		_outcome = SFP_CANCEL;
		return SFP_CANCEL;
	}

	_outcome = SFP_PROCEED;
	return SFP_PROCEED;
}


void SfpSaveAs::Test( HINSTANCE inst )
{
	char currDir[MAX_PATH+1] = "";
	DWORD len = GetCurrentDirectory( MAX_PATH, currDir ); assert( 0 < len );
	string filePath = currDir;
	filePath += "\\";
	filePath += "i bet this file doesn't exist.txt";

	SfpSaveAs saveAs;
	saveAs.SetInst( inst );
	saveAs.SetTitle( "SfpSaveAs::Test()" );
	saveAs.SetDefaultFilePath( filePath );
	//saveAs.SetParent( GetTopWindow(NULL));

	Outcome outcome = saveAs.Apply();

	string msg = "outcome is ";
	switch ( outcome )
	{
		case SFP_ERROR:
			msg = "SFP_ERROR";
			break;
		case SFP_CANCEL:
			msg = "SFP_CANCEL";
			break;
		case SFP_PROCEED:
			msg = "SFP_PROCEED";
			break;
		default:
			msg = "Unknown!";
			break;
	}
	msg += "\n";
	msg += "GetSelectedFilePath() returned ";
	msg += saveAs.GetSelectedFilePath();
	
	MessageBox( NULL, msg.c_str(), "SfpSaveAs::Test()", MB_OK );
}


// SfpConflictAlert


void SfpConflictAlert::SetParent( HWND parent )
{
	if ( parent == NULL ) { assert(false); return; }
	_parent = parent;
}


void SfpConflictAlert::SetTitle( const string &title )
{
	if ( title.length() == 0 || s_maxTitleLen < title.length()) { assert(false); return; }
	_title = title;
}


SaveFilePolicy::Outcome SfpConflictAlert::Apply()
{
	if ( _filePath.empty() || _title.empty())
	{
		_outcome = SFP_ERROR;
		assert(false);
		return SFP_ERROR;
	}

	if ( !FileExists( _filePath ))
	{
		_selectedFilePath = _filePath;
		_outcome = SFP_PROCEED;
		return SFP_PROCEED;
	}

	// display conflict alert message box
	string msg = "The file named\n\"";
	msg += _filePath;
	msg += "\"\nalready exists.\n\n";
	msg += "To replace the old file with the new one, click OK.\n";
	msg += "To keep the old file, click Cancel.";
	int choice = MessageBox( _parent, msg.c_str(), _title.c_str(), MB_OKCANCEL );
	if ( choice != IDOK )
	{
		_outcome = SFP_CANCEL;
		return SFP_CANCEL;
	}

	_selectedFilePath = _filePath;
	_outcome = SFP_PROCEED;
	return SFP_PROCEED;
}


void SfpConflictAlert::Test()
{
	// try a file that (presumably) doesn't exist
	{
		char currDir[MAX_PATH+1] = "";
		DWORD len = GetCurrentDirectory( MAX_PATH, currDir ); assert( 0 < len );
		string filePath = currDir;
		filePath += "\\";
		filePath += "i bet this file doesn't exist.txt";

		SfpConflictAlert alert;
		alert.SetFilePath( filePath );
		alert.SetTitle( "SfpConflictAlert::Test()" );
		//alert.SetParent( GetTopWindow(NULL));

		Outcome outcome = alert.Apply();

		if ( outcome != SFP_PROCEED )
			MessageBox( NULL, "Error", "SfpConflictAlert::Test()", MB_OK );
	}

	// try a file that does exist
	{
		char currDir[MAX_PATH+1] = "";
		DWORD len = GetCurrentDirectory( MAX_PATH, currDir ); assert( 0 < len );
		string filePath = currDir;
		filePath += "\\";
		filePath += "MaxisTeleporterTest.dsp";

		SfpConflictAlert alert;
		alert.SetFilePath( filePath );
		alert.SetTitle( "SfpConflictAlert::Test()" );
		//alert.SetParent( GetTopWindow(NULL));

		Outcome outcome = alert.Apply();

		string msg = "outcome is ";
		switch ( outcome )
		{
			case SFP_ERROR:
				msg = "SFP_ERROR";
				break;
			case SFP_CANCEL:
				msg = "SFP_CANCEL";
				break;
			case SFP_PROCEED:
				msg = "SFP_PROCEED";
				break;
			default:
				msg = "Unknown!";
				break;
		}
		
		MessageBox( NULL, msg.c_str(), "SfpConflictAlert::Test()", MB_OK );
	}
}


// SfpDoNotOverWrite


SaveFilePolicy::Outcome SfpDoNotOverWrite::Apply()
{
	if ( _filePath.empty())
	{
		_outcome = SFP_ERROR;
		assert(false);
		return SFP_ERROR;
	}

	if ( FileExists( _filePath ))
	{
		_outcome = SFP_CANCEL;
		return SFP_CANCEL;
	}

	_selectedFilePath = _filePath;

	_outcome = SFP_PROCEED;
	return SFP_PROCEED;
}


bool SfpDoNotOverWrite::Test()
{
	SfpDoNotOverWrite not1;
	not1.SetFilePath( "c:\\autoexec.bat" );

	Outcome outcome = not1.Apply();

	bool correct = true;
	switch ( outcome )
	{
		case SFP_CANCEL:
			break;
		case SFP_ERROR:
		case SFP_PROCEED:
		default:
			correct = false;
			break;
	}
	
	SfpDoNotOverWrite not2;
	not2.SetFilePath( "c:\\i assume this file doesn't exist.txt" );

	outcome = not2.Apply();

	switch ( outcome )
	{
		case SFP_PROCEED:
			break;
		case SFP_CANCEL:
		case SFP_ERROR:
		default:
			correct = false;
			break;
	}

	return correct;
}




// SfpJailedSaveAs


void SfpJailedSaveAs::SetJail( const string &jailDir )
{
	if ( !DirExists( jailDir ))
		{ assert(false); return; }

	_jailDir = jailDir;

	if ( HasEndingBackSlash( _jailDir ))
		_jailDir.resize( _jailDir.size() - 1 );
}


// path can be either a file path or directory path
bool SfpJailedSaveAs::InJail( const string &pathIn )
{
	if ( _jailDir.empty())
		{ assert(false); return false; }

	string path = ToLower( pathIn );
	if ( HasEndingBackSlash( path ))
		path.resize( path.size() - 1 );

	string dir;
	bool split = SplitFilePath( path, &dir, NULL, NULL );
	if ( !split )
		{ assert(false); return false; }

	return dir.find( ToLower( _jailDir )) == 0;
}


SaveFilePolicy::Outcome SfpJailedSaveAs::Apply()
{
	if ( _filePath.empty() || _jailDir.empty())
	{
		_outcome = SFP_ERROR;
		assert(false);
		return SFP_ERROR;
	}

	if ( !FileExists( _filePath ))
	{
		_selectedFilePath = _filePath;
		_outcome = SFP_PROCEED;
		return SFP_PROCEED;
	}

	while ( SfpSaveAs::Apply() == SFP_PROCEED && !InJail( GetSelectedFilePath()))
	{
		string text = "You must save the file within the directory\n" + _jailDir;
		MessageBox( _parent, text.c_str(), _title.c_str(), MB_OK );
	}

	return GetOutcome();
}


void SfpJailedSaveAs::Test( HINSTANCE inst, HWND parent /*=NULL*/ )
{
	// SfpSaveAs::Test( inst );

	SfpJailedSaveAs jailed;
	jailed.SetInst( inst );
	jailed.SetParent( parent );
	jailed.SetTitle( "SfpResolveFileName::Test()" );
	string filePath, jailDir; 
	{
		char currDir[MAX_PATH+1] = "";
		DWORD len = GetCurrentDirectory( MAX_PATH, currDir ); assert( 0 < len );
		jailDir = currDir;
		filePath = jailDir + "\\ResolveFileNameTestMfc.cpp";
	}
	jailed.SetDefaultFilePath( filePath );
	jailed.SetJail( jailDir );

	Outcome outcome = jailed.Apply();

	string text = "outcome is ";
	switch ( outcome )
	{
		case SFP_CANCEL: text += "SFP_CANCEL"; break;
		case SFP_ERROR: text += "SFP_ERROR"; break;
		case SFP_PROCEED: text += "SFP_PROCEED"; break;
		default:
			break;
	}

	MessageBox( parent, text.c_str(), "SfpResolveFileName::Test()", MB_OK );
	BOOL set = SetCurrentDirectory( jailDir.c_str()); assert(set);
}


