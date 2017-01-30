#ifndef _ADPROTOCOL_H
#define _ADPROTOCOL_H

#include <string>
#include "AdApi.h"
#include "AdCache.h"
#include "Utils.h"

namespace WONAPI
{

// Base class that all ad serving protocols are derrived from
// AdProtocol classes contain the protocol for talking to ad server
class AdProtocol
{
	
public:
	// Construct/Destruct
	AdProtocol(NetCommInterface* theNetCommInterfaceP, unsigned long theMaxDownloadRate, AdEngine* theAdEngineP, const std::string& theCachePath);
	virtual ~AdProtocol();

	// Public methods
	virtual bool DownloadAd();

	// Private methods
private:
	bool CreateAdFile(const RawBuffer& theContent);
	const std::string GenerateAdPath(const std::string& theFileName);
	void AddReplaceFileExt(std::string& theFilePath);

	// Inlines
public:
	// Get the URL of the destination to go to when ad is clicked on
	const std::string& GetAdDestination()
	{
		// Possible race condition
		return mCurrAdDest;
	}
	// Get path to file where current ad is stored..
	const std::string& GetCurrentAdPath()
	{
		// another possible race condition
		return mCurrAdPath;
	}
	
	// Protected member data
protected:
	NetCommInterface*	mNetCommInterfaceP;
	unsigned long		mMaxDownloadRate;
	AdEngine*			mAdEngineP;

	AdCache*			mAdCacheP;			// Cache where all of the downloaded ads are stored

	std::string			mCurrAdDest;		// The URL to go to when ad is clicked on...
	std::string			mCurrAdPath;		// Path to file where current ad is stored..
};

} // WONAPI

#endif // _ADPROTOCOL_H