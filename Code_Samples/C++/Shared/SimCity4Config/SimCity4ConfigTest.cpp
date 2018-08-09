// SimCityConfigTest.cpp : Defines the entry point for the console application.
//

#include "SimCity4Config.h"


int main(int argc, char* argv[])
{
	SimCity4Config sc;
	if ( !sc.Initialize( "\n" ))
		printf( "Initialize() failed\n" );

	if ( sc.IsSet() == sc.GetSerialNumber().empty())
		printf( "IsSet() or GetSerialNumber() failed\n" );

	printf( "%s", sc.GetDescription().c_str());

	string log;
	log += "_installPath is ";
	log += sc.GetInstallPath();
	log += " and _downloadPath is ";
	log += sc.GetDownloadPath();
	log += " and _language is ";
	log += sc.GetLanguage();
	log += " and it ";
	if (sc.IsReasonableCharSet())
		log += " IS a reasonable character set, yay!";
	else
		log += " is NOT a reasonable character set, boo.";
	if ( sc.IsRushHourThere())
		log += "\nRush Hour is found.\n";
	if ( sc.IsDeluxeThere())
		log += "\nDeluxe is found.\n";
	printf( "%s\n", log.c_str());

	return 0;
}
