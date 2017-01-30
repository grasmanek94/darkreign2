// WONException

// Defines Exception base class for WON exceptions.  All WON exceptions
// should derive from this class.

// Also defines MemoryException which is thrown by the installed new_handler
// if memory is exhausted.  A new_handler is installed by WONException when
// its static Init method is called (by AppBase).

// Exception Codes and standard text are defined in the WONExceptCodes.h header.

// **NOTE**
// Always catch WONExceptions by REFERENCE.  This will improve performance and
// limit ownership and exception logging issues.
// **NOTE**

// This is the Exception Source Module
#define _WON_EXCEPTION_SOURCE


#include "won.h"
#include <exception>
#include <map>
#include <new.h>
#include <stdlib.h>
#include <stdio.h>
#include "WONException.h"

// Private namespace
namespace {
	using namespace WONCommon;

	const char* UNDEFINED_EXCEPT = "Undefined exception.";

	typedef std::map<unsigned long, const char*> ExceptionMap;

	ExceptionMap    gExceptMap;             // Exception standard text by code
	MemoryException gMemExcept;             // Global memory exception
#ifdef WIN32
	_PNH            gOldNewHandler = NULL;  // Previous new handler
#endif
};

// Include EventLog, exception code, and standard text defs
// This MUST BE DONE AFTER the decls above and in the order below.
#include "WONExceptCodes.h"  //Defines LoadExceptionMap()
#include "EventLog.h"

// Static members
const unsigned long MemoryException::EX_CODE = ExMemory;  // MemoryException code


#ifdef WIN32
// WONNewHandler
// Installed new handler for WON apps.  Calls previous handler if needed,
// Logs memory exception to event log, throws memory exception.
static int __cdecl
WONNewHandler(size_t theSize)
{
	if (gOldNewHandler) (*gOldNewHandler)(theSize);
	gMemExcept.Log();
	throw gMemExcept;

	// We'll never get here
	return 0;
}
#endif


// ExStateToStr
// Converts ExState enum values to string.
static const char*
ExStateToStr(WONException::ExState theState)
{
const char* aP;

	switch (theState)
	{
	case WONException::Critical:
		aP = "Critcal";  break;
	case WONException::StateChanged:
		aP = "StateChanged";  break;
	case WONException::Continuing:
		aP = "Continuing";  break;
	default:
		aP = "Unknown";  break;
	}

	return aP;
}


// ** Constructors / Destructors **

// Default Constructor
WONException::WONException(unsigned long theCode, int theLine, const char* theFileP,
                           const char* addTextP) throw() :
	exception(),
	mState(Critical),
	mCode(theCode),
	mFile(theFileP ? theFileP : ""),
	mLine(theLine),
	mLogIt(true),
	mStream(),
	mWhat()
{
	WTRACE("WONException::ctor");
	if (addTextP) mStream << addTextP;
}


// Copy Constructor
WONException::WONException(const WONException& theExR) throw() :
	exception(theExR),
	mState(theExR.mState),
	mCode(theExR.mCode),
	mFile(theExR.mFile),
	mLine(theExR.mLine),
	mLogIt(theExR.mLogIt),
	mStream(),
	mWhat()
{
	WTRACE("WONException::copy ctor");
	mStream << theExR.mStream.rdbuf();

	// Don't log in previous version, this one will handle it
	const_cast<WONException&>(theExR).mLogIt = false;
}


// Destructor
WONException::~WONException(void) throw()
{
	WTRACE("WONException::dstrct");
	WDBG_LH("WONException::dstrct " << *this);

	// Send exception text to event log if enabled.
	if (mLogIt)
	{
		WDBG_LM("WONException::dstrct Logging exception to event log");
		EventLog aLog(what());
		aLog.Log();
	}
}


// ** Public Methods **

// WONException Assignment operator
WONException&
WONException::operator=(const WONException& theExR) throw()
{
	WTRACE("WONException::operator=");
	BuildFrom(theExR);
	return *this;
}


void
WONException::BuildFrom(const WONException& theExR) throw()
{
	WTRACE("WONException::BuildFrom");
	mState = theExR.mState;
	mCode  = theExR.mCode;
	mFile  = theExR.mFile;
	mLine  = theExR.mLine;
	mLogIt = theExR.mLogIt;
	mWhat.erase();

	// Clear stream and add text from assigned exception
	mStream.clear();
	mStream << theExR.mStream.rdbuf();

	// Don't log in previous version, this one will handle it
	const_cast<WONException&>(theExR).mLogIt = false;
}


// WONException::Init
// Initializes class data of WONException and installs the new handler
void
WONException::Init(void) throw()
{
	WTRACE("WONException::Init");

	// Load standard text into exception map
	WDBG_LH("WONException::Init Loading Exception Map");
	LoadExceptionMap();  //Defined in WONException.h

	// Init Memory exception and install new handler
	WDBG_LH("WONException::Init Installing new handler");
	gMemExcept.BuildText();
#ifdef WIN32
	gOldNewHandler = _set_new_handler(WONNewHandler);
#endif
}


// WONException::Raise
// Update line and file if needed and throw this.
void
WONException::Raise(int theLine, const char* theFileP)
{
	WTRACE("WONException::Raise");
	if ((theLine > 0) || (theFileP))
	{
		if (theLine > 0) mLine = theLine;
		if (theFileP)    mFile = theFileP;
		mWhat.erase();
	}

	WDBG_LH("WONException::Raise Throwing exception: " << *this);
	throw *this;
}


// WONException::what
// Returns exception text.  Generates text only if needed (mWhat empty).
// Text includes app info, exception code, state, standard text, additional
// text from the stream, and the line and file.
const char*
WONException::what(void) const throw()
{
	if (mWhat.size() == 0)
	{
		char aBuf[33];
		sprintf(aBuf, "%ld", mCode);

		// Add code and state
		mWhat = "Exception (";
		mWhat += aBuf;
		mWhat += " - ";
		mWhat += ExStateToStr(mState);
		mWhat += ") ";

		// Add standard text, lookup from exception map
		ExceptionMap::iterator anItr = gExceptMap.find(mCode);
		mWhat += (anItr != gExceptMap.end() ? anItr->second : UNDEFINED_EXCEPT);

		// Add additional data, file, and line if needed.
		if (mStream.size() > 0) { mWhat += "  ";  mWhat += mStream.str(); }
		if (mFile.size() > 0) { mWhat += "  File=";  mWhat += mFile; }

		sprintf(aBuf, "%ld", mCode);

		if (mLine > 0) { mWhat += "  Line=";  mWhat += aBuf; }
	}

	return mWhat.c_str();
}


// WONException::SetLog
// Enables or disables logging of the exception.
void
WONException::SetLog(bool theFlag) throw()
{
	mLogIt = theFlag;
}


// WONException::SetState
// Sets the exception state.
void
WONException::SetState(WONException::ExState theState) throw()
{
	mState = theState;
}


// ** MemoryException Methods **

// MemoryException::what
// Returns exception text
const char*
MemoryException::what(void) const throw()
{
	return mText.c_str();
}


// MemoryException::Raise
// Throws this exception
void
MemoryException::Raise(void)
{
	throw *this;
}


// MemoryException::BuildText
// Generates the text for a Memory Exception.  Text includes app info (from
// WONException), code, and standard text.
void
MemoryException::BuildText(void) throw()
{
	char aBuf[33];
	sprintf(aBuf, "%ld", EX_CODE);

	WTRACE("MemoryException::BuildText");
	WDBG_LL("MemoryException::BuildText Adding code");
	mText = "Exception (";
	mText += aBuf;
	mText += ") ";

	WDBG_LL("MemoryException::BuildText Adding standard text");
	ExceptionMap::iterator anItr = gExceptMap.find(EX_CODE);
	mText += (anItr != gExceptMap.end() ? anItr->second : UNDEFINED_EXCEPT);

	WDBG_LL("MemoryException::BuildText text: " << mText);
}


// MemoryException::Log
// Logs MemoryException to event log.  Goes directly to the (private)
// logging method of EventLog to bypass any memory allocation.
void
MemoryException::Log(void) const throw()
{
	WTRACE("MemoryException::Log");
	EventLog::LogEvent(EventLog::EventError, what());
}


// ** Output Operators **

// WONException::ExState
std::ostream&
operator<<(std::ostream& os, WONException::ExState theState)
{
	os << ExStateToStr(theState);
	return os;
}
