
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsEnumerateSubscriptions
//
// The EnumerateSubscriptionsReply message sends a list of all of the Subscriptions of this Observation server that 
// meet the criteria in the EnumerateSubscriptions message.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsEnumSubscriptions.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsEnumerateSubscriptions;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsEnumerateSubscriptions::MMsgObsEnumerateSubscriptions(void) :
	MiniMessage(),
	mPublisherId(0),
	mPublicationId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumerateSubscriptions);
}


// MiniMessage
MMsgObsEnumerateSubscriptions::MMsgObsEnumerateSubscriptions(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId(0),
	mPublicationId(0)
{
	Unpack();
}

// Copy
MMsgObsEnumerateSubscriptions::MMsgObsEnumerateSubscriptions(const MMsgObsEnumerateSubscriptions& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId(theMsgR.mPublisherId),
	mPublicationId(theMsgR.mPublicationId)
{
}

// Destructor
MMsgObsEnumerateSubscriptions::~MMsgObsEnumerateSubscriptions(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsEnumerateSubscriptions&
MMsgObsEnumerateSubscriptions::operator=(const MMsgObsEnumerateSubscriptions& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublisherId = theMsgR.mPublisherId;
	mPublicationId = theMsgR.mPublicationId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsEnumerateSubscriptions::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsEnumerateSubscriptions::Pack(void)
{
	WTRACE("MMsgObsEnumerateSubscriptions::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumerateSubscriptions);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsEnumerateSubscriptions::Pack Appending message data");

	AppendLong(mPublisherId);
	AppendLong(mPublicationId);
		
	return GetDataPtr();
}


// MMsgObsEnumerateSubscriptions::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsEnumerateSubscriptions::Unpack(void)
{
	WTRACE("MMsgObsEnumerateSubscriptions::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_EnumerateSubscriptions))
	{
		WDBG_AH("MMsgObsEnumerateSubscriptions::Unpack Not a ObsEnumerateSubscriptions message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsEnumerateSubscriptions message.");
	}

	WDBG_LL("MMsgObsEnumerateSubscriptions::Unpack Reading message data");

	mPublisherId = ReadLong();
	mPublicationId = ReadLong();
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsEnumerateSubscriptionsReply
//
// The EnumerateSubscriptionsReply message sends a list of all of the Subscriptions of this Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MMsgObsEnumerateSubscriptionsReply;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsEnumerateSubscriptionsReply::MMsgObsEnumerateSubscriptionsReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mSubscriptionList()
{	

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumerateSubscriptionsReply);
}


// MiniMessage
MMsgObsEnumerateSubscriptionsReply::MMsgObsEnumerateSubscriptionsReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mSubscriptionList()
{
	Unpack();
}

// Copy
MMsgObsEnumerateSubscriptionsReply::MMsgObsEnumerateSubscriptionsReply(const MMsgObsEnumerateSubscriptionsReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mSubscriptionList(theMsgR.mSubscriptionList)
{
}

// Destructor
MMsgObsEnumerateSubscriptionsReply::~MMsgObsEnumerateSubscriptionsReply(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsEnumerateSubscriptionsReply&
MMsgObsEnumerateSubscriptionsReply::operator=(const MMsgObsEnumerateSubscriptionsReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus  = theMsgR.mStatus;
	mSubscriptionList = theMsgR.mSubscriptionList;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsEnumerateSubscriptionsReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsEnumerateSubscriptionsReply::Pack(void)
{
	WTRACE("MMsgObsEnumerateSubscriptionsReply::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_EnumerateSubscriptionsReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsEnumerateSubscriptionsReply::Pack Appending message data");

	AppendShort((short)(mStatus));
	AppendLong(mSubscriptionList.size()); // Number of Subscriptions

	for (SubscriptionList::iterator aItr = mSubscriptionList.begin(); aItr != mSubscriptionList.end(); aItr++)
	{
		AppendSubscription(*aItr);
	}
	
	return GetDataPtr();
}


// MMsgObsEnumerateSubscriptionsReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsEnumerateSubscriptionsReply::Unpack(void)
{
	WTRACE("MMsgObsEnumerateSubscriptionsReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_EnumerateSubscriptionsReply))
	{
		WDBG_AH("MMsgObsEnumerateSubscriptionsReply::Unpack Not a ObsEnumerateSubscriptionsReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsEnumerateSubscriptionsReply message.");
	}

	WDBG_LL("MMsgObsEnumerateSubscriptionsReply::Unpack Reading message data");


	mStatus = (ServerStatus)(short)(ReadShort());
	unsigned long aSubscriptionListSize = ReadLong();

	// Clear current list
	mSubscriptionList.clear();

	for (int i=0; i < aSubscriptionListSize; i++)
	{
		ReadSubscription();
	}
}


////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////

// Add Subscription to message
void
MMsgObsEnumerateSubscriptionsReply::ReadSubscription()
{
	WTRACE("MMsgObsEnumerateSubscriptionsReply::ReadSubscription");

	Subscription aSubscription;

	aSubscription.mSubscriberId		= ReadLong();
	aSubscription.mPublicationId	= ReadLong();
	
	mSubscriptionList.push_back(aSubscription);
}

// Get Subscription from message
void
MMsgObsEnumerateSubscriptionsReply::AppendSubscription(const Subscription& theSubscriptionR)
{
	WTRACE("MMsgObsEnumerateSubscriptionsReply::AppendSubscription");

	AppendLong(theSubscriptionR.mSubscriberId);
	AppendLong(theSubscriptionR.mPublicationId);
}
