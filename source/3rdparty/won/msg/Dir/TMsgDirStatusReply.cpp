// TMsgDirStatusReply.h

// Directory Generic Reply class.  Returns the status of a request made
// to the Directory Server.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesDir.h"
#include "TMsgDirStatusReply.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirStatusReply;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirStatusReply::TMsgDirStatusReply(void) :
	TMessage(),
	mStatus(WONMsg::StatusCommon_Success)
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirStatusReply);
}


// TMessage ctor
TMsgDirStatusReply::TMsgDirStatusReply(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success)
{
	Unpack();
}


// Copy ctor
TMsgDirStatusReply::TMsgDirStatusReply(const TMsgDirStatusReply& theMsgR) :
	TMessage(theMsgR),
	mStatus(theMsgR.mStatus)
{}


// Destructor
TMsgDirStatusReply::~TMsgDirStatusReply(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirStatusReply&
TMsgDirStatusReply::operator=(const TMsgDirStatusReply& theMsgR)
{
	TMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	return *this;
}


// TMsgDirStatusReply::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirStatusReply::Pack(void)
{
	WTRACE("TMsgDirStatusReply::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirStatusReply);
	TMessage::Pack();

	WDBG_LL("TMsgDirStatusReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));

	return GetDataPtr();
}


// TMsgDirStatusReply::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirStatusReply::Unpack(void)
{
	WTRACE("TMsgDirStatusReply::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirStatusReply))
	{
		WDBG_AH("TMsgDirStatusReply::Unpack Not a DirStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirStatusReply message.");
	}

	WDBG_LL("TMsgDirStatusReply::Unpack Reading message data");
	mStatus = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));
}
