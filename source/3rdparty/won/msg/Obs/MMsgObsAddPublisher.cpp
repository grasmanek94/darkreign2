
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsAddPublisher
//
// The AddPublisher message adds an publisher to the Observation server's publisher list.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsAddPublisher.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsAddPublisher;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsAddPublisher::MMsgObsAddPublisher(void) :
	MiniMessage(),
	mPublisherName(),
	mPublisherDescription(),
	mConnectionType( PersistentTCP ),
	mNetAddress(),
	mAccessPubsByName(),
	mOnDemandPublishing()
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_AddPublisher);
}


// MiniMessage
MMsgObsAddPublisher::MMsgObsAddPublisher(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherName(),
	mPublisherDescription(),
	mConnectionType(PersistentTCP),
	mNetAddress(),
	mAccessPubsByName(),
	mOnDemandPublishing()

{
	Unpack();
}

// Copy
MMsgObsAddPublisher::MMsgObsAddPublisher(const MMsgObsAddPublisher& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherName(theMsgR.mPublisherName),
	mPublisherDescription(theMsgR.mPublisherDescription),
	mConnectionType(theMsgR.mConnectionType),
	mNetAddress(theMsgR.mNetAddress),
	mAccessPubsByName(theMsgR.mAccessPubsByName),
	mOnDemandPublishing(theMsgR.mOnDemandPublishing)
{
}

// Destructor
MMsgObsAddPublisher::~MMsgObsAddPublisher(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsAddPublisher&
MMsgObsAddPublisher::operator=(const MMsgObsAddPublisher& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublisherName = theMsgR.mPublisherName;
	mPublisherDescription = theMsgR.mPublisherDescription;
	mConnectionType = theMsgR.mConnectionType;
	mNetAddress = theMsgR.mNetAddress;
	mAccessPubsByName = theMsgR.mAccessPubsByName;
	mOnDemandPublishing = theMsgR.mOnDemandPublishing;

	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsAddPublisher::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsAddPublisher::Pack(void)
{
	WTRACE("MMsgObsAddPublisher::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_AddPublisher);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsAddPublisher::Pack Appending message data");
	Append_PA_STRING(mPublisherName);
	Append_PW_STRING(mPublisherDescription);
	AppendShort((short)mConnectionType);
	Append_PA_STRING(mNetAddress);
	AppendByte((char)mAccessPubsByName);
	AppendByte((char)mOnDemandPublishing);
	
	return GetDataPtr();
}


// MMsgObsAddPublisher::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsAddPublisher::Unpack(void)
{
	WTRACE("MMsgObsAddPublisher::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_AddPublisher))
	{
		WDBG_AH("MMsgObsAddPublisher::Unpack Not a ObsAddPublisher message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsAddPublisher message.");
	}

	WDBG_LL("MMsgObsAddPublisher::Unpack Reading message data");
	ReadString(mPublisherName);
	ReadWString(mPublisherDescription);
	mConnectionType			= (ConnectionTypeEnum)ReadShort();
	ReadString(mNetAddress);
	mAccessPubsByName		= ReadByte()?true:false;
	mOnDemandPublishing		= ReadByte()?true:false;
}



//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsAddPublisherReply
//
// Observation server standard reply message.  This message is sent in response to many of 
// the messages the Observation Server receives.
//
///////////////////////////////////////////////////////////////////////////////////////////

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MMsgObsAddPublisherReply;
};

////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default 
MMsgObsAddPublisherReply::MMsgObsAddPublisherReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mPublisherId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_AddPublisherReply);
}


// MiniMessage 
MMsgObsAddPublisherReply::MMsgObsAddPublisherReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mPublisherId(0)
{
	Unpack();
}


// Copy 
MMsgObsAddPublisherReply::MMsgObsAddPublisherReply(const MMsgObsAddPublisherReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mPublisherId(theMsgR.mPublisherId)
{
}


// Destructor
MMsgObsAddPublisherReply::~MMsgObsAddPublisherReply(void)
{
}

////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsAddPublisherReply&
MMsgObsAddPublisherReply::operator=(const MMsgObsAddPublisherReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mPublisherId = theMsgR.mPublisherId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsAddPublisherReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsAddPublisherReply::Pack(void)
{
	WTRACE("MMsgObsAddPublisherReply::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_AddPublisherReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsAddPublisherReply::Pack Appending message data");
	AppendShort((short)(mStatus));
	AppendLong((unsigned long)(mPublisherId));

	return GetDataPtr();
}


// MMsgObsAddPublisherReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsAddPublisherReply::Unpack(void)
{
	WTRACE("MMsgObsAddPublisherReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_AddPublisherReply))
	{
		WDBG_AH("MMsgObsAddPublisherReply::Unpack Not an ObsAddPublisherReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not an ObsAddPublisherReply message.");
	}

	WDBG_LL("MMsgObsAddPublisherReply::Unpack Reading message data");
	mStatus = (WONMsg::ServerStatus)(short)(ReadShort());
	mPublisherId = ReadLong();
}

