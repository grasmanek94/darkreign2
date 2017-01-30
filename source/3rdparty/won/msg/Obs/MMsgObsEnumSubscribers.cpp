
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsEnumerateSubscribers
//
// The EnumerateSubscribers message requests a list of the subscribers of the Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsEnumSubscribers.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsEnumerateSubscribers;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsEnumerateSubscribers::MMsgObsEnumerateSubscribers(void) :
	MiniMessage(),
	mPublisherId(0),
	mPublicationId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumerateSubscribers);
}


// MiniMessage
MMsgObsEnumerateSubscribers::MMsgObsEnumerateSubscribers(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId(0),
	mPublicationId(0)
{
	Unpack();
}

// Copy
MMsgObsEnumerateSubscribers::MMsgObsEnumerateSubscribers(const MMsgObsEnumerateSubscribers& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId(theMsgR.mPublisherId),
	mPublicationId(theMsgR.mPublicationId)
{
}

// Destructor
MMsgObsEnumerateSubscribers::~MMsgObsEnumerateSubscribers(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsEnumerateSubscribers&
MMsgObsEnumerateSubscribers::operator=(const MMsgObsEnumerateSubscribers& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublisherId = theMsgR.mPublisherId;
	mPublicationId = theMsgR.mPublicationId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsEnumerateSubscribers::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsEnumerateSubscribers::Pack(void)
{
	WTRACE("MMsgObsEnumerateSubscribers::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumerateSubscribers);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsEnumerateSubscribers::Pack Appending message data");

	AppendLong(mPublisherId);
	AppendLong(mPublicationId);
		
	return GetDataPtr();
}


// MMsgObsEnumerateSubscribers::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsEnumerateSubscribers::Unpack(void)
{
	WTRACE("MMsgObsEnumerateSubscribers::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_EnumerateSubscribers))
	{
		WDBG_AH("MMsgObsEnumerateSubscribers::Unpack Not a ObsEnumerateSubscribers message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsEnumerateSubscribers message.");
	}

	WDBG_LL("MMsgObsEnumerateSubscribers::Unpack Reading message data");

	mPublisherId = ReadLong();
	mPublicationId = ReadLong();
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsEnumerateSubscribersReply
//
// The EnumerateSubscribersReply message contains a list of all of the publishers on the Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MMsgObsEnumerateSubscribersReply;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsEnumerateSubscribersReply::MMsgObsEnumerateSubscribersReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mSubscriberList()
{	

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumerateSubscribersReply);
}


// MiniMessage
MMsgObsEnumerateSubscribersReply::MMsgObsEnumerateSubscribersReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mSubscriberList()
{
	Unpack();
}

// Copy
MMsgObsEnumerateSubscribersReply::MMsgObsEnumerateSubscribersReply(const MMsgObsEnumerateSubscribersReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mSubscriberList(theMsgR.mSubscriberList)
{
}

// Destructor
MMsgObsEnumerateSubscribersReply::~MMsgObsEnumerateSubscribersReply(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsEnumerateSubscribersReply&
MMsgObsEnumerateSubscribersReply::operator=(const MMsgObsEnumerateSubscribersReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus  = theMsgR.mStatus;
	mSubscriberList = theMsgR.mSubscriberList;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsEnumerateSubscribersReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsEnumerateSubscribersReply::Pack(void)
{
	WTRACE("MMsgObsEnumerateSubscribersReply::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumerateSubscribersReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsEnumerateSubscribersReply::Pack Appending message data");

	AppendShort((short)(mStatus));
	AppendLong(mSubscriberList.size()); // Number of publishers

	for (SubscriberList::iterator aItr = mSubscriberList.begin(); aItr != mSubscriberList.end(); aItr++)
	{
		AppendSubscriber(*aItr);
	}
	
	return GetDataPtr();
}


// MMsgObsEnumerateSubscribersReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsEnumerateSubscribersReply::Unpack(void)
{
	WTRACE("MMsgObsEnumerateSubscribersReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_EnumerateSubscribersReply))
	{
		WDBG_AH("MMsgObsEnumerateSubscribersReply::Unpack Not a ObsEnumerateSubscribersReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsEnumerateSubscribersReply message.");
	}

	WDBG_LL("MMsgObsEnumerateSubscribersReply::Unpack Reading message data");


	mStatus = (ServerStatus)(short)(ReadShort());
	unsigned long aSubscriberListSize = ReadLong();

	// Clear current list
	mSubscriberList.clear();

	for (int i=0; i < aSubscriberListSize; i++)
	{
		ReadSubscriber();
	}
}


////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////

// Add Subscriber to message
void
MMsgObsEnumerateSubscribersReply::ReadSubscriber()
{
	WTRACE("MMsgObsEnumerateSubscribersReply::ReadSubscriber");

	Subscriber aSubscriber;

	aSubscriber.mId						= ReadLong();
	ReadString(aSubscriber.mName);
	ReadWString(aSubscriber.mSubscriberDescription);
	aSubscriber.mConnectionType			= (ConnectionTypeEnum)ReadShort();
	ReadString(aSubscriber.mNetworkAddress);
	
	mSubscriberList.push_back(aSubscriber);
}

// Get Subscriber from message
void
MMsgObsEnumerateSubscribersReply::AppendSubscriber(const Subscriber& theSubscriberR)
{
	WTRACE("MMsgObsEnumerateSubscribersReply::AppendSubscriber");

	AppendLong(theSubscriberR.mId);					
	Append_PA_STRING(theSubscriberR.mName);					
	Append_PW_STRING(theSubscriberR.mSubscriberDescription);
	AppendShort(theSubscriberR.mConnectionType);		
	Append_PA_STRING(theSubscriberR.mNetworkAddress);
}


