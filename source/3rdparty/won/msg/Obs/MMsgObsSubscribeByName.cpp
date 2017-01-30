
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsSubscribeByName
//
// The SubscribeByName message uses the PublicationName to subscribe the sending server/client
// to the specified publication.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsSubscribeByName.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsSubscribeByName;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsSubscribeByName::MMsgObsSubscribeByName(void) :
	MiniMessage(),
	mPublicationName(),
	mPublisherId(0),
	mSubscriberId(0),
	mSubscriberName(),
	mSubscriberDescription(),
	mConnectionType(PersistentTCP),
	mSubscriberAddress(),
	mRequestPublish(false)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_SubscribeByName);
}


// MiniMessage
MMsgObsSubscribeByName::MMsgObsSubscribeByName(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationName(),
	mPublisherId(0),
	mSubscriberId(0),
	mSubscriberName(),
	mSubscriberDescription(),
	mConnectionType(PersistentTCP),
	mSubscriberAddress(),
	mRequestPublish(false)
{
	Unpack();
}

// Copy
MMsgObsSubscribeByName::MMsgObsSubscribeByName(const MMsgObsSubscribeByName& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationName(theMsgR.mPublicationName),
	mPublisherId(theMsgR.mPublisherId),
	mSubscriberId(theMsgR.mSubscriberId),
	mSubscriberName(theMsgR.mSubscriberName),
	mSubscriberDescription(theMsgR.mSubscriberDescription),
	mConnectionType(theMsgR.mConnectionType),
	mSubscriberAddress(theMsgR.mSubscriberAddress),
	mRequestPublish(theMsgR.mRequestPublish)
{
}

// Destructor
MMsgObsSubscribeByName::~MMsgObsSubscribeByName(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsSubscribeByName&
MMsgObsSubscribeByName::operator=(const MMsgObsSubscribeByName& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublicationName = theMsgR.mPublicationName;
	mPublisherId = theMsgR.mPublisherId;
	mSubscriberId = theMsgR.mSubscriberId;
	mSubscriberName = theMsgR.mSubscriberName;
	mSubscriberDescription = theMsgR.mSubscriberDescription;
	mConnectionType = theMsgR.mConnectionType;
	mSubscriberAddress = theMsgR.mSubscriberAddress;
	mRequestPublish = theMsgR.mRequestPublish;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsSubscribeByName::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsSubscribeByName::Pack(void)
{
	WTRACE("MMsgObsSubscribeByName::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_SubscribeByName);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsSubscribeByName::Pack Appending message data");

	Append_PA_STRING(mPublicationName);
	AppendLong(mPublisherId);
	AppendLong(mSubscriberId);
	if(!mSubscriberId) // Optional parameters follow
	{
		Append_PA_STRING(mSubscriberName);
		Append_PW_STRING(mSubscriberDescription);
		AppendShort(mConnectionType);
		Append_PA_STRING(mSubscriberAddress);
	}
	AppendShort(mRequestPublish);

	return GetDataPtr();
}


// MMsgObsSubscribeByName::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsSubscribeByName::Unpack(void)
{
	WTRACE("MMsgObsSubscribeByName::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_SubscribeByName))
	{
		WDBG_AH("MMsgObsSubscribeByName::Unpack Not a ObsSubscribeByName message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsSubscribeByName message.");
	}

	WDBG_LL("MMsgObsSubscribeByName::Unpack Reading message data");

	ReadString(mPublicationName);
	mPublisherId = ReadLong();
	mSubscriberId = ReadLong();
	if (!mSubscriberId) // Optional parameters follow
	{
		ReadString(mSubscriberName);
		ReadWString(mSubscriberDescription);
		mConnectionType = (ConnectionTypeEnum)ReadShort();
		ReadString(mSubscriberAddress);
	}
	mRequestPublish = ReadShort() ? true : false;
}

