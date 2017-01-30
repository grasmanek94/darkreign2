// BadMsgException

// Defines Bad Message Exception class.  Instances of this class are to be
// thrown when invalid/corrupt titan messages are encountered.

// Exception Codes and standard text are defined in the WONExceptCodes.h header.

// **NOTE**
// Always catch BadMsgExceptions by REFERENCE.  This will improve performance and
// limit ownership and exception logging issues.
// **NOTE**

#include "common/won.h"
#include "common/WONException.h"
#include "common/WONExceptCodes.h"
#include "TMessage.h"
#include "BadMsgException.h"

// Private namespace
namespace {
	using WONMsg::BadMsgException;
	using WONMsg::BaseMessage;

	const char* HDRCORRUPT_MSG = "Message header is corrupt!";
};


// Standard ctor from WONException
BadMsgException::BadMsgException(int theCode, int theLine, const char* theFileP,
                                 const char* addTextP) throw() :
	WONException(theCode, theLine, theFileP, addTextP)
{}


// TMessage ctor from WONException
BadMsgException::BadMsgException(const BaseMessage& theMsgR, int theLine,
                                 const char* theFileP, const char* addTextP) throw() :
	WONException(WONCommon::ExBadTitanMessage, theLine, theFileP)
{
	WTRACE("BadMsgException::ctor(TMessage)");

	// Add header type info or header corrupt message
	if (theMsgR.GetDataLen() >= theMsgR.GetHeaderLength())
	{
		WDBG_LL("BadMsgException::ctor(TMessage) Add header info.");
		GetStream() << "MessageClass=" << (int)theMsgR.GetMessageClass()
		            << "  MessageType="  << theMsgR.GetMessageType()
		            << "  ServiceType=" << theMsgR.GetServiceType();
	}
	else
	{
		WDBG_LH("BadMsgException::ctor(TMessage) Corrupt header!");
		GetStream() << HDRCORRUPT_MSG;
	}

	// Add message length and data length
	WDBG_LL("BadMsgException::ctor(TMessage) Add length info.");
	GetStream() << "  DataLength="    << theMsgR.GetDataLen();

	// Add additional text if defined
	if (addTextP) GetStream() << "  " << addTextP;
}


// Copy Constructor
BadMsgException::BadMsgException(const BadMsgException& theExR) throw() :
	WONException(theExR)
{}


// Destructor
BadMsgException::~BadMsgException(void) throw()
{}


// ** Public Methods **

// WONException Assignment operator
BadMsgException&
BadMsgException::operator=(const BadMsgException& theExR) throw()
{
	WONException::operator=(theExR);
	return *this;
}
