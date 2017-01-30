#ifndef _MMsgObsSubscribeByName_H
#define _MMsgObsSubscribeByName_H

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsSubscribeByName
//
// The SubscribeByName message uses the PublicationName to subscribe the sending server/client
// to the specified publication.
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsSubscribeByName : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsSubscribeByName(void);

	// MiniMessage Constructor
	explicit MMsgObsSubscribeByName(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsSubscribeByName(const MMsgObsSubscribeByName& theMsgR);

	// Destructor
	~MMsgObsSubscribeByName(void);

	// Operator overloads

	// Assignment
	MMsgObsSubscribeByName& operator=(const MMsgObsSubscribeByName& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsSubscribeByName(*this);
	}

	// Member access methods
	const string&
	GetPublicationName(void) const
	{
		return mPublicationName;
	}

	unsigned long
	GetPublisherId(void) const
	{
		return mPublisherId;
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

	bool
	GetRequestPublish(void) const
	{
		return mRequestPublish;
	}

	void
	SetPublicationName(const string& thePublicationName)
	{
		mPublicationName = thePublicationName;
	}

	void
	SetPublisherId(const unsigned long thePublisherId)
	{
		mPublisherId = thePublisherId;
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

	void
	SetRequestPublish(const bool theRequestPublish)
	{
		mRequestPublish = theRequestPublish;
	}
	
private:

	string				mPublicationName;
	unsigned long		mPublisherId;
	unsigned long		mSubscriberId;
	string				mSubscriberName;
	wstring				mSubscriberDescription;
	ConnectionTypeEnum	mConnectionType;
	string				mSubscriberAddress;
	bool				mRequestPublish;			
};

}

#endif