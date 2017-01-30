// AdApi.cpp : Defines the entry point for the DLL application.
//
#include "AdApi.h"
#include "Utils.h"
#include "AdProtocol.h"
#include "AdDisplay.h"
#include "AdEngine.h"

using namespace WONAPI;
using namespace std;

// Constants
const char* Default_Logfile_Path = "C:\\logs\\AdApi.log";

// Note: This dll is designed to be implicitly loaded so nothing should 
// be put here unless that changes
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

AdApi::AdApi() :
	mAdDisplayP(NULL),
	mAdEngineP(NULL),
	mInitialized(false)
{

	// Open log file
	INIT_LOG(Default_Logfile_Path);
}

AdApi::~AdApi()
{
	// Make sure system is shut down if not already so
	Shutdown();

	// Close log file
	SHUTDOWN_LOG();
}

//////////////////////////////////////////////////////////////////////
// Public Interface for Ad API
//////////////////////////////////////////////////////////////////////

AdApi::AdResult 
AdApi::Init( AdApi::AdProvider theAdProvider, // Might want to remove this.... may be able to determine this based on name
			const char* theProductName,
			NetCommInterface* theNetCommInterfaceP,
			const char* theCachePath, 
			int theMaxDownloadRate)
{
	if (!theProductName) 
	{
		LOG("AdApi::Init Error:  No product name supplied");
		return AdResult_Failure;
	}
	// ** Alow this for now
	//if (!theNetCommInterfaceP)
	//{
	//	LOG("AdApi::Init Error:  NetCommInterface is missing");
	//	return AdResult_Failure;
	//}
	string aCachePath;
	if (theCachePath)
	{
		aCachePath = theCachePath;
	}
	else // Use default cache path
	{
		if (WONAPI::GetModulePath(aCachePath))
		{
			aCachePath += "AdCache";
		}
		else // Failed
		{
			LOG("AdApi::Init Error:  No Ad Cache path supplied and unable to get default");
			return AdResult_Failure;
		}
	}

	AdOptions::Init();

	mAdDisplayP = new AdDisplay();
	mAdEngineP = new AdEngine(theNetCommInterfaceP, mAdDisplayP);
	if (mAdEngineP->Init(theAdProvider, theProductName, theMaxDownloadRate, aCachePath))
	{
		LOG("AdApi::Init Ad Engine intitialization completed successfully");
	}
	else // Failed
	{
		LOG("AdApi::Init Ad Engine intitialization failed");
		return AdResult_Failure;
	}
		
	LOG("AdApi::Init Initialization completed successfully");
	mInitialized = true;
	return AdResult_Success;
}

	
AdApi::AdResult 
AdApi::Shutdown() 
{
	mInitialized = false;
	delete mAdEngineP;
	mAdEngineP = NULL;
	delete mAdDisplayP;
	mAdDisplayP = NULL;

	AdOptions::Shutdown();

	return AdResult_Success;
}

bool 
AdApi::SetLogFile(const char* theLogFilePath)
{
#ifdef LOGGING
	return SET_LOGFILE(theLogFilePath);
#else
	return false;
#endif
}

//////////////////////////////////////////////////////////////////////
// Download functions
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Setting callbacks
//////////////////////////////////////////////////////////////////////
	
AdApi::AdResult 
AdApi::SetPaintCallback(PaintCallback thePaintCallbackP, void* theDataPaintP)
{
	if (!mInitialized)
	{
		LOG("AdApi::SetPaintCallback Error:  Not Initialized");
		return AdResult_NotInitialized;
	}
	if (!mAdDisplayP)
	{
		LOG("AdApi::SetPaintCallback Error:  Display module not present");
		return AdResult_Failure;
	}
	if (!mAdEngineP)
	{
		LOG("AdApi::SetPaintCallback Error:  Engine module not present");
		return AdResult_Failure;
	}

	mAdDisplayP->SetPaintCallback(thePaintCallbackP, theDataPaintP);

	// Display default ad
	mAdEngineP->DisplayDefaultAd();

	return AdResult_Success;
}

//////////////////////////////////////////////////////////////////////
// Display functions
//////////////////////////////////////////////////////////////////////

AdApi::AdResult 
AdApi::AdClicked()
{
	if (!mInitialized)
	{
		LOG("AdApi::AdClicked Error:  Not Initialized");
		return AdResult_NotInitialized;
	}
	if (!mAdEngineP)
	{
		LOG("AdApi::AdClicked Error:  Engine Module is not present");
		return AdResult_Failure;
	}

	// Launch browser
	if (!mAdEngineP->LaunchAdDestination())
	{
		LOG("AdApi::AdClicked Error: Unable to launch default browser");
		return AdResult_BrowserLaunchFailure;
	}

	return AdResult_Success;
}

// This is an example of an exported variable
//ADAPI_API int nAdApi=0;

// This is an example of an exported function.
//extern "C" ADAPI_API int fnAdApi(void)
//{
//	return 42;
//}

AdApi::AdResult 
AdApi::GetDisplayImage(HDC hDC, HBITMAP& theBitmapHR)
{
	if (!mInitialized)
	{
		LOG("AdApi::GetDisplayImage Error:  Not Initialized");
		return AdResult_NotInitialized;
	}
	if (!mAdDisplayP)
	{
		LOG("AdApi::GetDisplayImage Error:  Display module not present");
		return AdResult_Failure;
	}

	// Get current bitmap image to display
	if (mAdDisplayP->GetDisplayImage(hDC, theBitmapHR))
	{
		return AdResult_Success;
	}
	else
	{
		LOG("AdApi::GetDisplayImage Error:  Unable to get Display image");
		return AdResult_Failure;
	}
}

void
AdApi::ReleaseDisplayImage(HBITMAP theBitmapH)
{
	if (!mInitialized)
	{
		LOG("AdApi::GetDisplayImage Error:  Not Initialized");
		return;
	}
	if(mAdDisplayP)
		mAdDisplayP->ReleaseDisplayImage(theBitmapH);
	else
		LOG("AdApi::ReleaseDisplayImage Error:  Protocol Module not present");
}

AdApi::AdResult 
AdApi::GetDisplayImage(BYTE* &theRawPR, int& theWidthR, int& theHeightR)
{
	if (!mInitialized)
	{
		LOG("AdApi::GetDisplayImage Error:  Not Initialized");
		return AdResult_NotInitialized;
	}
	if (!mAdDisplayP)
	{
		LOG("AdApi::GetDisplayImage Error:  Display module not present");
		return AdResult_Failure;
	}

	// Get current bitmap image to display
	if (mAdDisplayP->GetDisplayImage(theRawPR, theWidthR, theHeightR))
	{
		return AdResult_Success;
	}
	else
	{
		LOG("AdApi::GetDisplayImage Error:  Unable to get Display image");
		return AdResult_Failure;
	}
}

void
AdApi::ReleaseDisplayImage(BYTE* theRawP)
{
	if (!mInitialized)
	{
		LOG("AdApi::GetDisplayImage Error:  Not Initialized");
		return;
	}
	if(mAdDisplayP)
		mAdDisplayP->ReleaseDisplayImage(theRawP);
	else
		LOG("AdApi::ReleaseDisplayImage Error:  Protocol Module not present");
}

void
AdApi::AllowSceneSkipping(bool isSceneSkippingAllowed /* = true */)
{
	if (!mInitialized)
	{
		LOG("AdApi::GetDisplayImage Error:  Not Initialized");
		return;
	}
	if(mAdDisplayP)
		mAdDisplayP->AllowSceneSkipping(isSceneSkippingAllowed);
	else
		LOG("AdApi::AllowSceneSkipping Error:  Protocol Module not present");
}

bool
AdApi::IsSceneSkippingAllowed(void)
{
	if (!mInitialized)
	{
		LOG("AdApi::GetDisplayImage Error:  Not Initialized");
		return true;
	}
	if(mAdDisplayP)
		return mAdDisplayP->IsSceneSkippingAllowed();
	else
		LOG("AdApi::IsSceneSkippingAllowed Error:  Protocol Module not present");
	return true;
}


