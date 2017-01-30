#ifndef _MMsgObsRemovePublication_H
#define _MMsgObsRemovePublication_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsRemovePublication
//
// The RemovePublication removes a publication from the list of publications published
// on the Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"

using namespace std;

namespace WONMsg {

class MMsgObsRemovePublication : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsRemovePublication(void);

	// MiniMessage Constructor
	explicit MMsgObsRemovePublication(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsRemovePublication(const MMsgObsRemovePublication& theMsgR);

	// Destructor
	~MMsgObsRemovePublication(void);

	// Operator overloads

	// Assignment
	MMsgObsRemovePublication& operator=(const MMsgObsRemovePublication& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsRemovePublication(*this);
	}

	// Member access methods
	unsigned long
	GetPublicationId(void) const
	{
		return mPublicationId;
	}

	void
	SetPublicationId(const unsigned long thePublicationId )
	{
		mPublicationId = thePublicationId;
	}
	
private:

	unsigned long	mPublicationId;
};

}

#endif