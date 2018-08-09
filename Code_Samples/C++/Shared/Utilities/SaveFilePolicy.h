// SaveFilePolicy.h: interface for the SaveFilePolicy classes.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SAVEFILEPOLICY_H__25EFA6B0_F472_4F0B_8F62_465C22F2001E__INCLUDED_)
#define AFX_SAVEFILEPOLICY_H__25EFA6B0_F472_4F0B_8F62_465C22F2001E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <windows.h>
#include <assert.h>

// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>

using namespace std;


// Note: None of these classes have code to actually write a file.
// This code only deals with deciding whether a file should be written,
// given the chosen policy.


class SaveFilePolicy
{
public:
	void SetFilePath( const string &filePath )
	{
		if ( filePath.empty() || MAX_PATH <= filePath.length())
			{ assert(false); return; }
		_filePath = filePath;
	}

	// SFP_PROCEED means proceed with the file write.  SFP_CANCEL means that
	// either the user or the policy canceled the file write.
	enum Outcome { SFP_ERROR, SFP_CANCEL, SFP_PROCEED };

	virtual Outcome Apply() = 0;
	// After Apply() finishes, _selectedFilePath should be set, even if empty.
	// _outcome should also be set.

	string GetSelectedFilePath()
	{
//		assert( !_selectedFilePath.empty());
		return _selectedFilePath;
	}
	Outcome GetOutcome() { return _outcome; }

	// Verify:  since this class has a non-default constructor, it cannot
	// be used as a base type for an STL container, like vector.
	// Even if its true, that's probably fine.
	SaveFilePolicy() : _outcome( SFP_ERROR ) {}

protected:
	string _filePath;
	string _selectedFilePath;
	Outcome _outcome;
};


class SfpSilentlyOverWrite : public SaveFilePolicy
{
public:
	virtual Outcome Apply()
	{
		assert( !_filePath.empty());
		_selectedFilePath = _filePath;
		_outcome = SFP_PROCEED;
		return SFP_PROCEED;
	}
};


class SfpSaveAs : public SaveFilePolicy
{
public:
	void SetInst( HINSTANCE inst );
	void SetTitle( const string &title );
	void SetDefaultFilePath( const string &defaultFilePath )
	{
		SetFilePath( defaultFilePath );
	}
	void SetParent( HWND parent );

	virtual Outcome Apply();

	SfpSaveAs() : _inst( NULL ), _parent( NULL ) {}

	static void Test( HINSTANCE inst );

protected:
	HINSTANCE _inst;
	string _title;
	HWND _parent;
};


class SfpConflictAlert : public SaveFilePolicy
{
public:
	void SetTitle( const string &title );
	void SetParent( HWND parent );

	virtual Outcome Apply();

	SfpConflictAlert() : _parent( NULL ) {}

	static void Test();

protected:
	string _title;
	HWND _parent;
};


class SfpDoNotOverWrite : public SaveFilePolicy
{
public:
	virtual Outcome Apply();

	static bool Test();
};


class SfpJailedSaveAs : public SfpSaveAs
{
public:
	void SetJail( const string &jailDir );
	bool InJail( const string &path );
	Outcome Apply();

	static void Test( HINSTANCE inst, HWND parent=NULL );

protected:
	string _jailDir;
};



// This class does nothing but returns SFP_ERROR.
// Use this class for HttpReqResp uploads, when there will be no call to
// MaxisTeleporter::SaveBodyAsFile().
class SfpNullPolicy : public SaveFilePolicy
{
public:
	Outcome Apply() { _outcome = SFP_ERROR; return SFP_ERROR; }
};


#endif // !defined(AFX_SAVEFILEPOLICY_H__25EFA6B0_F472_4F0B_8F62_465C22F2001E__INCLUDED_)
