#ifndef _MMsgObsGetPublicationId_H
#define _MMsgObsGetPublicationId_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetPublicationId
//
// The GetPublicationId message gets the unique identifier of the Publication from the unique name.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"

using namespace std;

namespace WONMsg {

class MMsgObsGetPublicationId : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsGetPublicationId(void);

	// MiniMessage Constructor
	explicit MMsgObsGetPublicationId(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsGetPublicationId(const MMsgObsGetPublicationId& theMsgR);

	// Destructor
	~MMsgObsGetPublicationId(void);

	// Operator overloads

	// Assignment
	MMsgObsGetPublicationId& operator=(const MMsgObsGetPublicationId& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsGetPublicationId(*this);
	}

	// Member access methods
	const string&
	GetPublicationName(void) const
	{
		return mPublicationName;
	}

	const string&
	GetPublisherName(void) const
	{
		return mPublisherName;
	}

	void
	SetPublicationName(const string& thePublicationName)
	{
		mPublicationName = thePublicationName;
	}

	void
	SetPublisherName(const string& thePublisherName)
	{
		mPublisherName = thePublisherName;
	}
	
private:

	string mPublicationName;
	string mPublisherName;
};

}

#endif