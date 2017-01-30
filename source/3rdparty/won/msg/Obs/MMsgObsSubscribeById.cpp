
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsSubscribeById
//
// The SubscribeById message uses the PublicationId to subscribe the sending server/client
// to the specified publication.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsSubscribeById.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsSubscribeById;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsSubscribeById::MMsgObsSubscribeById(void) :
	MiniMessage(),
	mPublicationId(0),
	mSubscriberId(0),
	mSubscriberName(),
	mSubscriberDescription(),
	mConnectionType(PersistentTCP),
	mSubscriberAddress()
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_SubscribeById);
}


// MiniMessage
MMsgObsSubscribeById::MMsgObsSubscribeById(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(0),
	mSubscriberId(0),
	mSubscriberName(),
	mSubscriberDescription(),
	mConnectionType(PersistentTCP),
	mSubscriberAddress()
{
	Unpack();
}

// Copy
MMsgObsSubscribeById::MMsgObsSubscribeById(const MMsgObsSubscribeById& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(theMsgR.mPublicationId),
	mSubscriberId(theMsgR.mSubscriberId),
	mSubscriberName(theMsgR.mSubscriberName),
	mSubscriberDescription(theMsgR.mSubscriberDescription),
	mConnectionType(theMsgR.mConnectionType),
	mSubscriberAddress(theMsgR.mSubscriberAddress)
{
}

// Destructor
MMsgObsSubscribeById::~MMsgObsSubscribeById(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsSubscribeById&
MMsgObsSubscribeById::operator=(const MMsgObsSubscribeById& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublicationId = theMsgR.mPublicationId;
	mSubscriberId = theMsgR.mSubscriberId;
	mSubscriberName = theMsgR.mSubscriberName;
	mSubscriberDescription = theMsgR.mSubscriberDescription;
	mConnectionType = theMsgR.mConnectionType;
	mSubscriberAddress = theMsgR.mSubscriberAddress;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsSubscribeById::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsSubscribeById::Pack(void)
{
	WTRACE("MMsgObsSubscribeById::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_SubscribeById);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsSubscribeById::Pack Appending message data");

	AppendLong(mPublicationId);
	AppendLong(mSubscriberId);
	if (!mSubscriberId) // Optional parameters follow
	{
		Append_PA_STRING(mSubscriberName);
		Append_PW_STRING(mSubscriberDescription);
		AppendShort(mConnectionType);
		Append_PA_STRING(mSubscriberAddress);
	}
	
	return GetDataPtr();
}


// MMsgObsSubscribeById::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsSubscribeById::Unpack(void)
{
	WTRACE("MMsgObsSubscribeById::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_SubscribeById))
	{
		WDBG_AH("MMsgObsSubscribeById::Unpack Not a ObsSubscribeById message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsSubscribeById message.");
	}

	WDBG_LL("MMsgObsSubscribeById::Unpack Reading message data");

	mPublicationId = ReadLong();
	mSubscriberId = ReadLong();
	if (!mSubscriberId) // Optional parameters follow
	{
		ReadString(mSubscriberName);
		ReadWString(mSubscriberDescription);
		mConnectionType = (ConnectionTypeEnum)ReadShort();
		ReadString(mSubscriberAddress);
	}
}

