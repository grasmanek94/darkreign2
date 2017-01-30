#ifndef _MMsgObsGetNumSubscribers_H
#define _MMsgObsGetNumSubscribers_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetNumSubscribers
//
// The GetNumSubscribers message requests the number of subscribers of a certain publication
// or the whole Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

using namespace std;

namespace WONMsg {

class MMsgObsGetNumSubscribers : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsGetNumSubscribers(void);

	// MiniMessage Constructor
	explicit MMsgObsGetNumSubscribers(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsGetNumSubscribers(const MMsgObsGetNumSubscribers& theMsgR);

	// Destructor
	~MMsgObsGetNumSubscribers(void);

	// Operator overloads

	// Assignment
	MMsgObsGetNumSubscribers& operator=(const MMsgObsGetNumSubscribers& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsGetNumSubscribers(*this);
	}

	// Member access methods
	unsigned long
	GetPublicationId(void) const
	{
		return mPublicationId;
	}

	void
	SetPublicationId(const unsigned long thePublicationId)
	{
		mPublicationId = thePublicationId;
	}
	
private:

	unsigned long	mPublicationId;
};

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetNumSubscribersReply
//
// The GetNumSubscribersReply message is sent to the client or server that requested the number
// of subscribers.  It contains status of the operation as well as the number of subscribers of
// the publication specified by the PublicationId in the GetNumSubscribers message. 
//
///////////////////////////////////////////////////////////////////////////////////////////

class MMsgObsGetNumSubscribersReply : public MiniMessage
{
public:

	// Constructors / Destructor
	MMsgObsGetNumSubscribersReply(void);

	// MiniMessage - will throw if MiniMessage type is not of this type
	explicit MMsgObsGetNumSubscribersReply(const MiniMessage& theMsgR);

	// Copy
	MMsgObsGetNumSubscribersReply(const MMsgObsGetNumSubscribersReply& theMsgR);

	~MMsgObsGetNumSubscribersReply(void);

	
	// Operator Overloads

	// Assignment operator overload
	MMsgObsGetNumSubscribersReply& operator=(const MMsgObsGetNumSubscribersReply& theMsgR);


	// Overridden methods

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type
	void* Pack(void); 
	void  Unpack(void);


	// Public Methods

	
	// Inlines
	TRawMsg* Duplicate(void) const
	{ 
		return new MMsgObsGetNumSubscribersReply(*this);
	}

	ServerStatus 
	GetStatus(void) const
	{ 
		return mStatus;
	}

	unsigned long
	GetNumSubscribers(void) const
	{ 
		return mNumSubscribers;
	}

	void 
	SetStatus(const ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

	void 
	SetNumSubscribers(const unsigned long theNumSubscribers)
	{ 
		mNumSubscribers = theNumSubscribers;
	}

private:
	ServerStatus	mStatus;				// Request status
	unsigned long	mNumSubscribers;		// Number of subscribers
};

}

#endif