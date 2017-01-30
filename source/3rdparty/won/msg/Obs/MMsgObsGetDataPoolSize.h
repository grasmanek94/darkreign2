#ifndef _MMsgObsGetDataPoolSize_H
#define _MMsgObsGetDataPoolSize_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// GetDataPoolSize
//
// The GetDataPoolSize message requests the size of the specified data pool.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

using namespace std;

namespace WONMsg {

class MMsgObsGetDataPoolSize : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsGetDataPoolSize(void);

	// MiniMessage Constructor
	explicit MMsgObsGetDataPoolSize(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsGetDataPoolSize(const MMsgObsGetDataPoolSize& theMsgR);

	// Destructor
	~MMsgObsGetDataPoolSize(void);

	// Operator overloads

	// Assignment
	MMsgObsGetDataPoolSize& operator=(const MMsgObsGetDataPoolSize& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsGetDataPoolSize(*this);
	}

	// Member access methods
	unsigned long
	GetDataPoolId(void) const
	{
		return mDataPoolId;
	}

	void
	SetDataPoolId(const unsigned long theDataPoolId)
	{
		mDataPoolId = theDataPoolId;
	}

private:

	unsigned long	mDataPoolId;
};

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetDataPoolSizeReply
//
// The GetDataPoolSizeReply message is sent to the client or server that requested the size 
// of the specified data pool.  It contains status of the operation as well as the size of
// the data pool specified by the PublicationId in the GetDataPoolSize message.   
//
///////////////////////////////////////////////////////////////////////////////////////////

class MMsgObsGetDataPoolSizeReply : public MiniMessage
{
public:

	// Constructors / Destructor
	MMsgObsGetDataPoolSizeReply(void);

	// MiniMessage - will throw if MiniMessage type is not of this type
	explicit MMsgObsGetDataPoolSizeReply(const MiniMessage& theMsgR);

	// Copy
	MMsgObsGetDataPoolSizeReply(const MMsgObsGetDataPoolSizeReply& theMsgR);

	~MMsgObsGetDataPoolSizeReply(void);

	
	// Operator Overloads

	// Assignment operator overload
	MMsgObsGetDataPoolSizeReply& operator=(const MMsgObsGetDataPoolSizeReply& theMsgR);


	// Overridden methods

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException if message is not of this type
	void* Pack(void); 
	void  Unpack(void);


	// Public Methods

	
	// Inlines
	TRawMsg* Duplicate(void) const
	{ 
		return new MMsgObsGetDataPoolSizeReply(*this);
	}

	ServerStatus 
	GetStatus(void) const
	{ 
		return mStatus;
	}

	unsigned long
	GetDataPoolSize(void) const
	{ 
		return mDataPoolSize;
	}

	void 
	SetStatus(const ServerStatus theStatus)
	{ 
		mStatus = theStatus;
	}

	void 
	SetDataPoolSize(const unsigned long theDataPoolSize)
	{ 
		mDataPoolSize = theDataPoolSize;
	}

private:
	ServerStatus	mStatus;			// Request status
	unsigned long	mDataPoolSize;		// Size of Data Pool
};


}

#endif