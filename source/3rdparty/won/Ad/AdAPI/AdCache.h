#ifndef _ADCACHE_H
#define _ADCACHE_H

#include "ThreadBase.h"
#include "RegKey.h"
#include "CriticalSection.h"

#pragma warning (disable : 4786)
#include <map>

// This class handles the saving of ad files to disk and maintaining the cache size
class AdCache : public WONCommon::ThreadBase
{
	// Constructor/Destructor
public:
	AdCache(const std::string& theBaseCacheDirectory, unsigned long theFileUsageLifespan = 24*3600, 
			unsigned long theFileExistenceLifespan = 7*24*3600, unsigned short theMaxFilesInCache = 100);
	~AdCache(void);

	// Requesting a file from the cache, which will return true if the file is in the cache and fill in theFileAndPath 
	bool RequestFile(const std::string& theFileName, std::string& theFileAndPath);

	// Adding a file to the cache, the file will be moved during this call so do not reference the file from its 
	// orignal loacation (use RequestFile to get the file's new location)
	bool AddFile(const std::string& theFileAndPath);

	const std::string& GetCachePath();

	// Thread over-rides
	void startThread();
	void stopThread();

	unsigned long     mFileUsageLifespan;      // Maximum time file can be in cache without usage
	unsigned long     mFileExistenceLifespan;  // Maximum time file can be in cache from time of download
	unsigned short    mMaxFilesInCache;        // Maximum files allowed to be stored in cache at any time

private:
	std::string       mCurrentlyRequestedFile; // The file that has been most currently requested from the cache
	std::string       mBaseCacheDirectory;     // The base directory to store cached files
	bool              mCleanupInProgress;      // Cleanup is in progress
	WONCommon::RegKey mRegistryKey;            // Base registry key for persistant data

	WONCommon::CriticalSection mCrit;

	struct FILE_DATA
	{
		std::string mFullNameAndPath;   // Full name and path of the cached file
		time_t      mTimeDownloaded;    // Time file was added to the cache
		time_t      mTimeLastRequested; // Time file was last requested from the cache
	};

	// Storing file data based on name
	typedef std::map<std::string, FILE_DATA> FILE_MAP;
	FILE_MAP mFileNameMap;

	// Time sorted key names
	typedef std::multimap<long, std::string> TIME_MAP;
	TIME_MAP mTimeFileRequestedMap;

	HANDLE mCleanupTriggerH;
	HANDLE getCleanupEvent();

	int threadProcess();

	std::string GetSubdirectory(const std::string& theFileName);
	void UpdateRegistryData(const std::string& theFileName, const FILE_DATA& theFileData);
	void RemoveFromRegistry(const std::string& theFileName);
	void GetFileInfoFromRegistry(const WONCommon::RegKey& aSubKey);
	void LoadAllFromRegistry(void);
	void Cleanup(void);
};

inline const std::string& 
AdCache::GetCachePath()
{
	return mBaseCacheDirectory;
}

inline HANDLE 
AdCache::getCleanupEvent()
{
	return mCleanupTriggerH;
}

#endif // _ADCACHE_H