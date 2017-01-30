// TMsgCommTimeout.h

// Common Message Timeout Message class.  Allows the message timeout for
// WON servers to be set.  May be set to infinite (value <= 0) or a number
// or milliseconds (value > 0)


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesComm.h"
#include "TMsgCommTimeout.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgCommTimeout;
};


// ** Constructors / Destructor

// Default ctor
TMsgCommTimeout::TMsgCommTimeout(void) :
	TMessage(),
	mTimeout(INFINITE)
{
	WTRACE("TMsgCommTimeout::ctor(def)");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommMsgTimeout);
}


// TMessage ctor
TMsgCommTimeout::TMsgCommTimeout(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mTimeout(INFINITE)
{
	WTRACE("TMsgCommTimeout::ctor(TMessage)");
	Unpack();
}


// Copy ctor
TMsgCommTimeout::TMsgCommTimeout(const TMsgCommTimeout& theMsgR) :
	TMessage(theMsgR),
	mTimeout(theMsgR.mTimeout)
{
	WTRACE("TMsgCommTimeout::ctor(copy)");
}


// Destructor
TMsgCommTimeout::~TMsgCommTimeout(void)
{
	WTRACE("TMsgCommTimeout::destruct");
}


// ** Public Methods

// Assignment operator
TMsgCommTimeout&
TMsgCommTimeout::operator=(const TMsgCommTimeout& theMsgR)
{
	WTRACE("TMsgCommTimeout::operator=");
	TMessage::operator=(theMsgR);
	mTimeout = theMsgR.mTimeout;
	return *this;
}


// TMsgCommTimeout::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgCommTimeout::Pack(void)
{
	WTRACE("TMsgCommTimeout::Pack");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommMsgTimeout);
	TMessage::Pack();

	WDBG_LL("TMsgCommTimeout::Pack Appending message data");
	AppendLong(mTimeout > 0 ? mTimeout : -1);

	return GetDataPtr();
}


// TMsgCommTimeout::unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgCommTimeout::Unpack(void)
{
	WTRACE("TMsgCommTimeout::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommMsgTimeout))
	{
		WDBG_AH("TMsgCommTimeout::Unpack Not a CommMsgTimeout message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommMsgTimeout message.");
	}

	WDBG_LL("TMsgCommTimeout::Unpack Reading message data");
	mTimeout = ReadLong();
	if (mTimeout <= 0) mTimeout = INFINITE;
}
