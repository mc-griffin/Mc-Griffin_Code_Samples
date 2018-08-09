//=============================================================================
//
//    Name: LotFileSummaryStruct.h
//
// Purpose: Lot File summary record structure for web exchange
//
// Written by Colin Andrews, 5/2003
//
// Copyright (c) 2003 EA/Maxis, Inc. -- All Rights Reserved WorldWide.
//
#ifndef LOTFILESUMMARYSTRUCT_H
#define LOTFILESUMMARYSTRUCT_H


///////////////////////////////////////////////////////////////////////////////
// Include Files
//
#ifndef IGZEXPORT_H
   #include <IGZExport.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// Constants
//
const Uint32   kLotFileSummaryVersion  =  0; // lets just go sequentially with this

enum eZoneTypeFlags {
      kZoneTypeUndefined               =  0,
      kZoneTypeLowDensityResidential   =  1,
      kZoneTypeMediumDensityResidential=  2,
      kZoneTypeHighDensityResidential  =  3,
      kZoneTypeLowDensityCommercial    =  4,
      kZoneTypeMediumDensityCommercial =  5,
      kZoneTypeHighDensityCommercial   =  6,
      kZoneTypeResourceIndustrial      =  7,
      kZoneTypeLightIndustrial         =  8,
      kZoneTypeHeavyIndustrial         =  9,
      kZoneTypeMilitary                = 10,
      kZoneTypeAirport                 = 11,
      kZoneTypeSeaport                 = 12,
      kZoneTypeSpaceport               = 13,
      kZoneTypeLandfill                = 14,
      kZoneTypePloppedBuilding         = 15,  // This is what user-plopped buildings are zoned as.
      kZoneTypeCount                   = 16,
      kZoneBitUndefined                = (1<<kZoneTypeUndefined),
      kZoneBitLowDensityResidential    = (1<<kZoneTypeLowDensityResidential),
      kZoneBitMediumDensityResidential = (1<<kZoneTypeMediumDensityResidential),
      kZoneBitHighDensityResidential   = (1<<kZoneTypeHighDensityResidential),
      kZoneBitLowDensityCommercial     = (1<<kZoneTypeLowDensityCommercial),
      kZoneBitMediumDensityCommercial  = (1<<kZoneTypeMediumDensityCommercial),
      kZoneBitHighDensityCommercial    = (1<<kZoneTypeHighDensityCommercial),
      kZoneBitResourceIndustrial       = (1<<kZoneTypeResourceIndustrial),
      kZoneBitLightIndustrial          = (1<<kZoneTypeLightIndustrial),
      kZoneBitHeavyIndustrial          = (1<<kZoneTypeHeavyIndustrial),
      kZoneBitMilitary                 = (1<<kZoneTypeMilitary),
      kZoneBitAirport                  = (1<<kZoneTypeAirport),
      kZoneBitSeaport                  = (1<<kZoneTypeSeaport),
      kZoneBitSpaceport                = (1<<kZoneTypeSpaceport),
      kZoneBitLandfill                 = (1<<kZoneTypeLandfill),
      kZoneBitPloppedBuilding          = (1<<kZoneTypePloppedBuilding),
};

enum eWealthCompatibilityFlags {
      kWealthTypeLow    = 2,
      kWealthTypeMedium = 4,
      kWealthTypeHigh   = 8
};

// even though these are flags, I know of no situation in which you would have
// more than one of these set. This is just the way they are given to me.
// when this field is zero, the lot is a plopable and you should refer to the
// lot group for more detailed information
enum ePurposeCompatibilityFlags {
	  kPurposeTypeResidentialResidence    =   2,     
	  kPurposeTypeCommercialService       =   4,      
	  kPurposeTypeCommercialOffice        =   8,
	  kPurposeTypeUnused                  =  16,   // unused 
	  kPurposeTypeIndustrialAgriculture   =  32,   // $ only
	  kPurposeTypeIndustrialProcessing    =  64,   // $$ only
	  kPurposeTypeIndustrialManufacturing = 128,   // $$ only
	  kPurposeTypeIndustrialHighTech      = 256,   // $$$ only
};

enum eLotGroupIDS {
      kResidentialLotGroupId     = 0x1000,
      kCommercialLotGroupId      = 0x1001,
      kIndustrialLotGroupId      = 0x1002,
      kTransportationLotGroupId  = 0x1003,
      kUtilityLotGroupId         = 0x1004,
      kCivicLotGroupId           = 0x1005,
      kParkLotGroupId            = 0x1006,
      kRailLotGroupId            = 0x1300,
      kBusLotGroupId             = 0x1301,
      kSubwayLotGroupId          = 0x1302,
      kPowerLotGroupId           = 0x1400,
      kWaterLotGroupId           = 0x1401,
      kPoliceLotGroupId          = 0x1500,
      kJailLotGroupId            = 0x1501,
      kFireLotGroupId            = 0x1502,
      kSchoolLotGroupId          = 0x1503,
      kCollegeLotGroupId         = 0x1504,
      kLibraryLotGroupId         = 0x1505,
      kMuseumLotGroupId          = 0x1506,
      kHealthLotGroupId          = 0x1507,
      kAirportLotGroupId         = 0x1508,
      kSeaportLotGroupId         = 0x1509,
      kLandmarkLotGroupId        = 0x150A,
      kRewardLotGroupId          = 0x150B
};

enum eUserLotType {
      kUserModifiedLot = 0,
      kUserCustomLot
};

typedef Uint16 tUserLotType;

///////////////////////////////////////////////////////////////////////////////
// struct cLotFileSummary
//
struct cLotFileSummary {
   Uint32   mnSummaryVersion; // versioning for this structure - could end up being the version of the lot file too
   Uint32   mnZoneCompatibilityFlags; // see cISC4ZoneManager::ZoneBitsets   
   Uint32   mnWealthCompatibility;    // 2^cISC4BuildingOccupant::WealthType  
   Uint32   mnPurposeCompatibilityFlags; 
   Uint32   mnLotGroup;
   Uint32   mnGrowthStage; // pretty self explanatory: 1 - 8
   tUserLotType mUserLotType;
   Uint16   mnLotWidth;
   Uint16   mnLotDepth;
   Uint16   mnLotPropCount;  // includes props & flora
   Uint32   mnNumDependantPlugins; // number of null terminated plugin names following
};

#endif // LOTFILESUMMARYSTRUCT_H
