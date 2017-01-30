
// Contains utility functions and classes for AdApi library

#include "Windows.h"
#include "Utils.h"

using namespace WONAPI;
using namespace std;
using namespace WONCommon;

// Constants

// Options
const char* WONAPI::OptionName_FileUsageLifespan		= "FileUsageLifespan";
const char* WONAPI::OptionName_FileExistenceLifespan	= "FileExistenceLifespan";
const char* WONAPI::OptionName_MaxFilesInCache			= "MaxFilesInCache";

// Default values
const unsigned long WONAPI::OptionDefault_FileUsageLifespan		= 24*3600;		// 1 day
const unsigned long WONAPI::OptionDefault_FileExistenceLifespan	= 7*24*3600;	// 7 days
const unsigned long WONAPI::OptionDefault_MaxFilesInCache		= 100;

////////////////////////////////////////////////////////////////////////////////////
// For logging
#ifdef LOGGING

// Static member declarations
std::stringstream AdLog::mLogStream;
std::ofstream AdLog::mLogFile;
bool AdLog::mLogging = false; 

bool 
AdLog::Init( const char* theLogfilePath )
{
	mLogStream.clear();

	return SetLogfile(theLogfilePath);
}

void 
AdLog::Shutdown()
{
	mLogFile.close();
	SetLogging(false);
}

void 
AdLog::SetLogging(bool theLogging)
{
	mLogging = theLogging; 
}

bool 
AdLog::SetLogfile( const char* theLogfilePath )
{
	bool aReturn; 

	if (!CreateFullPath(theLogfilePath))
	{
		return false; // Unable to create path
	}

	if (mLogFile.is_open())
	{
		mLogFile.close();
	}
	mLogFile.open(theLogfilePath, ios::out | ios::app);

	aReturn = mLogFile.is_open();
	SetLogging(aReturn);

	return aReturn;
}

stringstream& 
AdLog::GetLogStream()
{
	mLogStream << "AdApi: ";
	return mLogStream;
}

void 
AdLog::Flush()
{
	mLogStream << endl;

	if (mLogging && mLogFile.is_open())
	{
		mLogFile << mLogStream.str() << flush;
	}

#ifdef _DEBUG
	OutputDebugString(mLogStream.str().c_str());
#endif

	mLogStream.clear();
}

#endif // LOGGING

////////////////////////////////////////////////////////////////////////////////////
// Options

// Static member decl
RegKey AdOptions::mRegKey;

bool
AdOptions::Init()
{
	return mRegKey.OpenNewKey("SOFTWARE\\WON\\AdAPI", HKEY_LOCAL_MACHINE, true) == RegKey::Ok;
}

void
AdOptions::Shutdown()
{
	mRegKey.CloseKey();
}

bool
AdOptions::GetStringOption(const string& theOptionNameR, const string& theDefaultValR, string& theOptionValR)
{
	if (mRegKey.GetValue(theOptionNameR, theOptionValR) != RegKey::Ok) 
	{
		theOptionValR = theDefaultValR;
	}
	return true;
}

bool
AdOptions::GetLongOption(const string& theOptionNameR, unsigned long theDefaultValR, unsigned long& theOptionValR)
{
	if (mRegKey.GetValue(theOptionNameR, theOptionValR) != RegKey::Ok)
	{
		theOptionValR = theDefaultValR;
	}
	return true;
}

// Remember to delete binary data when done
bool
AdOptions::GetBinaryOption(const string& theOptionNameR, unsigned char* &theOptionValR, unsigned long& theLengthR)
{
	return mRegKey.GetValue(theOptionNameR, theOptionValR, theLengthR) == RegKey::Ok;
}

////////////////////////////////////////////////////////////////////////////////////
// File manipulation routines

// Check for existance of path
bool 
WONAPI::PathExists(const string& thePath)
{
	WIN32_FIND_DATA aFindData;
	
	HANDLE aPathHandle = ::FindFirstFile(thePath.c_str(), &aFindData);
	if (aPathHandle != INVALID_HANDLE_VALUE)
	{
		::FindClose(aPathHandle);
		return true;
	}
	return false;
}

// GetPath
// 
// Gets a path from a filename

bool
WONAPI::FindPath(string& theFilePath)
{
	// Extract the path
	// Determine path by stripping off everything after the
	// final backslash or slash
	string::size_type aSlash = theFilePath.find_last_of('\\');
	if (aSlash == string::npos)
		aSlash = theFilePath.find_last_of('/');

	// No backslash or slash; this is not a path!
	if (aSlash == string::npos)
	{
		return false;
	}

	theFilePath.resize(aSlash+1);
	return true;
}

// Create Full path
//
// Creates directories along the full path specified

bool 
WONAPI::CreateFullPath(const string& thePath)
{
	string aOutputDir = thePath;

	// Not a path error
	if (FindPath(aOutputDir) || aOutputDir.length() == 0)
	{
		return false;
	}

	// If directory exists, we're done
	if (WONAPI::PathExists(aOutputDir))
	{
		return true;
	}

	// Generate directories all the way down the path
	for (string::size_type aMarker = 0; aMarker != string::npos; )
	{
		++aMarker;
		string::size_type aBackSlash = aOutputDir.find('\\', aMarker);
		string::size_type aSlash = aOutputDir.find('/', aMarker);
		aMarker = min(aBackSlash, aSlash);
		string aDir = aOutputDir.substr(0, aMarker);
		if (!WONAPI::PathExists(aDir))
		{
			BOOL aSuccess = ::CreateDirectory(aDir.c_str(), NULL);

			// It's OK to have errors "creating" drives
			if (!aSuccess && aDir[1] != ':')
			{
				LOG("Unable to create " << aOutputDir.c_str() << " err " << ::GetLastError());
			}
		}
	}
	return true;
}

bool
WONAPI::GetModulePath(string& theModulePath)
{
	char aPath[256];
	if (GetModuleFileName(NULL,aPath,256))
	{
		theModulePath = aPath;
		if (WONAPI::FindPath(theModulePath))
		{
			return true;
		}
	}
	return false;
}



	
