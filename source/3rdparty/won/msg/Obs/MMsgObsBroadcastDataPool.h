#ifndef _MMsgObsBroadcastDataPool_H
#define _MMsgObsBroadcastDataPool_H

///////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsBroadcastDataPool
//
// The BroadcastDataPool message sends the contents of a data pool to a subscriber.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsBroadcastDataPool : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsBroadcastDataPool(void);

	// MiniMessage Constructor
	explicit MMsgObsBroadcastDataPool(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsBroadcastDataPool(const MMsgObsBroadcastDataPool& theMsgR);

	// Destructor
	~MMsgObsBroadcastDataPool(void);

	// Operator overloads

	// Assignment
	MMsgObsBroadcastDataPool& operator=(const MMsgObsBroadcastDataPool& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsBroadcastDataPool(*this);
	}

	// Member access methods
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

	unsigned long	mDataPoolId;
	unsigned short	mDataLength;
	BYTE*			mDataP;
};

}

#endif
