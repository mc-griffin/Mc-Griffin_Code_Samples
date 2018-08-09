// SelectionList.h
//

#if !defined( _SELECTION_LIST_H_ )
#define _SELECTION_LIST_H_

#include <windows.h>
#include <assert.h>

// okay to disable warning message "symbol greater than 255 character"
#pragma warning( disable : 4786 )
#include <string>
#include <vector>

using namespace std;


// I thought about making this class a template:
// template <class C> class SelectionList { vector<C> _names;
// but there is no immediate need to justify complicating the code.

class SelectionList
{
protected:
	vector<string> _names;
	long _selection;

public:
	SelectionList() : _selection(-1), _maxNameLen(2048) {}

	bool Append( const string &name );
	DWORD GetSize() const;

	bool SelectFirst();
	bool SelectNext();
	string GetSelected() const;

	void MakeEmpty();

	const WORD _maxNameLen;

	static bool Test();
};

#endif // !defined( _SELECTION_LIST_H_ )

