#ifndef _MMsgObsEnumerateSubscribers_H
#define _MMsgObsEnumerateSubscribers_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsEnumerateSubscribers
//
// The EnumerateSubscribers message requests a list of the subscribers of the Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <string>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsEnumerateSubscribers : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsEnumerateSubscribers(void);

	// MiniMessage Constructor
	explicit MMsgObsEnumerateSubscribers(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsEnumerateSubscribers(const MMsgObsEnumerateSubscribers& theMsgR);

	// Destructor
	~MMsgObsEnumerateSubscribers(void);

	// Operator overloads

	// Assignment
	MMsgObsEnumerateSubscribers& operator=(const MMsgObsEnumerateSubscribers& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsEnumerateSubscribers(*this);
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
//
// MMsgObsEnumerateSubscribersReply
//
// The EnumerateSubscribersReply message contains a list of all of the publishers on the Observation server.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MMsgObsEnumerateSubscribersReply : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsEnumerateSubscribersReply(void);

	// MiniMessage Constructor
	explicit MMsgObsEnumerateSubscribersReply(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsEnumerateSubscribersReply(const MMsgObsEnumerateSubscribersReply& theMsgR);

	// Destructor
	~MMsgObsEnumerateSubscribersReply(void);

	// Operator overloads

	// Assignment
	MMsgObsEnumerateSubscribersReply& operator=(const MMsgObsEnumerateSubscribersReply& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsEnumerateSubscribersReply(*this);
	}

	// Member access methods
	ServerStatus 
	GetStatus(void) const
	{ 
		return mStatus;
	}

	unsigned long 
	GetNumberOfSubscribers(void) const
	{ 
		return mSubscriberList.size();
	}


	const SubscriberList& 
	GetSubscriberList() const
	{
		return mSubscriberList;
	}

	// Get Publisher list for editing (Replaces Set function)
	SubscriberList& 
	GetSubscriberList()
	{
		return mSubscriberList;
	}

	void 
	SetStatus(ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

private:
	void AppendSubscriber(const Subscriber& theSubscriberListR);
	void ReadSubscriber();

private:
	ServerStatus mStatus;			// Request status
	SubscriberList mSubscriberList; // List of subscribers
};

}

#endif