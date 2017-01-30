#ifndef _MMsgObsCancelSubscription_H
#define _MMsgObsCancelSubscription_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsCancelSubscription
//
// The CancelSubscription message cancels the subscription of this subscriber to the
// subscription specified.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"

using namespace std;

namespace WONMsg {

class MMsgObsCancelSubscription : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsCancelSubscription(void);

	// MiniMessage Constructor
	explicit MMsgObsCancelSubscription(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsCancelSubscription(const MMsgObsCancelSubscription& theMsgR);

	// Destructor
	~MMsgObsCancelSubscription(void);

	// Operator overloads

	// Assignment
	MMsgObsCancelSubscription& operator=(const MMsgObsCancelSubscription& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsCancelSubscription(*this);
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

	void
	SetPublicationId(const unsigned long thePublicationId)
	{
		mPublicationId= thePublicationId;
	}

	void
	SetSubscriberId(const unsigned long theSubscriberId)
	{
		mSubscriberId = theSubscriberId;
	}
	
private:

	unsigned long	mPublicationId;
	unsigned long	mSubscriberId;
};

}

#endif