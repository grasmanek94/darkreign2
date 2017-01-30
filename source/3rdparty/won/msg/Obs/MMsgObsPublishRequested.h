#ifndef _MMsgObsPublishRequested_H
#define _MMsgObsPublishRequested_H

///////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsPublishRequested
//
// The PublishRequested message is sent to a publisher to request that it publish a stream or data pool.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsPublishRequested : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsPublishRequested(void);

	// MiniMessage Constructor
	explicit MMsgObsPublishRequested(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsPublishRequested(const MMsgObsPublishRequested& theMsgR);

	// Destructor
	~MMsgObsPublishRequested(void);

	// Operator overloads

	// Assignment
	MMsgObsPublishRequested& operator=(const MMsgObsPublishRequested& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsPublishRequested(*this);
	}

	// Member access methods
	const string&
	GetPublicationName(void) const
	{
		return mPublicationName;
	}

	void
	SetPublicationName(const string& thePublicationName)
	{
		mPublicationName = thePublicationName;
	}
		
private:

	string				mPublicationName;
};

}

#endif
