#ifndef _MMsgObsStatusReply_H
#define _MMsgObsStatusReply_H

// Observation server standard reply message.  This message is sent in response to many of 
// the messages the Observation Server receives.


#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class MMsgObsStatusReply : public MiniMessage
{
public:

	// Constructors / Destructor
	MMsgObsStatusReply(void);

	// MiniMessage - will throw if MiniMessage type is not of this type
	explicit MMsgObsStatusReply(const MiniMessage& theMsgR);

	// Copy
	MMsgObsStatusReply(const MMsgObsStatusReply& theMsgR);

	~MMsgObsStatusReply(void);

	
	// Operator Overloads

	// Assignment operator overload
	MMsgObsStatusReply& operator=(const MMsgObsStatusReply& theMsgR);


	// Overridden methods

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type
	void* Pack(void); 
	void  Unpack(void);


	// Public Methods

	
	// Inlines
	TRawMsg* Duplicate(void) const
	{ 
		return new MMsgObsStatusReply(*this);
	}

	ServerStatus GetStatus(void) const
	{ 
		return mStatus;
	}

	void SetStatus(ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

private:
	ServerStatus mStatus;  // Request status
};

};  // Namespace WONMsg

#endif