#ifndef _MMsgObsGetDataPool_H
#define _MMsgObsGetDataPool_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// GetDataPool
//
// The GetDataPool message requests the Observation server to send the data pool specified.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

using namespace std;

namespace WONMsg {

class MMsgObsGetDataPool : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsGetDataPool(void);

	// MiniMessage Constructor
	explicit MMsgObsGetDataPool(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsGetDataPool(const MMsgObsGetDataPool& theMsgR);

	// Destructor
	~MMsgObsGetDataPool(void);

	// Operator overloads

	// Assignment
	MMsgObsGetDataPool& operator=(const MMsgObsGetDataPool& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsGetDataPool(*this);
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

class MMsgObsGetDataPoolReply : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsGetDataPoolReply(void);

	// MiniMessage Constructor
	explicit MMsgObsGetDataPoolReply(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsGetDataPoolReply(const MMsgObsGetDataPoolReply& theMsgR);

	// Destructor
	~MMsgObsGetDataPoolReply(void);

	// Operator overloads

	// Assignment
	MMsgObsGetDataPoolReply& operator=(const MMsgObsGetDataPoolReply& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsGetDataPoolReply(*this);
	}

	// Member access methods
	ServerStatus
	GetStatus(void) const
	{
		return mStatus;
	}

	unsigned long
	GetDataPoolId(void) const
	{
		return mDataPoolId;
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

	void
	SetStatus(ServerStatus theStatus)
	{
		mStatus = theStatus;
	}

	void
	SetDataPoolId(const unsigned long theDataPoolId )
	{
		mDataPoolId = theDataPoolId;
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
	
private:

	ServerStatus	mStatus;		// Request status
	unsigned long	mDataPoolId;
	unsigned short	mDataLength;
	BYTE*			mDataP;
};


}

#endif