// WONDebug.h
//
// Debugging definitions, all acess via macros below.  DO NOT use
// debug class methods directly as they will not compile out like
// the macros will.  Each debug message will be on its own line
// (endl is appended automatically) and will be formatted as follows:
//
//   threadName(threadId): msg
//
// where threadName is the logical threadName (or appName if the main
// thread), threadId is the threadId, and msg is the message output.
// Messages will be output to a debug file and debug window.
//
// Setting a debug level will cause all debug methods that correspond
// to that level or a higher level to execute.  For example, setting
// debug level to WDBG_APPMED will cause all _AH and _AM macros to
// be executed.
//
// The constant WON_DEBUGENABLE must be defined to enable debugging
// and tracing in your application/library.
//
// The constant WON_TRACEENABLE must be defined to enable tracing in
// your application/library.


#include "won.h"
#include <stdio.h>
#include <fstream>
#include <map>
#include <algorithm>
#include <time.h>
#include "CriticalSection.h"
#include "WONDebug.h"
#include "WONString.h"

#ifdef WIN32
typedef DWORD ThreadID;
#elif defined(_LINUX)
typedef pthread_t ThreadID;
#define GetCurrentThreadId pthread_self
#elif defined(macintosh) && (macintosh == 1)
#define GetCurrentThreadId() (0)
#endif

// Private namespace for using, local types, private vars
namespace {
	using namespace WONDebug;
	using WONCommon::AutoCrit;
	using WONCommon::CriticalSection;
	using WONCommon::wonstringstream;

	// Types
	// Track information per thread (stored in a STL map)
	struct ThreadData
	{
		string               mName;        // Thread name
		WONDebug::Debugger::DebugLevel mDebugLevel;  // Thread debug level
		int                  mTraceLevel;  // Thread trace level
		wonstringstream      mBuf;         // Thread stream buf

		// Constructors, destructor, and assignment.
		explicit ThreadData(const char* theName=NULL);
		ThreadData(const ThreadData& theData);
		~ThreadData(void);
		ThreadData& operator=(const ThreadData& theData);
	};

	// STL Map for thread ID to ThreadData
	typedef std::map<ThreadID, ThreadData> ThreadMap;

	// Constants
	const int   SPACES_PER_LEVEL = 2;
	const int   DIVIDER_WIDTH    = 70;
	const char* MAIN_DEF_NAME    = "MAIN";
	const char* START_PREFIX     = "Started: ";
	const char* DEBUG_FILE_EXT   = ".txt";

	// Private Vars
	ThreadMap*                  gThreadMap = NULL;  // Map of thread IDs to ThreadData
	WONCommon::CriticalSection* gDebugCritP = NULL; // Debug critical section
	bool                        gCritAllocated = false; // -1 before allocation, 0 afterwards
}; // Private namespace

// Static Class Members
std::ofstream        WONDebug::Debugger::mDebugFile;
WONDebug::Debugger::DebugLevel WONDebug::Debugger::mGLevel = WONDebug::Debugger::Off;
int                  Tracer::mGEnable  = -1;


// ** Debugger::ThreadData methods **

// Default constructor
ThreadData::ThreadData(const char* theName) :
	mName(),
	mDebugLevel(WONDebug::Debugger::GetGlobalLevel()),
	mTraceLevel(Tracer::GetGlobalEnable() ? 0 : -1),
	mBuf()
{
	if (theName) mName = theName;
}


// Copy constructor
ThreadData::ThreadData(const ThreadData& theData) :
	mName(theData.mName),
	mDebugLevel(theData.mDebugLevel),
	mTraceLevel(theData.mTraceLevel),
	mBuf()
{}


// Destructor
ThreadData::~ThreadData(void)
{}


// Assignment operator
ThreadData&
ThreadData::operator=(const ThreadData& theData)
{
	mName.erase();  // Prevent VC++ 5.0 string assignment bug!
	mName       = theData.mName;
	mDebugLevel = theData.mDebugLevel;
	mTraceLevel = theData.mTraceLevel;
	mBuf.clear();

	return *this;
}

static CriticalSection&
GetCriticalSection()
{
	static CriticalSection crit;
	AutoCrit autoCrit(crit);

	if (!gCritAllocated)
	{
		gDebugCritP = new CriticalSection;
		gCritAllocated = true;;
	}
	
	return *gDebugCritP;
}

// ThreadData accessor method (thread safe)
static ThreadData&
GetThreadData(ThreadID theId)
{
	AutoCrit aCrit(GetCriticalSection());
	if (! gThreadMap)
		gThreadMap = new ThreadMap;

	return (*gThreadMap)[theId];
}


// Class Debugger Methods

// ** Private Methods **

// Debugger::BuildFileName
// Builds the path and file name to use as the debug file.  Path/file
// is built from the appName, logicalName, and an optional dir path.
string
WONDebug::Debugger::BuildFileName(const char* theAppName, const char* theLogicalName,
						unsigned short thePort, const char* theDirPath)
{
	string aFilePath;

	// Add dirPath and trailing \\ if dirPath given
	if (theDirPath)
	{
		aFilePath = theDirPath;
		aFilePath += "\\";
	}

	// Add the appName
	if (theAppName)
		aFilePath += theAppName;

	// If logicalName given, add logicalname prefixed by _
	if ((theLogicalName) && (*theLogicalName))
	{
		aFilePath += '_';
		aFilePath += theLogicalName;
	}

	// Add the primary port
	if (thePort != 0)
	{
		char aPortStr[6];
		sprintf(aPortStr, "%hu", thePort);
		aFilePath += '_';
		aFilePath += aPortStr;
	}

	// Add file extention and return the path
	aFilePath += DEBUG_FILE_EXT;
	return aFilePath;
}


// ** Public Methods **

// Debugger::Init
// Intialize debugging.  Simply calls appropriate other methods
void
WONDebug::Debugger::Init(DebugLevel theLevel, const char* theAppName,
			   const char* theLogicalName, unsigned short thePort, const char* theDirPath)
{
	SetGlobalLevel(theLevel, true);
	SetFile(theAppName, theLogicalName, thePort, theDirPath);
	ThreadStart(theAppName ? theAppName : MAIN_DEF_NAME);
}


// Debugger::SetFile
// Opens a debug file and outputs header info to the file.
bool
WONDebug::Debugger::SetFile(const char* theAppName, const char* theLogicalName,
				  unsigned short thePort, const char* theDirPath)
{
	// PUNT if no app name
	if ((! theAppName) || (! *theAppName)) return false;

	// Close current file if open
	if (mDebugFile.is_open()) mDebugFile.close();

	// Open new file
	string aFilePath(BuildFileName(theAppName, theLogicalName, thePort, theDirPath));
	mDebugFile.open(aFilePath.c_str(), std::ios::out | std::ios::app);

	// Output divider if file was opened (since we're appending)
	if (mDebugFile.is_open())
	{
		AutoCrit aCrit(GetCriticalSection());  // Enter crit sec
		time_t now = time(NULL);
		mDebugFile << endl << endl << string(DIVIDER_WIDTH, '*')
		           << endl << START_PREFIX << asctime(localtime(&now))
		           << endl << endl;
	}

	// Return open status of file
	return mDebugFile.is_open();
}


// Debugger::ThreadStart
// Initialize for a new thread, add entry to thread map.
void
WONDebug::Debugger::ThreadStart(const char* theName)
{
ThreadData aTData(theName);

	GetThreadData(GetCurrentThreadId()) = aTData;
}


// Debugger::ThreadStop
// Cleanup when thread terminates, remove entry from thread map.
void
WONDebug::Debugger::ThreadStop(void)
{
	AutoCrit aCrit(GetCriticalSection());
	if (gThreadMap)
		gThreadMap->erase(GetCurrentThreadId());
}


// Debugger::GetLevel
// Return debug level for the current thread.
WONDebug::Debugger::DebugLevel
WONDebug::Debugger::GetLevel(void)
{
	return GetThreadData(GetCurrentThreadId()).mDebugLevel;
}


// Debugger::SetLevel
// Set the debug level for the current thread
void
WONDebug::Debugger::SetLevel(WONDebug::Debugger::DebugLevel theLevel)
{
	GetThreadData(GetCurrentThreadId()).mDebugLevel = theLevel;
}


// Debugger::SetGlobalLevel
// Sets the global debug level and, if requested, updates debug level
// of all existing threads to the new global level.
void
WONDebug::Debugger::SetGlobalLevel(DebugLevel theLevel, bool allThreads)
{
	mGLevel = theLevel;
	if (allThreads)
	{
		AutoCrit aCrit(GetCriticalSection());  // Enter crit sec
		if (gThreadMap)
		{
			ThreadMap::iterator anItr = gThreadMap->begin();
			while (anItr != gThreadMap->end())
			{
				anItr->second.mDebugLevel = mGLevel;
				anItr++;
			}
		}
	}
}


// Debugger::CheckLevel
// Checks specified level against debug level for current thread.
// Returns true if specified level is <= thread debug level.
bool
WONDebug::Debugger::CheckLevel(WONDebug::Debugger::DebugLevel theLevel)
{
	return (theLevel <= GetThreadData(GetCurrentThreadId()).mDebugLevel);
}


// Debugger::GetStream
// Outputs debug line header info and returns the debug stream for
// the current thread.
std::ostream&
WONDebug::Debugger::GetStream(void)
{
ThreadID    anId    = GetCurrentThreadId();
ThreadData& aTDataR = GetThreadData(anId);

	// Output thread name if one is defined
	if (! aTDataR.mName.empty()) aTDataR.mBuf << aTDataR.mName;

	// Output thread id and return the stream
	aTDataR.mBuf << '(' << anId << "): ";
	return aTDataR.mBuf;
}


// Debugger::Flush
// Adds trailer to stream for current thread and outputs stream to
// debug file (if one is open) and debug window.  Clears the stream.
void
WONDebug::Debugger::Flush(void)
{
ThreadData& aTDataR = GetThreadData(GetCurrentThreadId());

	// Add an endline and endstring
	aTDataR.mBuf << endl;

	// Enter crit sec 
	AutoCrit aCrit(GetCriticalSection());

	// Output stream contents to debug file (if open) and debug win
	if (mDebugFile.is_open()) mDebugFile << aTDataR.mBuf.str() << std::flush;

#ifdef WIN32
	OutputDebugString(aTDataR.mBuf.str().c_str());
#endif

	// Clear stream buffer
	aTDataR.mBuf.clear();
}


// Tracer Methods

// ** Constructors / Destructor **

// Default constructor
Tracer::Tracer(const string& theMethod) :
	mMethod(theMethod)
{
ThreadData& aTDataR = GetThreadData(GetCurrentThreadId());

	// If tracing is on for current thead
	if (aTDataR.mTraceLevel >= 0)
	{
		// Output method enter message
		WONDebug::Debugger::GetStream() << GetIndentStr(aTDataR.mTraceLevel)
		                      << "++" << mMethod;

		// Flush and increment trace level
		WONDebug::Debugger::Flush();
		aTDataR.mTraceLevel++;
	}
}


// Destructor
Tracer::~Tracer(void)
{
ThreadData& aTDataR = GetThreadData(GetCurrentThreadId());

	// If tracing is on for current thead
	if (aTDataR.mTraceLevel >= 0)
	{
		// Decrement trace level, but no below 0.
		if (--aTDataR.mTraceLevel < 0) aTDataR.mTraceLevel = 0;

		// Output mesage exit message and flush
		WONDebug::Debugger::GetStream() << GetIndentStr(aTDataR.mTraceLevel)
		                      << "--" << mMethod;

		WONDebug::Debugger::Flush();
	}
}


// ** Private Methods **

// Tracer::GetIndentStr
// Returns string of spaces for the specified trace level.
string
Tracer::GetIndentStr(int theLevel)
{
	return string(theLevel * SPACES_PER_LEVEL, ' ');
}


// ** Public Methods **

// Tracer::GetEnable
// Returns whether current thread has tracing enabled.
bool
Tracer::GetEnable(void)
{
	return (GetThreadData(GetCurrentThreadId()).mTraceLevel >= 0);
}


// Tracer::SetEnable
// Enables/disables tracing for the current thread,
void 
Tracer::SetEnable(bool theFlag)
{
ThreadData& aTDataR = GetThreadData(GetCurrentThreadId());

	if (theFlag)
	{
		if (aTDataR.mTraceLevel < 0)
			aTDataR.mTraceLevel = 0;
	}
	else
		aTDataR.mTraceLevel = -1;
}


// Tracer::SetGlobalEnable
// Enable or disable tracing at a global level.  If requested,
// tracing for all existing threads will be updated to the new
// global value.
void
Tracer::SetGlobalEnable(bool theFlag, bool allThreads)
{
	mGEnable = (theFlag ? 0 : -1);
	if (allThreads)
	{
		AutoCrit aCrit(GetCriticalSection());  // Enter crit sec
		if (gThreadMap)
		{
			ThreadMap::iterator anItr = gThreadMap->begin();
			while (anItr != gThreadMap->end())
			{
				if (theFlag)
				{
					if (anItr->second.mTraceLevel < 0)
						anItr->second.mTraceLevel = 0;
				}
				else
					anItr->second.mTraceLevel = -1;
				anItr++;
			}
		}
	}
}
