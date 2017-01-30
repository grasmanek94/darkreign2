// AdEngine.cpp

#include "AdEngine.h"
#include "Utils.h"

using namespace WONAPI;
using namespace std;

const std::string AdEngine::Default_AdImageFileName = "DefaultAd.gif";
const std::string AdEngine::Default_AdLinkFileName = "DefaultLink.txt";

// Constructor/Destructor
AdEngine::AdEngine(NetCommInterface* theNetCommInterfaceP,  AdDisplay* theAdDisplayP) :
	mNetCommInterfaceP(theNetCommInterfaceP),
	mAdDisplayP(theAdDisplayP),
	mAdProtocolP(NULL),
	mStartDownloadOptionsEvent(NULL),
	mStartDownloadEvent(NULL),
	mLastDisplayTime(0),
	mOptionsDoneEvent(NULL),
	mOptionsResult(false),
	mAdReady(false),
	mOptionsDownloaded(false),
	mProductName(),
	mShowingDefaultAd(false)
{
	mStartDownloadOptionsEvent = CreateEvent(NULL, false, false, NULL);
	mStartDownloadEvent = CreateEvent(NULL, false, false, NULL);
	mOptionsDoneEvent = CreateEvent(NULL, false, false, NULL);
}

AdEngine::~AdEngine()
{
	//SetEvent(getStopEvent()); // Do this here so item waiting does not have events closed it is waiting on...
	stopThread();
	if (mStartDownloadOptionsEvent)
	{
		CloseHandle(mStartDownloadOptionsEvent);
		mStartDownloadOptionsEvent = NULL;
	}
	if (mStartDownloadEvent)
	{
		CloseHandle(mStartDownloadEvent);
		mStartDownloadEvent = NULL;
	}
	if (mOptionsDoneEvent)
	{
		CloseHandle(mOptionsDoneEvent);
		mOptionsDoneEvent = NULL;
	}
	LOG("AdEngine::AdEngine Download thread stopping");
	delete mAdProtocolP;
	mAdProtocolP = NULL;
}

bool
AdEngine::Init(AdApi::AdProvider theAdProvider, const string& theProductName, unsigned long theMaxDownloadRate, const string& theCachePath)
{
	mAdDisplayDuration = 60*1000; // ** To Do - This is an option that is downloaded default is 60 sec
	mLastDisplayTime = 0;
	mAdReady = false;
	mOptionsDownloaded = false;
	mProductName = theProductName;

	// Display default ad *** MOVED TO set paint call back
	//DisplayDefaultAd();

	// Sets event that tells engine to download options
	//SetEvent(mStartDownloadOptionsEvent);

	LOG("AdEngine::Init Thread starting");
	startThread(); // Start the download thread

	// ** Move this to completion function
	/*switch (theAdProvider)
	{
		case AdApi::AdProvider_24x7:
		{
			mAdProtocolP = new AdProtocol(mNetCommInterfaceP, theMaxDownloadRate, this, theCachePath);
		}
		break;
		case AdApi::AdProvider_Invalid:
		default:
			LOG("AdEngine::Init Error:  Invalid AdProvider supplied");
			return false;
		break;
	}*/

	return true;
}

void
AdEngine::StartAdDownload()
{
	// Sets event that starts downloading the ad
	SetEvent(mStartDownloadEvent);
}

// Launch browser with desination of ad currently being displayed.
bool
AdEngine::LaunchAdDestination()
{
	string aAdDestination;
	if (mShowingDefaultAd)
	{
		// Use default destination
		if (!GetDefaultDestination(aAdDestination))
		{
			LOG("AdEngine::LaunchAdDestination Error: Unable to get default destination");
			return false;
		}
	}
	else if (mAdProtocolP != NULL)
	{
		// Get destination downloaded
		aAdDestination = mAdProtocolP->GetAdDestination();
	}
	if (aAdDestination.length() != 0)
	{
		// Launch default browser
		// ** Might want to make sure that string is a URL
		ShellExecute(NULL, NULL, aAdDestination.c_str(), NULL, NULL, SW_SHOW);
	}
	else
	{
		LOG("AdEngine::LaunchAdDestination Error: Launch of ad destination attemped on empy destination");
		return false;
	}
	return true;
}


// Notifies the engine that an ad is ready to be displayed..
// Note: this function is designed to be called from another thread so must be thread safe
bool
AdEngine::DisplayAd(const string& theAdPath)
{
	if (mAdDisplayP->LoadNewImage(theAdPath))
	{
		mShowingDefaultAd = false;
		// reset timer for display of next ad
		mLastDisplayTime = GetTickCount();

		// ** To Do Add ad file to cache if need be

		// Get ready for next ad download
		mAdReady = false;
		SetEvent(mStartDownloadEvent);
		return true;
	}
	else
	{
		LOG("AdEngine::DisplayAd Error:  Error trying to display ad");
		return false;	
	}
}

void
AdEngine::DisplayDefaultAd()
{
	string aDefaultAdPath;
	if (GetModulePath(aDefaultAdPath))
	{
		aDefaultAdPath += Default_AdImageFileName;
		if (WONAPI::PathExists(aDefaultAdPath) && mAdDisplayP->LoadNewImage(aDefaultAdPath))
		{
			mShowingDefaultAd = true;
		}
		else
		{
			LOG("AdEngine::DisplayDefaultAd Error:  Unable to find default ad");
		}
	}
	else
	{
		LOG("AdEngine::DisplayDefaultAd Error:  Unable to find default ad");
	}
}

bool
AdEngine::GetDefaultDestination(string& theDefaultDestination)
{
	string aDefaultAdPath;
	if (GetModulePath(aDefaultAdPath))
	{
		aDefaultAdPath += Default_AdLinkFileName;
		char aBuffer[256];
		if (GetPrivateProfileString("AdDisplay", "DefaultLink", "", aBuffer, 256, aDefaultAdPath.c_str()))
		{
			theDefaultDestination = aBuffer;
			return true; // Succeeded
		}
	}

	return false;
}

void
AdEngine::OptionsDone(bool theResult)
{
	mOptionsResult = theResult;
	SetEvent(mOptionsDoneEvent);
}

// Overloaded Methods
int
AdEngine::threadProcess()
{
	HANDLE aWaitArray[3] = {getStopEvent(), getStartDownloadOptionsEvent(), getStartDownloadEvent()};
	while(true)
	{
		// If multiple events get signaled 
		DWORD aResult = WaitForMultipleObjects(3, aWaitArray, false, CalculateTimeUntilAdDisplay());

		switch(aResult)
		{
			case WAIT_FAILED: // Error
			{
				LOG("AdProtocol::threadProcess Error:  Wait failed");
				return 1;
			}
			break;
			case WAIT_TIMEOUT: // Time to display ad
			{
				// Display ad
				// Ad is ready and it is time to display it
				if (mAdReady)
				{
					DisplayAd(mAdProtocolP->GetCurrentAdPath());
				}
			}
			break;
			// Stop requested
			case WAIT_OBJECT_0:
			{
				LOG("AdProtocol::threadProcess Stop thread requested");
				return 0;
			}
			break;
			// Download options..
			case WAIT_OBJECT_0+1:
			{
				GetAdServOptions();
			}
			break;
			// Download the ad..
			case WAIT_OBJECT_0+2:
			{
				LOG("AdProtocol::threadProcess Ad download started");
				mAdReady = mAdProtocolP->DownloadAd();
				LOG("AdProtocol::threadProcess Ad download complete");
				
			}
			break;	
			default:
				LOG("AdProtocol::threadProcess Error: Unexpected error has occured");
			break;
		}
	}
}

// Private methods

DWORD
AdEngine::CalculateTimeUntilAdDisplay()
{
	// ***** This should be infinite until options and ad are downloaded...

	/*DWORD aElapsed = GetTickCount() - mLastDisplayTime;

	// This is done this way so that the 49 day wrap arround will not affect the calculation
	if (aElapsed >= mAdDisplayDuration)
	{
		return 0; // It is time or past time to show ad
	}
	else
	{
		return mAdDisplayDuration - aElapsed;
	}*/

	return INFINITE;
}

// Get ad serving options from Won server
// This function makes blocking network calls
bool
AdEngine::GetAdServOptions()
{
	if (!mOptionsDownloaded)
	{
		string aDestPath;
		string aServerPath = "AdApi\\";
		if (GetModulePath(aDestPath))
		{
			aServerPath += mProductName;
			// Warning:  There will be a problem here if AdEngine is deleted before the callback is called.
			mOptionsResult = false;
			mNetCommInterfaceP->HTTPGet("www.won.net", aServerPath.c_str(), aDestPath.c_str(), GetOptionsCallback, this); 

			WaitForSingleObject(mOptionsDoneEvent, 30000); // Wait for file to come back

			if(mOptionsResult)
			{
				mOptionsDownloaded = true;
			}
		}
	}
	return mOptionsDownloaded;
}

// Callbacks

void
__stdcall WONAPI::GetOptionsCallback(bool theResult, void* theDataP)
{
	((AdEngine*)theDataP)->OptionsDone(theResult);
}



