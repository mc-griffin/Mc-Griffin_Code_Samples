// MaxisAssetTest.cpp : Defines the entry point for the console application.
//

#include "MaxisAssetTest.h"
#include <assert.h>
#include <stdio.h>


int main(int argc, char* argv[])
{
	printf("Test MaxisAsset and MaxisLot\n");

	char currDir[MAX_PATH] = "";
	GetCurrentDirectory( MAX_PATH, currDir );
	string testDir = currDir;
	testDir += "\\Test Data";
	string srfPath = testDir + "\\2sections.srf";

	MaxisLotTest t1;
	if ( !t1.SelectSrf( srfPath ))
		printf( "SelectSrf() failed\n" );

	printf( "Name: %s\n", t1.GetName().c_str());
	printf( "Number: %s\n", t1.GetNumber().c_str());
	printf( "Description: %s\n", t1.GetDescription().c_str());
	printf( "Floors %s and %d\n", t1.GetFloors().c_str(), t1.GetNumberOfFloors());

	printf( "Lodging: %d\n", t1.GetLodging());
	printf( "Food: %d\n", t1.GetFood());
	printf( "Gardening: %d\n", t1.GetGardening());
	printf( "Shopping: %d\n", t1.GetShopping());
	printf( "Pet Store: %d\n", t1.GetPetStore());
	printf( "Small Animal: %d\n", t1.GetSmallAnimal());
	printf( "Park: %d\n", t1.GetPark());
	printf( "Spa: %d\n", t1.GetSpa());
	printf( "Movie: %d\n", t1.GetMovie());
	printf( "Model: %d\n", t1.GetModel());
	printf( "Music: %d\n", t1.GetMusic());
	printf( "MagiCo: %d\n", t1.GetMagiCo());
	printf( "Rides: %d\n", t1.GetRides());

	printf( "IsSet: %d\n", t1.IsSet());
	printf( "Asset Type: %s\n", t1.GetAssetType().c_str());
	printf( "File Path: %s\n", t1.GetFilePath().c_str());
	printf( "Asset File Name: %s\n", t1.GetAssetFileName().c_str());
	printf( "Asset File Path: %s\n", t1.GetAssetFilePath().c_str());

	if ( !t1.SetGameEdition( "666" ))
		printf( "SetGameEdition() failed\n" );
	if ( !t1.SetFeatures( "69" ))
		printf( "SetFeatures() failed\n" );

	printf( "Features: %s\n", t1.GetFeatures().c_str());
	printf( "GameEdition: %s\n", t1.GetGameEdition().c_str());

	printf( "Xml:\n%s", t1.GetXmlDoc().c_str());

	return 0;
}


