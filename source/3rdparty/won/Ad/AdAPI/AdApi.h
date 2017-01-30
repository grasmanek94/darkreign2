#ifndef _ADAPI_H
#define _ADAPI_H

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ADAPI_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ADAPI_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef ADAPI_EXPORTS
#define ADAPI_API __declspec(dllexport)
#else
#define ADAPI_API __declspec(dllimport)
#endif

#include <string>
#include <windows.h>
#include "NetCommInterface.h"

namespace WONAPI
{

// Prototypes
class AdDisplay;
class AdEngine;

// This class is exported from the AdApi.dll
class ADAPI_API AdApi
{
public:	
	// Enums 
	// Error code of the operation
	enum AdResult
	{
		AdResult_Success = 0,				// Success
		AdResult_Failure = 1,				// General failure
		AdResult_NotInitialized = 2,		// Failure because not properly initialized
		AdResult_NotSupported = 3,			// Current version of API does not support this functionality
		AdResult_BrowserLaunchFailure = 4	// Unable to launch default browser with current ad destination
	};

	// Ad provider protocol to use to get ads
	enum AdProvider
	{
		AdProvider_Invalid	= 0,			// Invalid Ad provider
		AdProvider_24x7		= 1				// 24x7 Ad provider
	};

	typedef void (__stdcall *PaintCallback)(void *thePaintDataP);

public:
	AdApi();
	~AdApi();
	
	// The public interface to the ad API

	// Initilizes the Ad API
	AdResult Init(	AdProvider theAdProvider,				// Determines which Protocol to use
					const char* theProductName,				// Name of product to display ads for
					NetCommInterface* theNetCommInterfaceP, // Interface for network communications - If this is null only default ads are displayed
					const char* theCachePath,				// Sets path in which to put add cache - Set to null to use default
					int theMaxDownloadRate);				// Sets the max number of bytes per second that downloads can occure in.	

	// Shuts down the Ad API
	AdResult Shutdown();

	// Set callbacks

	// Sets the function to be called when an animation frame of an ad needs to be painted.
	AdResult SetPaintCallback(	PaintCallback thePaintCallbackP,	// Function pointer of the Paint Callback
								void* thePaintDataP);				// Data to be passed into Paint Callback

	// Download functions
	//AdResult StartDownload(/*int theMaxDownloadRate*/);
	//AdResult StopDownload();

	//AdResult PauseDownload(bool thePause); 
	
	// Display functions

	// Call this function when the user clicks on the ad.  This will log some information and start the browser.
	AdResult AdClicked();

	// Used to get the current animation frame in bitmap form
	AdResult GetDisplayImage(HDC hDC, HBITMAP& theBitmapHR);

	// Used to get the current animation frame in raw form
	AdResult GetDisplayImage(BYTE* &theRawPR, int& theWidthR, int& theHeightR);

	// Used to release the animation resource - This should be done after it is no longer needed
	void ReleaseDisplayImage(HBITMAP theBitmapH);
	void ReleaseDisplayImage(BYTE* theRawP);

	// Allow or disallow skipping of Ad Scenes when timing issues interfere with the proper presentation
	void AllowSceneSkipping(bool isSceneSkippingAllowed = true);
	bool IsSceneSkippingAllowed(void);

	// Logging functions
	// Used to set the log file for debugging
	bool SetLogFile(const char* theLogFilePath);

private:
	AdDisplay*			mAdDisplayP;			// Keeps track of displaying add
	AdEngine*			mAdEngineP;				// Handles the coordination if downloading and displaying the ads

	bool				mInitialized;			// Determines if API is initilized yet
};

} // WONAPI

#endif // _ADAPI_H

//extern ADAPI_API int nAdApi;

//extern "C" ADAPI_API int fnAdApi(void);

