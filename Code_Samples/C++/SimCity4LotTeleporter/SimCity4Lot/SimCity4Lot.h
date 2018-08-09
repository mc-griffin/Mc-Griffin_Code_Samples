///////////////////////////////////////////////////////////////////////////////
// Name:    SimCity4Lot.h
// Purpose: 
///////////////////////////////////////////////////////////////////////////////

#ifndef _SIMCITY4LOT_H_
#define _SIMCITY4LOT_H_

#include <LotFileSummaryStruct.h>

// errorCodes
enum eLotErrorCodes {
	kNoError				= 0,
	kCannotOpenFile			= 1,
	kRecordSizeZero			= 2,
	kProblemReadingRecord	= 3,
	kDeprecatedFileVersion	= 4,
	kTooManyPreviewImages	= 5,
	kTmpPathTooLong			= 6
};
 
///////////////////////////////////////////////////////////////////////////////
// class SimCity4Lot
//
class SimCity4Lot {
public:
	SimCity4Lot(); // APP add init function
	~SimCity4Lot();
	bool Init(const char* lotPath, bool createFilesInTmpFolder=true);
	bool Close();

   // -- lot data accessors ---------------------
	Uint32 GetZoneCompatibility() { return mpLotFileSummary->mnZoneCompatibilityFlags; }
	Uint32 GetWealthCompatibility() { return mpLotFileSummary->mnWealthCompatibility; }
	Uint32 GetPurposeCompatibility() { return mpLotFileSummary->mnPurposeCompatibilityFlags; }
	Uint32 GetGroup() { return mpLotFileSummary->mnLotGroup; }
	Uint32 GetGrowthStage() { return mpLotFileSummary->mnGrowthStage; }
	tUserLotType GetUserLotType() { return mpLotFileSummary->mUserLotType; }
	Uint16 GetWidth() { return mpLotFileSummary->mnLotWidth; }
	Uint16 GetDepth() { return mpLotFileSummary->mnLotDepth; }
	Uint16 GetPropCount() { return mpLotFileSummary->mnLotPropCount; }
	Uint32 GetNumDependantPlugins() { return mpLotFileSummary->mnNumDependantPlugins; }
	Uint32 GetErrCode() { return mnErrCode; }
	const char* GetName() { return mpName; }
	const char* GetPlugin(int i) { return mpPluginArr[i]; }
	// -- end lot data accessors ---------------------

	bool ExtractLotThumbnail();
	bool CreateXMLFile();

	const char* GetThumbnailPath() { return mpThumbnailPath; }
	const char* GetXmlPath() { return mpXmlPath; }
	const char* GetLotPath() { return mpLotPath; }
   
   void   SetTitle(const char* title);
   void   SetDescription(const char* description);

private:
	Uint32 mnErrCode; // used to signal problems when returning false from Init(), etc
	Uint32 mnPreviewKeyInstanceID; // we need to send this up to recreate filename when later downloading.
								   // filename = lotName_InstanceID.  see ColinA for motivation behind this
	char*  mpName;
	char** mpPluginArr; // array of plugin names
	char*  mpThumbnailPath;
	char*  mpXmlPath;
	char*  mpLotPath;
    char*  mpTitle;
    char*  mpDescription;
	bool   mbCreateFilesInTmpFolder;
	cLotFileSummary *mpLotFileSummary;

};
/////////////////////////////////////////////////////////////////////////



#endif //sentry








