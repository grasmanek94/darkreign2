
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsRemovePublisher
//
// The RemovePublisher message removes an publisher from the Observation server's publisher list.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsRemovePublisher.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsRemovePublisher;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsRemovePublisher::MMsgObsRemovePublisher(void) :
	MiniMessage(),
	mPublisherId()
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_RemovePublisher);
}


// MiniMessage
MMsgObsRemovePublisher::MMsgObsRemovePublisher(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId()
{
	Unpack();
}

// Copy
MMsgObsRemovePublisher::MMsgObsRemovePublisher(const MMsgObsRemovePublisher& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId(theMsgR.mPublisherId)
{
}

// Destructor
MMsgObsRemovePublisher::~MMsgObsRemovePublisher(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsRemovePublisher&
MMsgObsRemovePublisher::operator=(const MMsgObsRemovePublisher& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublisherId = theMsgR.mPublisherId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsRemovePublisher::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsRemovePublisher::Pack(void)
{
	WTRACE("MMsgObsRemovePublisher::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_RemovePublisher);

	MiniMessage::Pack();

	WDBG_LL("MMsgObsRemovePublisher::Pack Appending message data");
	AppendLong(mPublisherId);
	
	return GetDataPtr();
}


// MMsgObsRemovePublisher::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsRemovePublisher::Unpack(void)
{
	WTRACE("MMsgObsRemovePublisher::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_RemovePublisher))
	{
		WDBG_AH("MMsgObsRemovePublisher::Unpack Not a ObsRemovePublisher message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsRemovePublisher message.");
	}

	WDBG_LL("MMsgObsRemovePublisher::Unpack Reading message data");
	mPublisherId        = ReadLong();
}
