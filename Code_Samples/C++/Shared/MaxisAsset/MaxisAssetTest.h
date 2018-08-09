// MaxisLotTest.h
//
// This file is not to be used in production code, just in testing.
// I assume that only MaxisAsset.cpp #includes this file.

#include "MaxisLot.h"


// MaxisLotTest does not test AllNeededFilesExist()
class MaxisLotTest : public MaxisLot
{
public:
	MaxisLotTest() : MaxisLot( "cmt", "CMT" )
	{
		_sectionNames.push_back( "CommunityLot" );
	}

	virtual bool AllNeededFilesExist() const { return true; }
};

