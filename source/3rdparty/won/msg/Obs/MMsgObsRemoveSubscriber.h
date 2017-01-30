#ifndef _MMsgObsRemoveSubscriber_H
#define _MMsgObsRemoveSubscriber_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsRemoveSubscriber
//
// The RemoveSubscriber message removes the specified subscriber and all of its the subscriptions. 
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"

using namespace std;

namespace WONMsg {

class MMsgObsRemoveSubscriber : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsRemoveSubscriber(void);

	// MiniMessage Constructor
	explicit MMsgObsRemoveSubscriber(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsRemoveSubscriber(const MMsgObsRemoveSubscriber& theMsgR);

	// Destructor
	~MMsgObsRemoveSubscriber(void);

	// Operator overloads

	// Assignment
	MMsgObsRemoveSubscriber& operator=(const MMsgObsRemoveSubscriber& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsRemoveSubscriber(*this);
	}

	// Member access methods
	unsigned long
	GetSubscriberId(void) const
	{
		return mSubscriberId;
	}

	void
	SetSubscriberId(const unsigned long theSubscriberId)
	{
		mSubscriberId = theSubscriberId;
	}
	
private:
	unsigned long	mSubscriberId;
};

}

#endif