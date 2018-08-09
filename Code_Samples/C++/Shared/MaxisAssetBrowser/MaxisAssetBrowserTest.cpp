// MaxisAssetBrowserTest.cpp : Defines the entry point for the console application.
//

#include "MaxisAssetBrowser.h"

// APP changed all "HdLot" to "SuperstarStudio" and "L24" to "L85"
//#include "HdLot.h"
#include "SuperstarStudio.h"

int main(int argc, char* argv[])
{
	//MaxisFileBrowser::Test();

	printf( "Test MaxisAssetBrowser\n" );
	bool passed = true;

	// Don't execute HdLot::Test() or MaxisFileBrowser::Test()
	// because it assumes the executable is in a particular directory.
	
	MaxisAssetBrowser<SuperstarStudio> b;

	/* APP this generates an assert because MaxisAssetBrowser doesn't want an empty dir
	if ( b.SelectDir( "" ))
	{
		printf( "SelectDir() failed\n" );
		passed = false;
	}

	if ( !b.GetFirstDir().empty() || !b.GetFirstFile().empty()
			|| !b.GetNextDir().empty() || !b.GetNextFile().empty())
	{
		printf( "GetFirst/NextDir/File() failed\n" );
		passed = false;
	}
	*/

	char dir[ MAX_PATH + 1 ] = "";
	DWORD len = GetCurrentDirectory( MAX_PATH, dir );
	assert( 0 < len );

	string testDir = dir;
	testDir += "\\Test Data";

	if ( !b.SelectDir( testDir ))
	{
		printf( "SelectDir() failed\n" );
		passed = false;
	}

	if ( b.GetFirstFile() != "L85.srf" )
	{
		printf( "GetFirstFile() failed\n" );
		passed = false;
	}

	if ( !b.GetNextFile().empty())
	{
		printf( "GetNextFile() failed\n" );
		passed = false;
	}

	return 0;
}
