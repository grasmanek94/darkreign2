#ifndef _MMsgObsSubscribeById_H
#define _MMsgObsSubscribeById_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsSubscribeById
//
// The SubscribeById message uses the PublicationId to subscribe the sending server/client 
// to the specified publication.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsSubscribeById : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsSubscribeById(void);

	// MiniMessage Constructor
	explicit MMsgObsSubscribeById(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsSubscribeById(const MMsgObsSubscribeById& theMsgR);

	// Destructor
	~MMsgObsSubscribeById(void);

	// Operator overloads

	// Assignment
	MMsgObsSubscribeById& operator=(const MMsgObsSubscribeById& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsSubscribeById(*this);
	}

	// Member access methods
	unsigned long
	GetPublicationId(void) const
	{
		return mPublicationId;
	}

	unsigned long
	GetSubscriberId(void) const
	{
		return mSubscriberId;
	}

	const string&
	GetSubscriberName(void) const
	{
		return mSubscriberName;
	}

	const wstring&
	GetSubscriberDescription(void) const
	{
		return mSubscriberDescription;
	}

	ConnectionTypeEnum
	GetConnectionType( void ) const
	{
		return mConnectionType;
	}

	const string&
	GetSubscriberAddress(void) const
	{
		return mSubscriberAddress;
	}

	void
	SetPublicationId(const unsigned long thePublicationId)
	{
		mPublicationId = thePublicationId;
	}

	void
	SetSubscriberId(const unsigned long theSubscriberId)
	{
		mSubscriberId = theSubscriberId;
	}

	void
	SetSubscriberName(const string& theSubscriberName)
	{
		mSubscriberName = theSubscriberName;
	}

	void
	SetSubscriberDescription(const wstring& theSubscriberDescription)
	{
		mSubscriberDescription = theSubscriberDescription;
	}

	void
	SetConnectionType(ConnectionTypeEnum theConnectionType)
	{
		mConnectionType = theConnectionType;
	}

	void
	SetSubscriberAddress(const string& theSubscriberAddress)
	{
		mSubscriberAddress = theSubscriberAddress;
	}
	
private:

	unsigned long		mPublicationId;
	unsigned long		mSubscriberId;
	string				mSubscriberName;
	wstring				mSubscriberDescription;
	ConnectionTypeEnum	mConnectionType;
	string				mSubscriberAddress;
};

}

#endif