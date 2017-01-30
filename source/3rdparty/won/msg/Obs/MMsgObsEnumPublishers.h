#ifndef _MMsgObsEnumeratePublishers_H
#define _MMsgObsEnumeratePublishers_H

//////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsEnumeratePublishers
//
// The EnumeratePublishers message requests a list of all of the publishers of this Observation server.

#include <list>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsEnumeratePublishers : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsEnumeratePublishers(void);

	// MiniMessage Constructor
	explicit MMsgObsEnumeratePublishers(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsEnumeratePublishers(const MMsgObsEnumeratePublishers& theMsgR);

	// Destructor
	~MMsgObsEnumeratePublishers(void);

	// Operator overloads

	// Assignment
	MMsgObsEnumeratePublishers& operator=(const MMsgObsEnumeratePublishers& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsEnumeratePublishers(*this);
	}

	// Member access methods - no members
	
private:
	// No message specific message header info
	
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsEnumeratePublishersReply
//
// The EnumeratePublishersReply message sends a list of all of the publishers of this Observation server.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MMsgObsEnumeratePublishersReply : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsEnumeratePublishersReply(void);

	// MiniMessage Constructor
	explicit MMsgObsEnumeratePublishersReply(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsEnumeratePublishersReply(const MMsgObsEnumeratePublishersReply& theMsgR);

	// Destructor
	~MMsgObsEnumeratePublishersReply(void);

	// Operator overloads

	// Assignment
	MMsgObsEnumeratePublishersReply& operator=(const MMsgObsEnumeratePublishersReply& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsEnumeratePublishersReply(*this);
	}

	// Member access methods
	ServerStatus GetStatus(void) const
	{ 
		return mStatus;
	}

	unsigned long GetNumberOfPublishers(void) const
	{ 
		return mPublisherList.size();
	}

	const PublisherList& GetPublisherList() const
	{
		return mPublisherList;
	}

	// Get Publisher list for editing
	PublisherList& GetPublisherList()
	{
		return mPublisherList;
	}

	void SetStatus(ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

private:
	void AppendPublisher(const Publisher& thePublisherR);
	void ReadPublisher();

private:
	ServerStatus mStatus;  // Request status
	PublisherList mPublisherList; // List of publishers
};

};  // Namespace WONMsg

#endif