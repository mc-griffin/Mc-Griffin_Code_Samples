// SelectionList.cpp: implementation of the SelectionList class.
//
//////////////////////////////////////////////////////////////////////

#include "SelectionList.h"


bool SelectionList::Append( const string &name )
{
	if ( name.empty() || _maxNameLen <= name.length()) { assert(false); return false; }

	_names.push_back( name );
	return true;
}


DWORD SelectionList::GetSize() const
{
	return _names.size();
}


bool SelectionList::SelectFirst()
{
	if ( _names.empty())
		return false;
	_selection = 0;
	return true;
}


bool SelectionList::SelectNext()
{
	if ( _selection < 0 || _names.size() <= _selection )
	{
		assert( _selection == _names.size()); // at end of list
		return false;
	}

	++_selection;
	return _selection < _names.size();
}


string SelectionList::GetSelected() const
{
	if ( _selection < 0 || _names.size() <= _selection )
		return "";

	return _names[ _selection ];
}


void SelectionList::MakeEmpty()
{
	_names.clear();
	_selection = -1;
}


bool SelectionList::Test()
{
	printf( "Test SelectionList\n" );
	SelectionList l;
	bool passed = true;

	if ( !l.GetSelected().empty())
	{
		printf( "GetSelected() failed\n" );
		passed = false;
	}

	if ( !l.Append("a") || !l.Append("b") || !l.Append("c"))
	{
		printf( "Append() failed\n" );
		passed = false;
	}

	if ( l.GetSize() != 3 )
	{
		printf( "GetSelected() failed\n" );
		passed = false;
	}

	if ( !l.GetSelected().empty())
	{
		printf( "GetSelected() failed\n" );
		passed = false;
	}

	if ( !l.SelectFirst())
	{
		printf( "SelectFirst() failed\n" );
		passed = false;
	}

	if ( l.GetSelected() != "a" )
	{
		printf( "GetSelected() failed\n" );
		passed = false;
	}

	if ( !l.SelectNext())
	{
		printf( "SelectNext() failed\n" );
		passed = false;
	}

	if ( l.GetSelected() != "b" )
	{
		printf( "GetSelected() failed\n" );
		passed = false;
	}

	if ( !l.SelectNext())
	{
		printf( "SelectNext() failed\n" );
		passed = false;
	}

	if ( l.GetSelected() != "c" )
	{
		printf( "GetSelected() failed\n" );
		passed = false;
	}

	if ( l.SelectNext())
	{
		printf( "SelectNext() failed\n" );
		passed = false;
	}

	if ( l.GetSelected() != "c" )
	{
		printf( "GetSelected() failed\n" );
		passed = false;
	}

	if ( l.SelectNext())
	{
		printf( "SelectNext() failed\n" );
		passed = false;		
	}

	return passed;
}


