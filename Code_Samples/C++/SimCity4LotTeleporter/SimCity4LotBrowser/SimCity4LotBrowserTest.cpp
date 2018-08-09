// UnleashedLotBrowser.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include "SimCity4LotBrowser.h"


int main(int argc, char* argv[])
{
	printf("Test SimCity4Browser\n");

	SimCity4LotBrowser b;

	// set testDir
	char dir[MAX_PATH] = "";
	if ( !GetCurrentDirectory( MAX_PATH, dir ))
		printf( "GetCurrentDirectory() failed\n" );
	else {
		char testDir[MAX_PATH] = "";
		strcpy( testDir, dir );
		strcat( testDir, "\\" );
		strcat( testDir, "Test Data" );

		bool sel = b.SelectDir( testDir );
		if ( !sel ) 
			printf( "SelectDir() failed\n" );
		else {
			for ( string s = b.GetFirstDir(); !s.empty(); s = b.GetNextDir())
				printf( "%s\n", s.c_str());
			for ( s = b.GetFirstFile(); !s.empty(); s = b.GetNextFile())
				printf( "%s\n", s.c_str());
		}
	}
	return 0;
}

