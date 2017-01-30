#ifndef _MMsgObsAddSubscriber_H
#define _MMsgObsAddSubscriber_H

///////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsAddSubscriber
//
// The AddSubscriber message adds an Subscriber to the Observation server's Subscriber list.

#include <string>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsAddSubscriber : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsAddSubscriber(void);

	// MiniMessage Constructor
	explicit MMsgObsAddSubscriber(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsAddSubscriber(const MMsgObsAddSubscriber& theMsgR);

	// Destructor
	~MMsgObsAddSubscriber(void);

	// Operator overloads

	// Assignment
	MMsgObsAddSubscriber& operator=(const MMsgObsAddSubscriber& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsAddSubscriber(*this);
	}

	// Member access methods
	const string&
	GetSubscriberName(void) const
	{
		return mSubscriberName;
	}

	const wstring&
	GetSubscriberDescription(void) const
	{
		return mSubscriberDescription;
	}

	ConnectionTypeEnum 
	GetConnectionType() const
	{
		return mConnectionType;
	}

	const string&
	GetNetAddress(void) const
	{
		return mNetAddress;
	}

	void
	SetSubscriberName( const string &theSubscriberName ) 
	{
		mSubscriberName = theSubscriberName;
	}

	void 
	SetConnectionType(const ConnectionTypeEnum theConnectionType )
	{
		mConnectionType = theConnectionType;
	}

	void
	SetSubscriberDescription( const wstring &theSubscriberDescription ) 
	{
		mSubscriberDescription = theSubscriberDescription;
	}

	void
	SetNetAddress( const string &theNetAddress ) 
	{
		mNetAddress = theNetAddress;
	}

private:

	string				mSubscriberName;				// Name of the Subscriber
	wstring				mSubscriberDescription;		// Description of the Subscriber
	ConnectionTypeEnum	mConnectionType;			// Type of connection to Subscriber
	string				mNetAddress;				// Network address of Subscriber <IP_string>:<Port>
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MMsgObsAddSubscriberReply
//
// Observation server reply message for AddSubscriber.  This message is sent in response to the AddSubscriber
// message.  This message sends the status of the add operation as well as the unique id of the Subscriber added.


class MMsgObsAddSubscriberReply : public MiniMessage
{
public:

	// Constructors / Destructor
	MMsgObsAddSubscriberReply(void);

	// MiniMessage - will throw if MiniMessage type is not of this type
	explicit MMsgObsAddSubscriberReply(const MiniMessage& theMsgR);

	// Copy
	MMsgObsAddSubscriberReply(const MMsgObsAddSubscriberReply& theMsgR);

	~MMsgObsAddSubscriberReply(void);

	
	// Operator Overloads

	// Assignment operator overload
	MMsgObsAddSubscriberReply& operator=(const MMsgObsAddSubscriberReply& theMsgR);


	// Overridden methods

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);


	// Public Methods

	
	// Inlines
	TRawMsg* Duplicate(void) const
	{ 
		return new MMsgObsAddSubscriberReply(*this);
	}

	ServerStatus GetStatus(void) const
	{ 
		return mStatus;
	}

	unsigned long GetSubscriberId(void) const
	{ 
		return mSubscriberId;
	}

	void SetStatus(ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

	void SetSubscriberId(unsigned long theSubscriberId)
	{ 
		mSubscriberId = theSubscriberId;
	}

private:
	ServerStatus	mStatus;		// Request Status
	unsigned long	mSubscriberId;	// Unique Id of Subscriber added
};

};  // Namespace WONMsg


#endif