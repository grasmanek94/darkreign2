#ifndef _MMsgObsPublicationSubscribedTo_H
#define _MMsgObsPublicationSubscribedTo_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsPublicationSubscribedTo
//
// The PublicationSubscribedTo message is a message to notify a publisher when a subscriber
// subscribes to one of its publications.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"

using namespace std;

namespace WONMsg {

class MMsgObsPublicationSubscribedTo : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsPublicationSubscribedTo(void);

	// MiniMessage Constructor
	explicit MMsgObsPublicationSubscribedTo(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsPublicationSubscribedTo(const MMsgObsPublicationSubscribedTo& theMsgR);

	// Destructor
	~MMsgObsPublicationSubscribedTo(void);

	// Operator overloads

	// Assignment
	MMsgObsPublicationSubscribedTo& operator=(const MMsgObsPublicationSubscribedTo& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsPublicationSubscribedTo(*this);
	}

	// Member access methods
	unsigned long
	GetPublicationId(void) const
	{
		return mPublicationId;
	}

	unsigned long
	GetSubscriberId(void) const
	{
		return mSubscriberId;
	}

	const string&
	GetSubscriberName(void) const
	{
		return mSubscriberName;
	}

	void
	SetPublicationId(const unsigned long thePublicationId)
	{
		mPublicationId = thePublicationId;
	}

	void
	SetSubscriberId(const unsigned long theSubscriberId)
	{
		mSubscriberId = theSubscriberId;
	}

	void
	SetSubscriberName(const string& theSubscriberName)
	{
		mSubscriberName = theSubscriberName;
	}
	
private:

	unsigned long	mPublicationId;
	unsigned long	mSubscriberId;
	string			mSubscriberName;
};

}

#endif