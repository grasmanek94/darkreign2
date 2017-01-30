#ifndef _MMsgObsAddPublisher_H
#define _MMsgObsAddPublisher_H

///////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsAddPublisher
//
// The AddPublisher message adds an publisher to the Observation server's publisher list.

#include <string>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsAddPublisher : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsAddPublisher(void);

	// MiniMessage Constructor
	explicit MMsgObsAddPublisher(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsAddPublisher(const MMsgObsAddPublisher& theMsgR);

	// Destructor
	~MMsgObsAddPublisher(void);

	// Operator overloads

	// Assignment
	MMsgObsAddPublisher& operator=(const MMsgObsAddPublisher& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsAddPublisher(*this);
	}

	// Member access methods
	const string&
	GetPublisherName(void) const
	{
		return mPublisherName;
	}

	const wstring&
	GetPublisherDescription(void) const
	{
		return mPublisherDescription;
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

	bool
	GetAccessPubsByName(void) const
	{
		return mAccessPubsByName;
	}

	bool
	GetOnDemandPublishing(void) const
	{
		return mOnDemandPublishing;
	}

	void
	SetPublisherName( const string &thePublisherName ) 
	{
		mPublisherName = thePublisherName;
	}

	void 
	SetConnectionType(const ConnectionTypeEnum theConnectionType )
	{
		mConnectionType = theConnectionType;
	}

	void
	SetPublisherDescription( const wstring &thePublisherDescription ) 
	{
		mPublisherDescription = thePublisherDescription;
	}

	void
	SetNetAddress( const string &theNetAddress ) 
	{
		mNetAddress = theNetAddress;
	}

	void
	SetAccessPubsByName( const bool theAccessPubsByName ) 
	{
		mAccessPubsByName = theAccessPubsByName;
	}

	void
	SetOnDemandPublishing( const bool theOnDemandPublishing ) 
	{
		mOnDemandPublishing = theOnDemandPublishing;
	}

private:

	string				mPublisherName;				// Name of the Publisher
	wstring				mPublisherDescription;		// Description of the Publisher
	ConnectionTypeEnum	mConnectionType;			// Type of connection to Publisher
	string				mNetAddress;				// Network address of Publisher <IP_string>:<Port>
	bool				mAccessPubsByName;			// Access to publications by name is supported by this publisher.
	bool				mOnDemandPublishing;		// Determines whether this publisher supports ondemand publishing 
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MMsgObsAddPublisherReply
//
// Observation server reply message for AddPublisher.  This message is sent in response to the AddPublisher
// message.  This message sends the status of the add operation as well as the unique id of the publisher added.


class MMsgObsAddPublisherReply : public MiniMessage
{
public:

	// Constructors / Destructor
	MMsgObsAddPublisherReply(void);

	// MiniMessage - will throw if MiniMessage type is not of this type
	explicit MMsgObsAddPublisherReply(const MiniMessage& theMsgR);

	// Copy
	MMsgObsAddPublisherReply(const MMsgObsAddPublisherReply& theMsgR);

	~MMsgObsAddPublisherReply(void);

	
	// Operator Overloads

	// Assignment operator overload
	MMsgObsAddPublisherReply& operator=(const MMsgObsAddPublisherReply& theMsgR);


	// Overridden methods

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);


	// Public Methods

	
	// Inlines
	TRawMsg* Duplicate(void) const
	{ 
		return new MMsgObsAddPublisherReply(*this);
	}

	ServerStatus GetStatus(void) const
	{ 
		return mStatus;
	}

	unsigned long GetPublisherId(void) const
	{ 
		return mPublisherId;
	}

	void SetStatus(ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

	void SetPublisherId(unsigned long thePublisherId)
	{ 
		mPublisherId = thePublisherId;
	}

private:
	ServerStatus	mStatus;		// Request Status
	unsigned long	mPublisherId;	// Unique Id of publisher added
};

};  // Namespace WONMsg


#endif