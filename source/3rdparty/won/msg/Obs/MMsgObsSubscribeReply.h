#ifndef _MMsgObsSubscribeReply_H
#define _MMsgObsSubscribeReply_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsSubscribeReply
//
// The SubscribeReply message is sent to the subscriber that requested to subscribe to the specified
// data.  It contains status of the operation as well as the assigned Id of the subscription and
// the subscriber.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

// Forwards from WONSocket
namespace WONMsg {

class MMsgObsSubscribeReply : public MiniMessage
{
public:

	// Constructors / Destructor
	MMsgObsSubscribeReply(void);

	// MiniMessage - will throw if MiniMessage type is not of this type
	explicit MMsgObsSubscribeReply(const MiniMessage& theMsgR);

	// Copy
	MMsgObsSubscribeReply(const MMsgObsSubscribeReply& theMsgR);

	~MMsgObsSubscribeReply(void);

	
	// Operator Overloads

	// Assignment operator overload
	MMsgObsSubscribeReply& operator=(const MMsgObsSubscribeReply& theMsgR);


	// Overridden methods

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type
	void* Pack(void); 
	void  Unpack(void);


	// Public Methods

	
	// Inlines
	TRawMsg* Duplicate(void) const
	{ 
		return new MMsgObsSubscribeReply(*this);
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

	unsigned long
	GetSubscriberId(void) const
	{ 
		return mSubscriberId;
	}

	unsigned short
	GetDataLength(void) const
	{
		return mDataLength;
	}

	const BYTE*
	GetData(void) const
	{
		return mDataP;
	}

	const string&
	GetPeerRedirection(void) const
	{ 
		return mPeerRedirection;
	}

	void 
	SetStatus(ServerStatus theStatus)
	{ 
		mStatus = theStatus;
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
	SetData(const BYTE* theDataP, const unsigned short theDataLength )
	{
		delete [] mDataP;
		mDataLength = theDataLength;

		if (theDataLength > 0)
		{
			mDataP = new BYTE [theDataLength];
			memcpy(mDataP, theDataP, theDataLength);
		}
		else
		{
			mDataP = NULL;
		}
	}

	void
	SetPeerRedirection(const string& thePeerRedirection)
	{ 
		mPeerRedirection = thePeerRedirection;
	}

private:
	ServerStatus	mStatus;			// Request status
	unsigned long	mPublicationId;		// Id of Publication subscribed to
	unsigned long	mSubscriberId;		// Id of subscriber added
	unsigned short  mDataLength;		// Length of data pool to send
	BYTE*			mDataP;				// Data pool data
	string			mPeerRedirection;	// Address of peer to redirect to
};

};  // Namespace WONMsg

#endif