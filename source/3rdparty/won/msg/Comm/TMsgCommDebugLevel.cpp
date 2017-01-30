
// TMsgCommDebugLevel.h

// Common Debug Level Message class.  Allows a debug level to be sent to
// WON servers.  See WONDebug.h for debug level definitions.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesComm.h"
#include "TMsgCommDebugLevel.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgCommDebugLevel;
};


// ** Constructors / Destructor

// Default ctor
TMsgCommDebugLevel::TMsgCommDebugLevel(void) :
	TMessage(),
	mDebugLevel()
{
	WTRACE("TMsgCommDebugLevel::ctor(def)");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommDebugLevel);
}


// TMessage ctor
TMsgCommDebugLevel::TMsgCommDebugLevel(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mDebugLevel()
{
	WTRACE("TMsgCommDebugLevel::ctor(TMessage)");
	Unpack();
}


// Copy ctor
TMsgCommDebugLevel::TMsgCommDebugLevel(const TMsgCommDebugLevel& theMsgR) :
	TMessage(theMsgR),
	mDebugLevel(theMsgR.mDebugLevel)
{
	WTRACE("TMsgCommDebugLevel::ctor(copy)");
}


// Destructor
TMsgCommDebugLevel::~TMsgCommDebugLevel(void)
{
	WTRACE("TMsgCommDebugLevel::destruct");
}


// ** Public Methods

// Assignment operator
TMsgCommDebugLevel&
TMsgCommDebugLevel::operator=(const TMsgCommDebugLevel& theMsgR)
{
	WTRACE("TMsgCommDebugLevel::operator=");
	TMessage::operator=(theMsgR);
	mDebugLevel = theMsgR.mDebugLevel;
	return *this;
}


// TMsgCommDebugLevel::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgCommDebugLevel::Pack(void)
{
	WTRACE("TMsgCommDebugLevel::Pack");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommDebugLevel);
	TMessage::Pack();

	WDBG_LL("TMsgCommDebugLevel::Pack Appending message data");
	Append_PA_STRING(mDebugLevel);

	return GetDataPtr();
}


// TMsgCommDebugLevel::unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgCommDebugLevel::Unpack(void)
{
	WTRACE("TMsgCommDebugLevel::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommDebugLevel))
	{
		WDBG_AH("TMsgCommDebugLevel::Unpack Not a CommDebugLevel message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommDebugLevel message.");
	}

	WDBG_LL("TMsgCommDebugLevel::Unpack Reading message data");
	ReadString(mDebugLevel);
}
