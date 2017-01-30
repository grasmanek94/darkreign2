#ifndef _MMsgObsGetIdReply_H
#define _MMsgObsGetIdReply_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetIdReply
//
// The GetIdReply message is sent to the client or server that requested the identifier of
// the specified resource.  It contains status of the operation as well as the unique identifier
// of the resource specified by the resource name in the GetPublisherId or GetPublicationId message.  
//
///////////////////////////////////////////////////////////////////////////////////////////

#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class MMsgObsGetIdReply : public MiniMessage
{
public:

	// Constructors / Destructor
	MMsgObsGetIdReply(void);

	// MiniMessage - will throw if MiniMessage type is not of this type
	explicit MMsgObsGetIdReply(const MiniMessage& theMsgR);

	// Copy
	MMsgObsGetIdReply(const MMsgObsGetIdReply& theMsgR);

	~MMsgObsGetIdReply(void);

	
	// Operator Overloads

	// Assignment operator overload
	MMsgObsGetIdReply& operator=(const MMsgObsGetIdReply& theMsgR);


	// Overridden methods

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type
	void* Pack(void); 
	void  Unpack(void);


	// Public Methods

	
	// Inlines
	TRawMsg* Duplicate(void) const
	{ 
		return new MMsgObsGetIdReply(*this);
	}

	ServerStatus 
	GetStatus(void) const
	{ 
		return mStatus;
	}

	unsigned long
	GetResourceId(void) const
	{ 
		return mResourceId;
	}

	void 
	SetStatus(const ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

	void 
	SetResourceId(const unsigned long theResourceId)
	{ 
		mResourceId = theResourceId;
	}

private:
	ServerStatus	mStatus;			// Request status
	unsigned long	mResourceId;		// Id of publisher, publication, or subscriber
};

};  // Namespace WONMsg

#endif