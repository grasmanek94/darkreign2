#ifndef _MMsgObsEnumerateSubscriptions_H
#define _MMsgObsEnumerateSubscriptions_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsEnumerateSubscriptions
//
// The EnumerateSubscriptions message requests a list of the subscriptions of the Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsEnumerateSubscriptions : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsEnumerateSubscriptions(void);

	// MiniMessage Constructor
	explicit MMsgObsEnumerateSubscriptions(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsEnumerateSubscriptions(const MMsgObsEnumerateSubscriptions& theMsgR);

	// Destructor
	~MMsgObsEnumerateSubscriptions(void);

	// Operator overloads

	// Assignment
	MMsgObsEnumerateSubscriptions& operator=(const MMsgObsEnumerateSubscriptions& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsEnumerateSubscriptions(*this);
	}

	// Member access methods
	unsigned long
	GetPublisherId(void) const
	{
		return mPublisherId;
	}

	unsigned long
	GetPublicationId(void) const
	{
		return mPublicationId;
	}

	void
	SetPublisherId(const unsigned long thePublisherId)
	{
		mPublisherId = thePublisherId;
	}

	void
	SetPublicationId(const unsigned long thePublicationId)
	{
		mPublicationId = thePublicationId;
	}

private:

	unsigned long	mPublisherId;
	unsigned long	mPublicationId;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsEnumerateSubscriptionsReply
//
// The EnumerateSubscriptionsReply message sends a list of all of the Subscriptions of this Observation server that 
// meet the criteria in the EnumerateSubscriptions message.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MMsgObsEnumerateSubscriptionsReply : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsEnumerateSubscriptionsReply(void);

	// MiniMessage Constructor
	explicit MMsgObsEnumerateSubscriptionsReply(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsEnumerateSubscriptionsReply(const MMsgObsEnumerateSubscriptionsReply& theMsgR);

	// Destructor
	~MMsgObsEnumerateSubscriptionsReply(void);

	// Operator overloads

	// Assignment
	MMsgObsEnumerateSubscriptionsReply& operator=(const MMsgObsEnumerateSubscriptionsReply& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsEnumerateSubscriptionsReply(*this);
	}

	// Member access methods
	ServerStatus 
	GetStatus(void) const
	{ 
		return mStatus;
	}

	unsigned long 
	GetNumberOfSubscriptions(void) const
	{ 
		return mSubscriptionList.size();
	}

	const SubscriptionList& 
	GetSubscriptionList() const
	{
		return mSubscriptionList;
	}

	// Get Subscription list for editing
	SubscriptionList& 
	GetSubscriptionList()
	{
		return mSubscriptionList;
	}

	void 
	SetStatus(ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

private:
	void AppendSubscription(const Subscription& theSubscriptionR);
	void ReadSubscription();

private:
	ServerStatus mStatus;  // Request status
	SubscriptionList mSubscriptionList; // List of Subscriptions
};

}

#endif