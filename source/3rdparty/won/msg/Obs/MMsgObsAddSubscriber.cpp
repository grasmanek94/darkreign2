
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsAddSubscriber
//
// The AddSubscriber message adds an Subscriber to the Observation server's Subscriber list.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsAddSubscriber.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsAddSubscriber;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsAddSubscriber::MMsgObsAddSubscriber(void) :
	MiniMessage(),
	mSubscriberName(),
	mSubscriberDescription(),
	mConnectionType( PersistentTCP ),
	mNetAddress()	
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_AddSubscriber);
}


// MiniMessage
MMsgObsAddSubscriber::MMsgObsAddSubscriber(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mSubscriberName(),
	mSubscriberDescription(),
	mConnectionType(PersistentTCP),
	mNetAddress()
{
	Unpack();
}

// Copy
MMsgObsAddSubscriber::MMsgObsAddSubscriber(const MMsgObsAddSubscriber& theMsgR) :
	MiniMessage(theMsgR),
	mSubscriberName(theMsgR.mSubscriberName),
	mSubscriberDescription(theMsgR.mSubscriberDescription),
	mConnectionType(theMsgR.mConnectionType),
	mNetAddress(theMsgR.mNetAddress)
{
}

// Destructor
MMsgObsAddSubscriber::~MMsgObsAddSubscriber(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsAddSubscriber&
MMsgObsAddSubscriber::operator=(const MMsgObsAddSubscriber& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mSubscriberName = theMsgR.mSubscriberName;
	mSubscriberDescription = theMsgR.mSubscriberDescription;
	mConnectionType = theMsgR.mConnectionType;
	mNetAddress = theMsgR.mNetAddress;
	
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsAddSubscriber::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsAddSubscriber::Pack(void)
{
	WTRACE("MMsgObsAddSubscriber::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_AddSubscriber);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsAddSubscriber::Pack Appending message data");
	Append_PA_STRING(mSubscriberName);
	Append_PW_STRING(mSubscriberDescription);
	AppendShort((short)mConnectionType);
	Append_PA_STRING(mNetAddress);
		
	return GetDataPtr();
}


// MMsgObsAddSubscriber::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsAddSubscriber::Unpack(void)
{
	WTRACE("MMsgObsAddSubscriber::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_AddSubscriber))
	{
		WDBG_AH("MMsgObsAddSubscriber::Unpack Not a ObsAddSubscriber message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsAddSubscriber message.");
	}

	WDBG_LL("MMsgObsAddSubscriber::Unpack Reading message data");
	ReadString(mSubscriberName);
	ReadWString(mSubscriberDescription);
	mConnectionType			= (ConnectionTypeEnum)ReadShort();
	ReadString(mNetAddress);
}



//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsAddSubscriberReply
//
// Observation server standard reply message.  This message is sent in response to many of 
// the messages the Observation Server receives.
//
///////////////////////////////////////////////////////////////////////////////////////////

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MMsgObsAddSubscriberReply;
};

////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default 
MMsgObsAddSubscriberReply::MMsgObsAddSubscriberReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mSubscriberId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_AddSubscriberReply);
}


// MiniMessage 
MMsgObsAddSubscriberReply::MMsgObsAddSubscriberReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mSubscriberId(0)
{
	Unpack();
}


// Copy 
MMsgObsAddSubscriberReply::MMsgObsAddSubscriberReply(const MMsgObsAddSubscriberReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mSubscriberId(theMsgR.mSubscriberId)
{
}


// Destructor
MMsgObsAddSubscriberReply::~MMsgObsAddSubscriberReply(void)
{
}

////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsAddSubscriberReply&
MMsgObsAddSubscriberReply::operator=(const MMsgObsAddSubscriberReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mSubscriberId = theMsgR.mSubscriberId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsAddSubscriberReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsAddSubscriberReply::Pack(void)
{
	WTRACE("MMsgObsAddSubscriberReply::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_AddSubscriberReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsAddSubscriberReply::Pack Appending message data");
	AppendShort((short)(mStatus));
	AppendLong((unsigned long)(mSubscriberId));

	return GetDataPtr();
}


// MMsgObsAddSubscriberReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsAddSubscriberReply::Unpack(void)
{
	WTRACE("MMsgObsAddSubscriberReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_AddSubscriberReply))
	{
		WDBG_AH("MMsgObsAddSubscriberReply::Unpack Not an ObsAddSubscriberReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not an ObsAddSubscriberReply message.");
	}

	WDBG_LL("MMsgObsAddSubscriberReply::Unpack Reading message data");
	mStatus = (WONMsg::ServerStatus)(short)(ReadShort());
	mSubscriberId = ReadLong();
}

