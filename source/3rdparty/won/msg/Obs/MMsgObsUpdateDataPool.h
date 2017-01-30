#ifndef _MMsgObsUpdateDataPool_H
#define _MMsgObsUpdateDataPool_H

///////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsUpdateDataPool
//
// The UpdateDataPool message updates the specified data pool.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsUpdateDataPool : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsUpdateDataPool(void);

	// MiniMessage Constructor
	explicit MMsgObsUpdateDataPool(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsUpdateDataPool(const MMsgObsUpdateDataPool& theMsgR);

	// Destructor
	~MMsgObsUpdateDataPool(void);

	// Operator overloads

	// Assignment
	MMsgObsUpdateDataPool& operator=(const MMsgObsUpdateDataPool& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsUpdateDataPool(*this);
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
