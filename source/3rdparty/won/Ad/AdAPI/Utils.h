#ifndef _AD_UTILS_H
#define _AD_UTILS_H

#include <fstream>
#include <sstream>
#include <string>

#include "RegKey.h"

// Contains utility functions and classes for AdApi library

namespace WONAPI
{

// Constants

// Options
extern const char* OptionName_FileUsageLifespan;
extern const char* OptionName_FileExistenceLifespan;
extern const char* OptionName_MaxFilesInCache;

// Default values
extern const unsigned long OptionDefault_FileUsageLifespan;
extern const unsigned long OptionDefault_FileExistenceLifespan;
extern const unsigned long OptionDefault_MaxFilesInCache;

// Typedefs
// Raw binary buffer type
typedef std::basic_string<unsigned char> RawBuffer;

// Statics for logging to file and debug stream
#ifdef LOGGING

#define INIT_LOG(logfilepath)		AdLog::Init(logfilepath)
#define SHUTDOWN_LOG()				AdLog::Shutdown()
#define SET_LOGFILE(logfilepath)	AdLog::SetLogfile(logfilepath)
#define LOG(message)				{ AdLog::GetLogStream() << message; AdLog::Flush(); }

#else // No logging

#define INIT_LOG(logfilepath)
#define SHUTDOWN_LOG()			
#define SET_LOGFILE(logfilepath)
#define LOG(message)
	
#endif // LOGGING

#ifdef LOGGING

class AdLog
{

private:
	AdLog(); // Don't alow constructor

	// Methods
public:
	static bool	Init( const char* theLogfilePath );
	static void	Shutdown();
	static void SetLogging(bool theLogging);
	static bool SetLogfile( const char* theLogfilePath );
	static std::stringstream& GetLogStream();
	static void Flush();

	// Members
private:
	static std::stringstream	mLogStream;
	static std::ofstream		mLogFile; 

	static bool					mLogging;
};

#endif // NO_LOGGING

// Options class that loads options from the registry
class AdOptions
{
private:
	AdOptions (); // Don't alow constructor

	// Methods
public:
	static bool Init();
	static void Shutdown();
	static bool GetStringOption(const std::string& theOptionNameR, const std::string& theDefaultValR, std::string& theOptionValR);
	static bool GetLongOption(const std::string& theOptionNameR, unsigned long theDefaultValR, unsigned long& theOptionValR);
	static bool GetBinaryOption(const std::string& theOptionNameR, unsigned char* &theOptionValR, unsigned long& theLengthR);

	// Members
private:
	static WONCommon::RegKey mRegKey;
};

// File manipulation routines
bool PathExists(const std::string& thePath);
bool FindPath(std::string& theFilePath);
bool CreateFullPath(const std::string& thePath);
bool GetModulePath(std::string& theModulePath);

}

#endif // _AD_UTILS_H