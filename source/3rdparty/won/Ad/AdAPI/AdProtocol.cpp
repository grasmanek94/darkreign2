#include "AdProtocol.h"
#include "AdDisplay.h"
#include "Utils.h"

using namespace WONAPI;
using namespace std;

AdProtocol::AdProtocol(NetCommInterface* theNetCommInterfaceP, unsigned long theMaxDownloadRate, AdEngine* theAdEngineP, const std::string& theCachePath) :
	mNetCommInterfaceP(theNetCommInterfaceP),
	mMaxDownloadRate(theMaxDownloadRate),
	mAdEngineP(theAdEngineP),
	mAdCacheP(NULL),
	mCurrAdDest(),
	mCurrAdPath()
{
	if (mNetCommInterfaceP && mAdEngineP)
	{
		unsigned long aFileUsageLifespan;
		unsigned long aFileExistenceLifespan;
		unsigned long aMaxFilesInCache;

		// Get these things from options in Registry...
		AdOptions::GetLongOption(OptionName_FileUsageLifespan, OptionDefault_FileUsageLifespan, aFileUsageLifespan);
		AdOptions::GetLongOption(OptionName_FileExistenceLifespan, OptionDefault_FileExistenceLifespan, aFileExistenceLifespan);
		AdOptions::GetLongOption(OptionName_MaxFilesInCache, OptionDefault_MaxFilesInCache, aMaxFilesInCache);

		mAdCacheP = new AdCache(theCachePath, aFileUsageLifespan, aFileExistenceLifespan, aMaxFilesInCache);
	}
	else
	{
		LOG("AdProtocol::AdProtocol Error:  Null pointers passed into contstructor");
	}
}

AdProtocol::~AdProtocol()
{
	delete mAdCacheP;
	mAdCacheP = NULL;
}

// Public methods


// This method should be overriden for each protocol since each
// method of downloading will be different. 
// This function can block since ad download will not be done in the main thread
bool
AdProtocol::DownloadAd() 
{
	// Open connection
	//mNetCommInterfaceP->Connect();

	// Request ad file name and image source
	//SendAdFileNameRequest();

	//mCurrAdPath = ?

	// Check ad cache for file

	// ...


	// Create Ad file

	// Set boolean saying ad is ready to display
	return true;
}

bool 
AdProtocol::CreateAdFile(const RawBuffer& theContent)
{
	// Might want to clear out cache first

	// This was determined at download time
	//mCurrAdPath

	if (mCurrAdPath.length() == 0)
	{
		return false;
	}

	// Create the folder first
	if (!CreateFullPath(mCurrAdPath))
	{
		return false;
	}

	// Create the file
	HANDLE aAdFile = ::CreateFile(mCurrAdPath.c_str(), GENERIC_WRITE, 0, NULL, 
									CREATE_ALWAYS, 0, NULL);
	//assert(aAdFile != INVALID_HANDLE_VALUE);
	if (aAdFile == INVALID_HANDLE_VALUE)
	{
		LOG("Failed to create ad file " << mCurrAdPath.c_str() << " Err: "<< ::GetLastError());
		return false;
	}

	// Write the file
	DWORD aBytesWritten = 0;
	BOOL aSuccess = ::WriteFile(aAdFile, theContent.c_str(), theContent.length(), &aBytesWritten, NULL);
	::CloseHandle(aAdFile);
	//assert(aSuccess);
	//assert(aBytesWritten == aContent.length());
	if (!aSuccess || aBytesWritten != theContent.length())
	{
		LOG("Failed to write data to ad file " << mCurrAdPath.c_str() << " Err: " << ::GetLastError());
		::DeleteFile(mCurrAdPath.c_str());
		return false;
	}

	return true;
}

// Create full filename with path
const string 
AdProtocol::GenerateAdPath(const string& theFileName)
{
	string aFileName = theFileName;
	AddReplaceFileExt(aFileName);

	// Get base directory
	string aAdPath = mAdCacheP->GetCachePath();

	// Ad filename
	aAdPath += aFileName;

	return aAdPath; 
}

void 
AdProtocol::AddReplaceFileExt(string& theFilePath)
{
	// Replace the extension if it exists with the proper extension
	string::size_type aDot = theFilePath.rfind('.');
	if (aDot != string::npos) // There currently exists an extention
	{
		theFilePath.resize(aDot+1);
	}
	else // Currently no extention
	{
		theFilePath += ".";
	}
	
	// ** Currently we only support one file type.  This will have to be changed when we support more
	theFilePath += "gif";
}




