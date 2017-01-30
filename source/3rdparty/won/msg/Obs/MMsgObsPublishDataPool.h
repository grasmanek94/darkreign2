#ifndef _MMsgObsPublishDataPool_H
#define _MMsgObsPublishDataPool_H

///////////////////////////////////////////////////////////////////////////////////////////////// 
// MMsgObsPublishDataPool
//
// The PublishDataPool message publishes the given data pool on the Observation server
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "msg/TMessage.h"
#include "ObsServerListElements.h"

using namespace std;

namespace WONMsg {

class MMsgObsPublishDataPool : public MiniMessage
{
public:
	// Constructors/Destructors

	// Default Constructor
	MMsgObsPublishDataPool(void);

	// MiniMessage Constructor
	explicit MMsgObsPublishDataPool(const MiniMessage& theMsgR);

	// Copy Constructor
	MMsgObsPublishDataPool(const MMsgObsPublishDataPool& theMsgR);

	// Destructor
	~MMsgObsPublishDataPool(void);

	// Operator overloads

	// Assignment
	MMsgObsPublishDataPool& operator=(const MMsgObsPublishDataPool& theMsgR);

	// Virtual overrides

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Inlines

	TRawMsg*
	Duplicate(void) const
	{
		return new MMsgObsPublishDataPool(*this);
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

	unsigned long	mPublisherId;
	string			mPublicationName;
	wstring			mPublicationDescription;
	bool			mSubscriptionNotification;
	unsigned short	mDataLength;
	BYTE*			mDataP;
};

}

#endif
