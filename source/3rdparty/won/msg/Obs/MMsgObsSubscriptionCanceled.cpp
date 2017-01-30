
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsSubscriptionCanceled
//
// The SubscriptionCanceled message is sent to a subscriber when the publication they are subscribing to has been removed.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsSubscriptionCanceled.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsSubscriptionCanceled;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsSubscriptionCanceled::MMsgObsSubscriptionCanceled(void) :
	MiniMessage(),
	mPublicationId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_SubscriptionCanceled);
}


// MiniMessage
MMsgObsSubscriptionCanceled::MMsgObsSubscriptionCanceled(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(0)
{
	Unpack();
}

// Copy
MMsgObsSubscriptionCanceled::MMsgObsSubscriptionCanceled(const MMsgObsSubscriptionCanceled& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(theMsgR.mPublicationId)
{
}

// Destructor
MMsgObsSubscriptionCanceled::~MMsgObsSubscriptionCanceled(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsSubscriptionCanceled&
MMsgObsSubscriptionCanceled::operator=(const MMsgObsSubscriptionCanceled& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublicationId = theMsgR.mPublicationId;

	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsSubscriptionCanceled::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsSubscriptionCanceled::Pack(void)
{
	WTRACE("MMsgObsSubscriptionCanceled::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_SubscriptionCanceled);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsSubscriptionCanceled::Pack Appending message data");

	AppendLong(mPublicationId);
	
	return GetDataPtr();
}


// MMsgObsSubscriptionCanceled::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsSubscriptionCanceled::Unpack(void)
{
	WTRACE("MMsgObsSubscriptionCanceled::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_SubscriptionCanceled))
	{
		WDBG_AH("MMsgObsSubscriptionCanceled::Unpack Not a ObsSubscriptionCanceled message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsSubscriptionCanceled message.");
	}

	WDBG_LL("MMsgObsSubscriptionCanceled::Unpack Reading message data");

	mPublicationId = ReadLong();
}

