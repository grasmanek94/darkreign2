#ifndef _MMsgObsReestablishPersistentTCP_H
#define _MMsgObsReestablishPersistentTCP_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsReestablishPersistentTCP
//
// The ReestablishPersistentTCP message reestablishes a persistent TCP connection to the Observation server
//
///////////////////////////////////////////////////////////////////////////////////////////

#include "msg/TMessage.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsReestablishPersistentTCP : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsReestablishPersistentTCP(void);

	// MiniMessage Constructor
	explicit MMsgObsReestablishPersistentTCP(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsReestablishPersistentTCP(const MMsgObsReestablishPersistentTCP& theMsgR);

	// Destructor
	~MMsgObsReestablishPersistentTCP(void);

	// Operator overloads

	// Assignment
	MMsgObsReestablishPersistentTCP& operator=(const MMsgObsReestablishPersistentTCP& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsReestablishPersistentTCP(*this);
	}

	// Member access methods
	unsigned long
	GetClientId(void) const
	{
		return mClientId;
	}

	ResourceTypeEnum
	GetClientType(void) const
	{
		return mClientType;
	}

	void
	SetClientId(const unsigned long theResourceId)
	{
		mClientId = theResourceId;
	}

	void
	SetClientType(const ResourceTypeEnum theClientType)
	{
		mClientType = theClientType;
	}
	
private:

	unsigned long		mClientId;
	ResourceTypeEnum	mClientType;
};

};

#endif