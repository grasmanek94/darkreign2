
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsRemoveSubscriber
//
// The RemoveSubscriber message removes the specified subscriber and all of its the subscriptions. 
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsRemoveSubscriber.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsRemoveSubscriber;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsRemoveSubscriber::MMsgObsRemoveSubscriber(void) :
	MiniMessage(),
	mSubscriberId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_RemoveSubscriber);
}


// MiniMessage
MMsgObsRemoveSubscriber::MMsgObsRemoveSubscriber(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mSubscriberId(0)
{
	Unpack();
}

// Copy
MMsgObsRemoveSubscriber::MMsgObsRemoveSubscriber(const MMsgObsRemoveSubscriber& theMsgR) :
	MiniMessage(theMsgR),
	mSubscriberId(theMsgR.mSubscriberId)
{
}

// Destructor
MMsgObsRemoveSubscriber::~MMsgObsRemoveSubscriber(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsRemoveSubscriber&
MMsgObsRemoveSubscriber::operator=(const MMsgObsRemoveSubscriber& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mSubscriberId = theMsgR.mSubscriberId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsRemoveSubscriber::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsRemoveSubscriber::Pack(void)
{
	WTRACE("MMsgObsRemoveSubscriber::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_RemoveSubscriber);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsRemoveSubscriber::Pack Appending message data");

	AppendLong(mSubscriberId);

	return GetDataPtr();
}


// MMsgObsRemoveSubscriber::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsRemoveSubscriber::Unpack(void)
{
	WTRACE("MMsgObsRemoveSubscriber::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_RemoveSubscriber))
	{
		WDBG_AH("MMsgObsRemoveSubscriber::Unpack Not a ObsRemoveSubscriber message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsRemoveSubscriber message.");
	}

	WDBG_LL("MMsgObsRemoveSubscriber::Unpack Reading message data");

	mSubscriberId = ReadLong();
}

