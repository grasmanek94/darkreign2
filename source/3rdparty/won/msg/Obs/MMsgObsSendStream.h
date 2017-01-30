#ifndef _MMsgObsSendStream_H
#define _MMsgObsSendStream_H

///////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsSendStream
//
// The SendStream message updates the specified data pool.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsSendStream : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsSendStream(void);

	// MiniMessage Constructor
	explicit MMsgObsSendStream(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsSendStream(const MMsgObsSendStream& theMsgR);

	// Destructor
	~MMsgObsSendStream(void);

	// Operator overloads

	// Assignment
	MMsgObsSendStream& operator=(const MMsgObsSendStream& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsSendStream(*this);
	}

	// Member access methods
	unsigned long
	GetStreamId(void) const
	{
		return mStreamId;
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

	bool
	GetSendReply(void) const
	{
		return mSendReply;
	}

	void
	SetStreamId(const unsigned long theStreamId )
	{
		mStreamId = theStreamId;
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
	SetSendReply(const bool theSendReply )
	{
		mSendReply = theSendReply;
	}
	
private:

	unsigned long	mStreamId;
	unsigned short	mDataLength;
	BYTE*			mDataP;
	bool			mSendReply;
};

}

#endif
