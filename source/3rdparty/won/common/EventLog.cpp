// EventLog

// Class that encapsulates writing messages to the EventLog.  Uses C++
// streams fill buffer to be logged.  A global instance of this class should be
// created with the initialization constructor and maintained for the life of
// the application.  Other instances may be created very cheaply,
// used and then discarded.

// Note that if the connection to the Event log cannot be created, log
// attempts are written to the LASTCHANCE_FILE.  If writes to this file
// fail, the log message is discarded.  Use the IsOK method to see if a
// connection to the actual event log exists or not (if you care).

// Note that event logs are an NT thing and under 95, all logs will go to
// the LASTCHANCE_FILE: %TEMP%\WONEventLog.log


#include "won.h"
#include <time.h>
#include "EventLog.h"

#ifdef WIN32
#include <process.h>
#endif


// private namespace for using and constants
namespace {
	using WONCommon::EventLog;

	const char* LASTCHANCE_FILE = "WONEventLog.txt";
	const char* STAMP_PREFIX    = "\tLogged: ";
	const char* MSG_PREFIX      = "\tText:   ";

	const char* sADVAPI        = "advapi32";
	const char* sDEREGISTEREVENTSOURCE = "DeregisterEventSource";

#if defined(UNICODE)
	const char* sREGISTEREVENTSOURCE = "RegisterEventSourceW";
	const char* sREPORTEVENT         = "ReportEventW";
#else
	const char* sREGISTEREVENTSOURCE = "RegisterEventSourceA";
	const char* sREPORTEVENT         = "ReportEventA";
#endif
};

// Static Members
std::string   EventLog::mAppName;        // App name
std::string   EventLog::mAppId;          // App LogicalName
std::ofstream EventLog::mEventFile;

#ifdef WIN32
HANDLE        EventLog::mHandle = NULL;  // Event log handle
long          EventLog::mInstCt = 0;     // Instance count
#endif

// CriticalSection
WONCommon::CriticalSection EventLog::mLogCrit;


// ** Constructors / Destructor

// Default constructor
// Adds theP to log stream if defined
EventLog::EventLog(const char* theP, EventType theDefault) :
	mDefType(theDefault),
	mStream()
{
#ifdef WIN32
	InterlockedIncrement(&mInstCt);
#endif
	if (theP)
		GetStream() << theP;
}


// Default constructor
// Adds theP to log stream if defined
EventLog::EventLog(EventType theDefault) :
	mDefType(theDefault),
	mStream()
{
#ifdef WIN32
	InterlockedIncrement(&mInstCt);
#endif
}


// Initialization constructor
// Instance created by this constructor maintains EventLog connection.
// Use this constructor once to make a global instance.  After that just
// use the standard constructor.  Make sure instance created with this
// constructor is not deleted until app exits.
EventLog::EventLog(const std::string& theAppName, const std::string& theLogicalName) :
	mDefType(EventError),
	mStream()
{
#ifdef WIN32
	InterlockedIncrement(&mInstCt);
#endif

	mAppName = theAppName;

	// Build logicalName/PID buffer
	if (! theLogicalName.empty())
		mStream << '(' << theLogicalName << ')';
#if defined(macintosh) && (macintosh == 1)
#else
	mStream << " [PID=" << getpid() << ']';
#endif
	mAppId = mStream.str();
	Clear();

#ifdef WIN32
#ifndef WON_DEBUGENABLE
		if (mHandle) EventLog::DeregisterEventSource(mHandle);
		mHandle = EventLog::RegisterEventSource(NULL, mAppName.c_str());
#endif
#endif
}


// Destructor
EventLog::~EventLog(void)
{
#ifdef WIN32
	long aCt = InterlockedDecrement(&mInstCt);
	if ((aCt == 0) && (mHandle))
		EventLog::DeregisterEventSource(mHandle);
#endif
}


// Returns true if event log connection is available, false if not.
bool
EventLog::IsOK(void) const
{
#ifdef WIN32
	if (mHandle != NULL)
		return true;
#endif

	if (OpenLastChanceFile())
	{
		mEventFile.close();
		return true;
	}
	else
		return false;
}


// EventLog::Log
// Logs stream to the event log.  Adds theP to stream if defined.
// In the spirit of "Don't check for an error that you aren't prepared to handle",
// this function doesn't check the return value of ReportEvent
void
EventLog::Log(const char* theP, EventType theType)
{
	// Add theP to stream if needed, and log the event
	if (theP) GetStream() << theP;
	Log(theType);
}


// EventLog::Log
// Logs stream to the event log. 
// In the spirit of "Don't check for an error that you aren't prepared to handle",
// this function doesn't check the return value of ReportEvent
void
EventLog::Log(EventType theType)
{
	// Log the event and clear the buffer
	LogEvent((theType == EventDefault ? mDefType : theType), mStream.str().c_str());
	Clear();
}


// EventLog::LogEvent
// Static method used to log a string to the event log
void
EventLog::LogEvent(EventType theType, const char* theP)
{
BOOL logged = 0;

	// Log if there's data to log
	if ((! theP) || (! *theP)) return;  // PUNT on no data
	WDBG_AH("EventLog::LogEvent Msg=(" << theP << ')');

	// Enter Crit
	WONCommon::AutoCrit aCrit(mLogCrit);

#ifdef WIN32
	if (mHandle)
	{
		const char* aStrArr[2] = { mAppId.c_str(), theP };
		WORD anEventType       = (theType == EventWarn ? EVENTLOG_WARNING_TYPE     :
		                         (theType == EventInfo ? EVENTLOG_INFORMATION_TYPE :
		                          EVENTLOG_ERROR_TYPE));

		logged = EventLog::ReportEvent(mHandle,      // event log handle
									   anEventType,  // event type
									   0,            // category zero
									   0x1002,       // event identifier 
									   NULL,         // no user security identifier
									   2,            // two substitution string
									   0,            // no data
									   aStrArr,      // buffer to log
									   NULL);        // address of data
	}
#endif

	// If event log not available, try last chance file!
	if (logged == 0)
	{
		WDBG_LH("EventLog::LogEvent Can't access event log, trying last chance file!");
		if (OpenLastChanceFile())
		{
			time_t now = time(NULL);
			mEventFile << theType << " - " << mAppName << mAppId << endl
			           << STAMP_PREFIX << asctime(localtime(&now))
			           << MSG_PREFIX << theP << endl << endl;
			mEventFile.close();
		}
	}
}

bool EventLog::OpenLastChanceFile()
{
	static char* aLastChanceFilePath = NULL;
	if (aLastChanceFilePath == NULL)
	{
		aLastChanceFilePath = new char[MAX_PATH];
		aLastChanceFilePath[0] = '\0';
	}

	// attempt to get the path to the last-chance file if we don't have it already
	if (aLastChanceFilePath[0] == '\0')
	{
#ifdef WIN32
		if (GetTempPath(MAX_PATH, aLastChanceFilePath) == 0)
			return false;
#endif
		strcat(aLastChanceFilePath, LASTCHANCE_FILE);
	}

	// attempt to open that file
	mEventFile.open(aLastChanceFilePath, std::ios::out | std::ios::app);
	return mEventFile.is_open();
}

// ** Output Operators **

ostream&
operator<<(ostream& os, EventLog::EventType theType)
{
	switch (theType)
	{
	case EventLog::EventDefault:
	case EventLog::EventError:
		os << "ERROR";  break;
	case EventLog::EventWarn:
		os << "WARNING";  break;
	case EventLog::EventInfo:
		os << "INFORMATIONAL";  break;
	default:
		os << "UNKNOWN";  break;
	}

	return os;
}

#ifdef WIN32

// Helper class
class AdvApi32
{
	HMODULE mAdvApiHandle;
public:
	AdvApi32()
	{
		mAdvApiHandle = LoadLibrary(sADVAPI);
	}
	~AdvApi32()
	{
		if (mAdvApiHandle != NULL)
			FreeLibrary(mAdvApiHandle);
	}
	FARPROC GetProcAddr(LPCSTR sFn) const
	{
		if (mAdvApiHandle == NULL)
			return (NULL);
		return (GetProcAddress(mAdvApiHandle, sFn));
	}
};

// These functions are not supported on all versions of Windows, so we do
// a little checking and call them via function pointers if they're available
BOOL EventLog::DeregisterEventSource(HANDLE h)
{
	typedef BOOL (WINAPI * pfnDeregisterEventSource)(HANDLE);

	AdvApi32 AdvApi;
	pfnDeregisterEventSource fnDeregisterEventSource = 
								reinterpret_cast<pfnDeregisterEventSource>
									(AdvApi.GetProcAddr(sDEREGISTEREVENTSOURCE));
	return (fnDeregisterEventSource ? fnDeregisterEventSource(h) : FALSE);
}

HANDLE EventLog::RegisterEventSource(LPCTSTR lpUNCServerName, LPCTSTR lpSourceName)
{
	typedef HANDLE (WINAPI * pfnRegisterEventSource)(LPCTSTR, LPCTSTR);

	AdvApi32 AdvApi;
	pfnRegisterEventSource fnRegisterEventSource = 
									reinterpret_cast<pfnRegisterEventSource>
										(AdvApi.GetProcAddr(sREGISTEREVENTSOURCE));
	return (fnRegisterEventSource ? fnRegisterEventSource(lpUNCServerName, 
															lpSourceName) : NULL);
}

BOOL EventLog::ReportEvent(HANDLE hEventLog, WORD wType, WORD wCategory, 
							DWORD dwEventID, PSID lpUserSid, WORD wNumStrings,
							DWORD dwDataSize, LPCTSTR *lpStrings, LPVOID lpRawData)
{
	typedef BOOL (WINAPI * pfnReportEvent)(HANDLE, WORD, WORD, DWORD, PSID, WORD,
											DWORD, LPCTSTR*, LPVOID);

	AdvApi32 AdvApi;
	pfnReportEvent fnReportEvent = reinterpret_cast<pfnReportEvent>
									(AdvApi.GetProcAddr(sREPORTEVENT));
	return (fnReportEvent ? fnReportEvent(hEventLog, wType, wCategory, dwEventID, 
											lpUserSid, wNumStrings, dwDataSize, 
											lpStrings, lpRawData) : FALSE);
}
#endif
