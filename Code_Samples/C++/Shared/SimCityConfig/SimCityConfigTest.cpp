// SimCityConfigTest.cpp : Defines the entry point for the console application.
//

#include "SimCityConfig.h"


int main(int argc, char* argv[])
{
	SimCityConfig sc;
	if ( !sc.Initialize( "\n" ))
		printf( "Initialize() failed\n" );

	if ( sc.IsSet() == sc.GetSerialNumber().empty())
		printf( "IsSet() or GetSerialNumber() failed\n" );

	printf( "%s", sc.GetDescription().c_str());

	string log;
	log += "_pathNoApps is ";
	log += sc.GetPathNoApps();
	printf( "%s\n", log.c_str());

	return 0;
}
