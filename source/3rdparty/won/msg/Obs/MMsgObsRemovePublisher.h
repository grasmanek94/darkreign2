#ifndef _MMsgObsRemovePublisher_H
#define _MMsgObsRemovePublisher_H

// 
// TMsObsRemovePublisher

// The RemovePublisher message removes an publisher to the Observation server's publisher list.

#include "msg/TMessage.h"

using namespace std;

namespace WONMsg {

class MMsgObsRemovePublisher : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsRemovePublisher();

	// MiniMessage Constructor
	explicit MMsgObsRemovePublisher(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsRemovePublisher(const MMsgObsRemovePublisher& theMsgR);

	// Destructor
	~MMsgObsRemovePublisher();

	// Operator overloads

	// Assignment
	MMsgObsRemovePublisher& operator=(const MMsgObsRemovePublisher& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsRemovePublisher(*this);
	}

	// Member access methods
	unsigned long
	GetPublisherId(void) const
	{
		return mPublisherId;
	}

	void
	SetPublisherId( const unsigned long thePublisherId ) 
	{
		mPublisherId = thePublisherId;
	}

private:

	unsigned long mPublisherId;		// Unique Identifier of the Publisher to remove

};

};  // Namespace WONMsg

#endif