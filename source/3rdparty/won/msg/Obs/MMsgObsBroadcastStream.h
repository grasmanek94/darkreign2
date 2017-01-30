#ifndef _MMsgObsBroadcastStream_H
#define _MMsgObsBroadcastStream_H

///////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsBroadcastStream
//
// The BroadcastStream message sends the contents of a stream to a subscriber.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsBroadcastStream : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsBroadcastStream(void);

	// MiniMessage Constructor
	explicit MMsgObsBroadcastStream(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsBroadcastStream(const MMsgObsBroadcastStream& theMsgR);

	// Destructor
	~MMsgObsBroadcastStream(void);

	// Operator overloads

	// Assignment
	MMsgObsBroadcastStream& operator=(const MMsgObsBroadcastStream& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsBroadcastStream(*this);
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
	
private:

	unsigned long	mStreamId;
	unsigned short	mDataLength;
	BYTE*			mDataP;
};

}

#endif
