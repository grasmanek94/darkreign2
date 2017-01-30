#ifndef _MMsgObsPublishReply_H
#define _MMsgObsPublishReply_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsPublishReply
//
// The PublishReply message is sent to the publisher that requested that the specified data
// be published on the Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class MMsgObsPublishReply : public MiniMessage
{
public:

	// Constructors / Destructor
	MMsgObsPublishReply(void);

	// MiniMessage - will throw if MiniMessage type is not of this type
	explicit MMsgObsPublishReply(const MiniMessage& theMsgR);

	// Copy
	MMsgObsPublishReply(const MMsgObsPublishReply& theMsgR);

	~MMsgObsPublishReply(void);

	
	// Operator Overloads

	// Assignment operator overload
	MMsgObsPublishReply& operator=(const MMsgObsPublishReply& theMsgR);


	// Overridden methods

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type
	void* Pack(void); 
	void  Unpack(void);


	// Public Methods

	
	// Inlines
	TRawMsg* Duplicate(void) const
	{ 
		return new MMsgObsPublishReply(*this);
	}

	ServerStatus 
	GetStatus(void) const
	{ 
		return mStatus;
	}

	unsigned long
	GetPublicationId(void) const
	{ 
		return mPublicationId;
	}

	void 
	SetStatus(const ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

	void 
	SetPublicationId(const unsigned long thePublicationId)
	{ 
		mPublicationId = thePublicationId;
	}

private:
	ServerStatus	mStatus;			// Request status
	unsigned long	mPublicationId;		// Id of publication published
};

};  // Namespace WONMsg

#endif