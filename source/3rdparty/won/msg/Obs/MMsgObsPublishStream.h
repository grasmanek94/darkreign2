#ifndef _MMsgObsPublishStream_H
#define _MMsgObsPublishStream_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsPublishStream
//
// The PublishStream message publishes the given stream on the Observation server
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"

using namespace std;

namespace WONMsg {

class MMsgObsPublishStream : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsPublishStream(void);

	// MiniMessage Constructor
	explicit MMsgObsPublishStream(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsPublishStream(const MMsgObsPublishStream& theMsgR);

	// Destructor
	~MMsgObsPublishStream(void);

	// Operator overloads

	// Assignment
	MMsgObsPublishStream& operator=(const MMsgObsPublishStream& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsPublishStream(*this);
	}

	// Member access methods
	unsigned long
	GetPublisherId(void) const
	{
		return mPublisherId;
	}

	const string&
	GetPublicationName(void) const
	{
		return mPublicationName;
	}

	const wstring&
	GetPublicationDescription(void) const
	{
		return mPublicationDescription;
	}

	bool
	GetSubscriptionNotification(void) const
	{
		return mSubscriptionNotification;
	}

	void
	SetPublisherId(const unsigned long thePublisherId )
	{
		mPublisherId = thePublisherId;
	}

	void
	SetPublicationName(const string& thePublicationName)
	{
		mPublicationName = thePublicationName;
	}

	void
	SetPublicationDescription(const wstring& thePublicationDescription)
	{
		mPublicationDescription = thePublicationDescription;
	}

	void
	SetSubscriptionNotification(const bool theSubscriptionNotification)
	{
		mSubscriptionNotification = theSubscriptionNotification;
	}
	
	
private:

	unsigned long	mPublisherId;
	string			mPublicationName;
	wstring			mPublicationDescription;
	bool			mSubscriptionNotification;
};

}

#endif