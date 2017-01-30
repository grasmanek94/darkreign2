
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsPublicationSubscribedTo
//
// The PublicationSubscribedTo message is a message to notify a publisher when a subscriber
// subscribes to one of its publications.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsPublicationSubTo.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsPublicationSubscribedTo;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsPublicationSubscribedTo::MMsgObsPublicationSubscribedTo(void) :
	MiniMessage(),
	mPublicationId(0),
	mSubscriberId(0),
	mSubscriberName()
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_PublicationSubscribedTo);
}


// MiniMessage
MMsgObsPublicationSubscribedTo::MMsgObsPublicationSubscribedTo(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(0),
	mSubscriberId(0),
	mSubscriberName()
{
	Unpack();
}

// Copy
MMsgObsPublicationSubscribedTo::MMsgObsPublicationSubscribedTo(const MMsgObsPublicationSubscribedTo& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(theMsgR.mPublicationId),
	mSubscriberId(theMsgR.mSubscriberId),
	mSubscriberName(theMsgR.mSubscriberName)
{
}

// Destructor
MMsgObsPublicationSubscribedTo::~MMsgObsPublicationSubscribedTo(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsPublicationSubscribedTo&
MMsgObsPublicationSubscribedTo::operator=(const MMsgObsPublicationSubscribedTo& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublicationId = theMsgR.mPublicationId;
	mSubscriberId = theMsgR.mSubscriberId;
	mSubscriberName = theMsgR.mSubscriberName;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsPublicationSubscribedTo::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsPublicationSubscribedTo::Pack(void)
{
	WTRACE("MMsgObsPublicationSubscribedTo::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_PublicationSubscribedTo);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsPublicationSubscribedTo::Pack Appending message data");

	AppendLong(mPublicationId);
	AppendLong(mSubscriberId);
	Append_PA_STRING(mSubscriberName);
	
	return GetDataPtr();
}


// MMsgObsPublicationSubscribedTo::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsPublicationSubscribedTo::Unpack(void)
{
	WTRACE("MMsgObsPublicationSubscribedTo::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_PublicationSubscribedTo))
	{
		WDBG_AH("MMsgObsPublicationSubscribedTo::Unpack Not a ObsPublicationSubscribedTo message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsPublicationSubscribedTo message.");
	}

	WDBG_LL("MMsgObsPublicationSubscribedTo::Unpack Reading message data");

	mPublicationId = ReadLong();
	mSubscriberId = ReadLong();
	ReadString(mSubscriberName);
}

