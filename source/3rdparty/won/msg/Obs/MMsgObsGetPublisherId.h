#ifndef _MMsgObsGetPublisherId_H
#define _MMsgObsGetPublisherId_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetPublisherId
//
// The GetPublisherId message gets the unique identifier of the Publisher from the unique name.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"

using namespace std;

namespace WONMsg {

class MMsgObsGetPublisherId : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsGetPublisherId(void);

	// MiniMessage Constructor
	explicit MMsgObsGetPublisherId(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsGetPublisherId(const MMsgObsGetPublisherId& theMsgR);

	// Destructor
	~MMsgObsGetPublisherId(void);

	// Operator overloads

	// Assignment
	MMsgObsGetPublisherId& operator=(const MMsgObsGetPublisherId& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsGetPublisherId(*this);
	}

	// Member access methods
	const string&
	GetPublisherName(void) const
	{
		return mPublisherName;
	}

	void
	SetPublisherName(const string& thePublisherName )
	{
		mPublisherName = thePublisherName;
	}
	
private:

	string mPublisherName;
};

}

#endif