
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsCancelSubscription
//
// The CancelSubscription message cancels the subscription of this subscriber to the
// subscription specified.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsCancelSubscription.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsCancelSubscription;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsCancelSubscription::MMsgObsCancelSubscription(void) :
	MiniMessage(),
	mPublicationId(0),
	mSubscriberId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_CancelSubscription);
}


// MiniMessage
MMsgObsCancelSubscription::MMsgObsCancelSubscription(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(0),
	mSubscriberId(0)
{
	Unpack();
}

// Copy
MMsgObsCancelSubscription::MMsgObsCancelSubscription(const MMsgObsCancelSubscription& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(theMsgR.mPublicationId),
	mSubscriberId(theMsgR.mSubscriberId)
{
}

// Destructor
MMsgObsCancelSubscription::~MMsgObsCancelSubscription(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsCancelSubscription&
MMsgObsCancelSubscription::operator=(const MMsgObsCancelSubscription& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublicationId = theMsgR.mPublicationId;
	mSubscriberId = theMsgR.mSubscriberId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsCancelSubscription::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsCancelSubscription::Pack(void)
{
	WTRACE("MMsgObsCancelSubscription::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_CancelSubscription);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsCancelSubscription::Pack Appending message data");

	AppendLong(mPublicationId);
	AppendLong(mSubscriberId);
	
	return GetDataPtr();
}


// MMsgObsCancelSubscription::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsCancelSubscription::Unpack(void)
{
	WTRACE("MMsgObsCancelSubscription::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_CancelSubscription))
	{
		WDBG_AH("MMsgObsCancelSubscription::Unpack Not a ObsCancelSubscription message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsCancelSubscription message.");
	}

	WDBG_LL("MMsgObsCancelSubscription::Unpack Reading message data");

	mPublicationId = ReadLong();
	mSubscriberId = ReadLong();
}

