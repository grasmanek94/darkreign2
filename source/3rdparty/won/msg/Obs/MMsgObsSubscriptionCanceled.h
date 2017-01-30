#ifndef _MMsgObsSubscriptionCanceled_H
#define _MMsgObsSubscriptionCanceled_H

///////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsSubscriptionCanceled
//
// The SubscriptionCanceled message is sent to a subscriber when the publication they are subscribing to has been removed.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"

using namespace std;

namespace WONMsg {

class MMsgObsSubscriptionCanceled : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsSubscriptionCanceled(void);

	// MiniMessage Constructor
	explicit MMsgObsSubscriptionCanceled(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsSubscriptionCanceled(const MMsgObsSubscriptionCanceled& theMsgR);

	// Destructor
	~MMsgObsSubscriptionCanceled(void);

	// Operator overloads

	// Assignment
	MMsgObsSubscriptionCanceled& operator=(const MMsgObsSubscriptionCanceled& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsSubscriptionCanceled(*this);
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

	unsigned long mPublicationId;
};

}

#endif
