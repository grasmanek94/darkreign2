#ifndef _MMsgDataPoolUpdated_H
#define _MMsgDataPoolUpdated_H

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MMsgObsDataPoolUpdated
//
// The DataPoolUpdated message notifies subscribers to a data pool that it has been updated.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

using namespace std;

namespace WONMsg {

class MMsgObsDataPoolUpdated : public MiniMessage
{
public:

	// Constructors / Destructor
	MMsgObsDataPoolUpdated(void);

	// MiniMessage - will throw if MiniMessage type is not of this type
	explicit MMsgObsDataPoolUpdated(const MiniMessage& theMsgR);

	// Copy
	MMsgObsDataPoolUpdated(const MMsgObsDataPoolUpdated& theMsgR);

	~MMsgObsDataPoolUpdated(void);

	
	// Operator Overloads

	// Assignment operator overload
	MMsgObsDataPoolUpdated& operator=(const MMsgObsDataPoolUpdated& theMsgR);


	// Overridden methods

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);


	// Inlines
	TRawMsg* Duplicate(void) const
	{ 
		return new MMsgObsDataPoolUpdated(*this);
	}

	unsigned long GetDataPoolId(void) const
	{ 
		return mDataPoolId;
	}

	void SetDataPoolId(unsigned long theDataPoolId)
	{ 
		mDataPoolId = theDataPoolId;
	}

private:
	unsigned long	mDataPoolId;	// Unique Id of Data Pool updated
};

}

#endif
