// SimCity4CityTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SimCity4Lot.h"
#include <iostream.h>

#include <string>
#include <fstream>
using namespace std;

int main(int argc, char* argv[])
{
	int resultCode;
	if (argc < 2) {
		printf("usage: SimCity4LotTest <lot_path>");
		resultCode = 1;
	} else {
		SimCity4Lot lot;
		lot.Init(argv[1], false); 
		lot.CreateXMLFile();
		lot.ExtractLotThumbnail();
		ifstream i(lot.GetXmlPath());
		char xmlBuf[5000];
		i.read(xmlBuf, 5000);
		xmlBuf[i.gcount()] = 0; // null terminate
		printf("Lot file %s was successfully opened.\n\n", lot.GetLotPath());
		printf("Thumbnail path: %s\n\n", lot.GetThumbnailPath());
		printf("Lot xml: \n%s\n", xmlBuf);
		resultCode = 0;
	}
	return resultCode;
}

