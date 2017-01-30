
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsStatusReply
//
// Observation server standard reply message.  This message is sent in response to many of 
// the messages the Observation Server receives.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "msg/ServerStatus.h"
#include "MMsgObsStatusReply.h"



// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsStatusReply;
};

////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default 
MMsgObsStatusReply::MMsgObsStatusReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_StatusReply);
}


// MiniMessage 
MMsgObsStatusReply::MMsgObsStatusReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success)
{
	Unpack();
}


// Copy 
MMsgObsStatusReply::MMsgObsStatusReply(const MMsgObsStatusReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus)
{
}


// Destructor
MMsgObsStatusReply::~MMsgObsStatusReply(void)
{
}

////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsStatusReply&
MMsgObsStatusReply::operator=(const MMsgObsStatusReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsStatusReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsStatusReply::Pack(void)
{
	WTRACE("MMsgObsStatusReply::Pack");
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_StatusReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsStatusReply::Pack Appending message data");
	AppendShort((short)(mStatus));

	return GetDataPtr();
}


// MMsgObsStatusReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsStatusReply::Unpack(void)
{
	WTRACE("MMsgObsStatusReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_StatusReply))
	{
		WDBG_AH("MMsgObsStatusReply::Unpack Not an ObsStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not an ObsStatusReply message.");
	}

	WDBG_LL("MMsgObsStatusReply::Unpack Reading message data");
	mStatus = (WONMsg::ServerStatus)(short)(ReadShort());
}
