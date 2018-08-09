///////////////////////////////////////////////////////////////////////////////
// Name:    SimCity4Lot.cpp
// Purpose: Provides interface to an SC4 lot file (metadata and thumbnail)
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Compiler shutups
//
#pragma warning(disable: 4201) //nonstandard extension used : nameless struct/union
#pragma warning(disable: 4146) //unary minus operator applied to unsigned type, result still unsigned

#ifdef _MSC_VER
   #pragma warning(disable: 4239) //onversion from 'class cRZCmdLine' to 'class cRZCmdLine &'
   #pragma warning(disable: 4512) //assignment operator could not be generated
#endif

///////////////////////////////////////////////////////////////////////////////
// Include files
//

#include <SimCity4Lot.h>

#include <GZPersistResKeyList.h>
#include <GZDBSegmentPackedFile.h>
#include <IGZPersistBufferResource.h>
#include <RZSerializable.h>

#include <shlobj.h>

// constants
//const GZRESID kGZRESID_cGZPersistBufferResource         = 0x856ddbac;
const Uint32 kPreviewBitmapGroup          = 0xebdd10a4; // this comes from lot editor
const cGZPersistResourceKey   kLotFileSummaryKey(0x6be74c60, 0x6be74c60, 1); // all summary records will be written with this key

///////////////////////////////////////////////////////////////////////////////
// SimCity4Lot::SimCity4Lot
//
SimCity4Lot::SimCity4Lot()
{
	mpName = NULL;
	mpPluginArr = NULL;
	mpThumbnailPath = NULL;
	mpXmlPath = NULL;
	mpLotPath = NULL;
    mpTitle = NULL;
    mpDescription = NULL;
	mpLotFileSummary = NULL;
	mbCreateFilesInTmpFolder = false;
	mnErrCode = kNoError; 
	mnPreviewKeyInstanceID = 0;
}

bool SimCity4Lot::Init(const char* lotPath, bool createFilesInTmpFolder) {

	Close();

	mpName = NULL;
	mpPluginArr = NULL;
	mpThumbnailPath = NULL;
	mpXmlPath = NULL;
    mpTitle = NULL;
    mpDescription = NULL;
	mpLotFileSummary = NULL;
	mpLotPath = new char[MAX_PATH];
	strcpy(mpLotPath, lotPath);
	mbCreateFilesInTmpFolder = createFilesInTmpFolder;
	mnErrCode = kNoError;
	mnPreviewKeyInstanceID = 0;

	cGZDBSegmentPackedFile* const pDBSegment = new cGZDBSegmentPackedFile;
	pDBSegment->AddRef();
	pDBSegment->Init();
	cRZString pPath(mpLotPath);
	pDBSegment->SetPath(pPath);

	//Open for read-only access.
	if(! pDBSegment->Open(true, false))  {
		mnErrCode = kCannotOpenFile;
		pDBSegment->Release();
		return false;
	}

	// --- start: get preview key instance id ---

	cGZPersistResourceKeyList* pKeyList = new cGZPersistResourceKeyList;    
	pKeyList->AddRef();
	cGZPersistResourceKeyFilterByTypeAndGroup* pKeyFilter = new cGZPersistResourceKeyFilterByTypeAndGroup(kGZRESID_cGZPersistBufferResource /*type id*/, kPreviewBitmapGroup /*group id*/);
	pKeyFilter->AddRef();
	pDBSegment->GetResourceKeyList(pKeyList, pKeyFilter);
        
	// we're guaranteed there will be only one preview image per lot, so just use index 0
	if (pKeyList->Size() != 1) {
		mnErrCode = kTooManyPreviewImages;
		pDBSegment->Close();
		pDBSegment->Release();
		pKeyList->Release();
		pKeyFilter->Release();
		return false;
	}

	cGZPersistResourceKey lotPreviewKey = pKeyList->GetKey(0);
	mnPreviewKeyInstanceID = lotPreviewKey.mInstance;
	pKeyList->Release();
	pKeyFilter->Release();

	// --- done ----------------------------------

	unsigned long recordSize, bytesRead;
	
	// first get size
	recordSize = pDBSegment->DoReadRecord(kLotFileSummaryKey, NULL, bytesRead);

	if (recordSize == 0) {
		mnErrCode = kRecordSizeZero;
		pDBSegment->Close();
		pDBSegment->Release();
		return false;
	}

	mpLotFileSummary = (cLotFileSummary*) malloc(recordSize);

	if (! pDBSegment->DoReadRecord(kLotFileSummaryKey, (void*) mpLotFileSummary, recordSize)) {
		mnErrCode = kProblemReadingRecord;
		pDBSegment->Close();
		pDBSegment->Release();
		return false;
	}

	if (mpLotFileSummary->mnSummaryVersion < 1) {
		mnErrCode = kDeprecatedFileVersion;
		pDBSegment->Close();
		pDBSegment->Release();
		return false;
	}

	int numDependantPlugins = mpLotFileSummary->mnNumDependantPlugins;
	mpPluginArr = new char*[numDependantPlugins];

	cRZSerializableStream s(/*isLittleEndian*/ true);
	// stream starts after lot file summary struct and goes till end of record
	s.SetVoid(mpLotFileSummary + 1, recordSize - sizeof(cLotFileSummary));

	// for some reason the DecodeStringLength method call within
	// GetRZCharStr doesn't read in an entire Uint32; rather it reads
	// till it hits 0 and assumes that's the length.  can that be right?
	// anyway for now use the uglier GetVoid
	Uint32 size;
	s.GetUint32(size);
	mpName = new char[size + 1];
	mpName[size] = 0;
	s.GetVoid((void*)mpName, size);
	//s.GetRZCharStr(mpName, MAX_PATH);
	for (int i = 0; i < numDependantPlugins; i++) {
		s.GetUint32(size);
		mpPluginArr[i] = new char[size + 1];
		(mpPluginArr[i])[size] = 0;
		s.GetVoid((void*)mpPluginArr[i], size);
		//s.GetRZCharStr(mpPluginArr[i], MAX_PATH);
	}
	
	pDBSegment->Close();
	pDBSegment->Release();

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// SimCity4Lot::~SimCity4Lot
//
bool SimCity4Lot::Close() {
	if (mpName) {
		delete[] mpLotPath;
		mpLotPath = NULL;
	}
	if (mpPluginArr) {
		delete[] mpPluginArr; // APP check, does this recursively delete all the char*'s?
		mpPluginArr = NULL;
	}
	if (mpThumbnailPath) {
		delete[] mpThumbnailPath;
		mpThumbnailPath = NULL;
	}
	if (mpXmlPath) {
		delete[] mpXmlPath;
		mpXmlPath = NULL;
	}
	if (mpLotPath) {
		delete[] mpXmlPath;
		mpXmlPath = NULL;
	}
   if (mpTitle) {
		delete[] mpTitle;
		mpTitle = NULL;
	}
   if (mpDescription) {
		delete[] mpDescription;
		mpDescription = NULL;
	}
	if (mpLotFileSummary) {
		free((void*) mpLotFileSummary);
		mpLotFileSummary = NULL;
	}
	return true;
}




///////////////////////////////////////////////////////////////////////////////
// SimCity4Lot::~SimCity4Lot
//
SimCity4Lot::~SimCity4Lot(){
	// call close here?
}


///////////////////////////////////////////////////////////////////////////////
// SimCity4Lot::ExtractLotThumbnail
//
bool SimCity4Lot::ExtractLotThumbnail(){

	bool success = false;
	mnErrCode = kNoError;

	cGZDBSegmentPackedFile* const pDBSegment = new cGZDBSegmentPackedFile;
	pDBSegment->AddRef();
	pDBSegment->Init();
	cRZString pPath(mpLotPath);
	pDBSegment->SetPath(pPath);

	//Open for read-only access.
	if(! pDBSegment->Open(true, false)) {
		mnErrCode = kCannotOpenFile;
		pDBSegment->Release();
		return false;
	}

	cGZPersistResourceKey lotPreviewKey(kGZRESID_cGZPersistBufferResource /*type id*/, kPreviewBitmapGroup /*group id*/, mnPreviewKeyInstanceID);
	
	// build final png path
	cRZString psDrive;
	cRZString psDirectory; 
	cRZString psFileName;
	cRZString psExtension;

	// assuming init was called successfully, pPath is good
	Splitpath(pPath, &psDrive, &psDirectory, &psFileName, &psExtension);

	cRZString finalPngPath;

	if (mbCreateFilesInTmpFolder) {
		// APP only use the psFilename part...
		char* tmpPath = new char[_MAX_PATH]; 
		int tmpPathSize = GetTempPath(_MAX_PATH, tmpPath);
		if (tmpPathSize > _MAX_PATH) {
			mnErrCode = kTmpPathTooLong;
			pDBSegment->Close();
			pDBSegment->Release();
			delete[] tmpPath;
			return false; 
		}
		finalPngPath = cRZString(tmpPath) + psFileName + ".png";
		delete[] tmpPath;
	}
	else {
		finalPngPath = psDrive + psDirectory + "\\" + psFileName + ".png";
	}

	// need this for teleporter / browser
	mpThumbnailPath = new char[finalPngPath.Strlen()+1];
	strcpy(mpThumbnailPath, finalPngPath.ToChar());

	// finally get the thumbnail, put it at finalPngPath
	if(!pDBSegment->DoReadRecord(lotPreviewKey, finalPngPath)) {
		mnErrCode = kProblemReadingRecord; 
		pDBSegment->Close();
		pDBSegment->Release();
		return false;
	}
		
	pDBSegment->Close();
	pDBSegment->Release();

	// success!
	return true;
}

bool SimCity4Lot::CreateXMLFile() {

	mnErrCode = 0;

	cRZString psDrive;
	cRZString psDirectory; 
	cRZString psFileName;
	cRZString psExtension;

	// assuming init was called successfully, mpLotPath is good
	Splitpath(mpLotPath, &psDrive, &psDirectory, &psFileName, &psExtension);

	cRZString finalXmlPath;

	if (mbCreateFilesInTmpFolder) {
		// APP only use the psFilename part...
		char* tmpPath = new char[_MAX_PATH]; 
		int tmpPathSize = GetTempPath(_MAX_PATH, tmpPath);
		if (tmpPathSize > _MAX_PATH) {
			mnErrCode = kTmpPathTooLong;
			RZ_ASSERT(false);
			return false;
		}
		finalXmlPath = cRZString(tmpPath) + psFileName + ".xml";
		delete[] tmpPath;
	} else {
		finalXmlPath = psDrive + psDirectory + "\\" + psFileName + ".xml";
	}

	mpXmlPath = new char[finalXmlPath.Strlen()+1];
	strcpy(mpXmlPath, finalXmlPath.ToChar());

	fstream myFile;
	myFile.open(finalXmlPath.ToChar(), ios::out);

	myFile << "<?xml version=\"1.0\"?>\r\n\r\n";
	myFile << "<Lot>\r\n";
	myFile << "<LotName>" << mpName << "</LotName>\r\n";
	myFile.setf ( ios::hex, ios::basefield );  // set hex as the basefield          
	myFile << "<GUID>" << mnPreviewKeyInstanceID << "</GUID>\r\n";
	myFile.setf ( ios::dec, ios::basefield ); // set dec as the basefield
	myFile << "<ZoneCompatibility>" << GetZoneCompatibility() << "</ZoneCompatibility>\r\n";
	myFile << "<WealthCompatibility>" << GetWealthCompatibility() << "</WealthCompatibility>\r\n";
	myFile << "<PurposeCompatibility>" << GetPurposeCompatibility() << "</PurposeCompatibility>\r\n";
	myFile << "<Group>" << GetGroup() << "</Group>\r\n";
	myFile << "<GrowthStage>" << GetGrowthStage() << "</GrowthStage>\r\n";
	myFile << "<UserLotType>" << GetUserLotType() << "</UserLotType>\r\n";
	myFile << "<Width>" << GetWidth() << "</Width>\r\n";
	myFile << "<Depth>" << GetDepth() << "</Depth>\r\n";
	myFile << "<PropCount>" << GetPropCount() << "</PropCount>\r\n";
	myFile << "<NumDependantPlugins>" << GetNumDependantPlugins() << "</NumDependantPlugins>\r\n";
	myFile << "<Plugins>\r\n";
	for (Uint32 i = 0; i < GetNumDependantPlugins(); i++) {
		myFile << "<Plugin>" << mpPluginArr[i] << "</Plugin>\r\n"; 
	}
	myFile << "</Plugins>\r\n";
   if (mpTitle)
	  myFile << "<Title>" << mpTitle << "</Title>\r\n";
   else
	  myFile << "<Title>Untitled Lot</Title>\r\n";
   if (mpDescription)
	  myFile << "<Description>" << mpDescription << "</Description>\r\n";
   else
	  myFile << "<Description>No Description</Description>\r\n";
	myFile << "</Lot>\r\n";

	myFile.close();
	
	return true;
}

void SimCity4Lot::SetTitle(const char* title) {
   if (mpTitle) {
      assert(false); // shouldn't be setting title more than once
		delete[] mpTitle;
		mpTitle = NULL;
	}
   mpTitle = new char[strlen(title) + 1];
	strcpy(mpTitle, title);
}

void SimCity4Lot::SetDescription(const char* description) {
   if (mpDescription) {
      assert(false); // shouldn't be setting description more than once
		delete[] mpDescription;
		mpDescription = NULL;
	}
   mpDescription = new char[strlen(description) + 1];
	strcpy(mpDescription, description);
}
///////////////////////////////////////////////////////////////////////////////
//EOF
///////////////////////////////////////////////////////////////////////////////



