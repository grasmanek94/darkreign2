#ifndef _ADENGINE_H
#define _ADENGINE_H

// AdEngine

#include "NetCommInterface.h"
#include "AdProtocol.h"
#include "AdDisplay.h"

namespace WONAPI
{

// This class uses the other ad serving classes to drive the entire ad serving process.
class AdEngine: public WONCommon::ThreadBase
{
	// Constants
	static const std::string Default_AdImageFileName;// = "DefaultAd.gif";
	static const std::string Default_AdLinkFileName;// = "DefaultLink.txt";
public:
	// Constructor/Destructor
	AdEngine(NetCommInterface* theNetCommInterfaceP,  AdDisplay* theAdDisplayP);
	virtual ~AdEngine();

	// Methods
	bool Init(AdApi::AdProvider theAdProvider, const std::string& theProductName, unsigned long theMaxDownloadRate, const std::string& theCachePath/*, unsigned long theFileUsageLifespan, unsigned long theFileExistenceLifespan, unsigned short theMaxFilesInCache*/);

	void StartAdDownload();
	bool LaunchAdDestination();
	bool DisplayAd(const std::string& theAdPath);
	void DisplayDefaultAd();
	bool GetDefaultDestination(std::string& theDefaultDestination);
	void OptionsDone(bool theResult);

private:
	// Private methods
	int threadProcess();
	DWORD CalculateTimeUntilAdDisplay();
	bool GetAdServOptions();

	// Public Inlines
public:
	HANDLE getStartDownloadOptionsEvent()
	{
		return mStartDownloadOptionsEvent;
	};
	HANDLE getStartDownloadEvent()
	{
		return mStartDownloadEvent;
	};

	// members
private:
	NetCommInterface*	mNetCommInterfaceP;			// Communication interface for network communication // ** May not need this here
	AdDisplay*			mAdDisplayP;				// Keeps track of displaying add
	
	AdProtocol*			mAdProtocolP;				// Communication protocol to talk to ad server

	HANDLE				mStartDownloadOptionsEvent;	// Event to signal to start downloading options
	HANDLE				mStartDownloadEvent;		// Event to signal to start the ad download process
	DWORD				mLastDisplayTime;

	HANDLE				mOptionsDoneEvent;			// Event signaled when options download is done
	bool				mOptionsResult;				// Result of options download attempt

	DWORD				mAdDisplayDuration;			// Option that shows how long to display an ad
	bool				mAdReady;					// Is ad downloaded and ready to be displayed
	bool				mOptionsDownloaded;			// Have options been downloaded yet?

	std::string			mProductName;				// Name of the product to display ads for
	bool				mShowingDefaultAd;			// True if showing a default ad


};

// Callbacks
void __stdcall GetOptionsCallback(bool theResult, void* theDataP);
}

#endif