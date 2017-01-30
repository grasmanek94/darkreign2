// TMsgCommTracing.h

// Common Tracing Message class.  Allows tracing to be turned on or
// off in WON servers.  See WONDebug.h for tracing definitions.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesComm.h"
#include "TMsgCommTracing.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgCommTracing;
};


// ** Constructors / Destructor

// Default ctor
TMsgCommTracing::TMsgCommTracing(void) :
	TMessage(),
	mTracing(false)
{
	WTRACE("TMsgCommTracing::ctor(def)");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommTracing);
}


// TMessage ctor
TMsgCommTracing::TMsgCommTracing(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mTracing(false)
{
	WTRACE("TMsgCommTracing::ctor(TMessage)");
	Unpack();
}


// Copy ctor
TMsgCommTracing::TMsgCommTracing(const TMsgCommTracing& theMsgR) :
	TMessage(theMsgR),
	mTracing(theMsgR.mTracing)
{
	WTRACE("TMsgCommTracing::ctor(copy)");
}


// Destructor
TMsgCommTracing::~TMsgCommTracing(void)
{
	WTRACE("TMsgCommTracing::destruct");
}


// ** Public Methods

// Assignment operator
TMsgCommTracing&
TMsgCommTracing::operator=(const TMsgCommTracing& theMsgR)
{
	WTRACE("TMsgCommTracing::operator=");
	TMessage::operator=(theMsgR);
	mTracing = theMsgR.mTracing;
	return *this;
}


// TMsgCommTracing::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgCommTracing::Pack(void)
{
	WTRACE("TMsgCommTracing::Pack");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommTracing);
	TMessage::Pack();

	WDBG_LL("TMsgCommTracing::Pack Appending message data");
	AppendByte(mTracing ? 1 : 0);

	return GetDataPtr();
}


// TMsgCommTracing::unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgCommTracing::Unpack(void)
{
	WTRACE("TMsgCommTracing::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommTracing))
	{
		WDBG_AH("TMsgCommTracing::Unpack Not a CommTracing message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommTracing message.");
	}

	WDBG_LL("TMsgCommTracing::Unpack Reading message data");
	mTracing = (ReadByte() != 0);
}
