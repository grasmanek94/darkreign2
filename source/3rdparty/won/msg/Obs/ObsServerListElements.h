#ifndef _OBSSERVERLISTELEMENTS_H
#define _OBSSERVERLISTELEMENTS_H

//////////////////////////////////////////////////////////////////////
// ObsServerListElements.h
//////////////////////////////////////////////////////////////////////

#include <string>
#include <list>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Global Enums
//////////////////////////////////////////////////////////////////////

enum ConnectionTypeEnum 
{
	PersistentTCP, TransientTCP, UDP
};

enum PublicationTypeEnum
{
	DataPool, Stream
};

enum ResourceTypeEnum
{
	ResourcePublisher, ResourceSubscriber, ResourcePublication
};

//////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////

class Publisher;
class Publication;
class Subscriber;
class Subscription;

typedef list<Publisher> PublisherList;
typedef list<Publication> PublicationList;
typedef list<Subscriber> SubscriberList;
typedef list<Subscription> SubscriptionList;

//////////////////////////////////////////////////////////////////////
// ListElement
//////////////////////////////////////////////////////////////////////

class ListElement
{

// Methods
public:

	// Constructors/Destructors
	ListElement() :
	  mId(0),
	  mName()
	{
	}

	ListElement(const ListElement& theListElement)  :
		mId(theListElement.mId),
		mName(theListElement.mName)
	{
	}

	virtual ~ListElement()
	{
	}


	// Operator Overloads
	ListElement& operator=(const ListElement& theListElement)
	{
		mId = theListElement.mId;
		mName = theListElement.mName;
		return *this;
	}

	bool operator==(const ListElement& theListElementR) const
	{
		return ( mId			== theListElementR.mId &&
				 mName			== theListElementR.mName );
	}

	bool operator<(const ListElement& theListElement) const
	{
		return ( mId < theListElement.mId );
	}
	

// Member data
public:

	//bool NameUnique;

	// Keys that data can be retreived by
	unsigned long	mId;			// Unique id of element
	string			mName;			// Name of element
};

///////////////////////////////////////////////////////////////////////
// Output function

inline ostream&
operator<<(ostream& os, const ListElement& theListElement )
{
		os << theListElement.mId << theListElement.mName;
		return os;
}

////////////////////////////////////////////////////////////////////////////////////////
// Publisher
// Contains all of the information about a publisher
////////////////////////////////////////////////////////////////////////////////////////
class Publisher : public ListElement
{

public:
	
	Publisher() :
	  ListElement(),
	  mPublisherDescription(),
	  mConnectionType(PersistentTCP),
	  mNetworkAddress(),
	  mAccessPubsByName(),
	  mOnDemandPublishing(false)
	{
	}

	Publisher( const Publisher& thePublisherR ) :
		ListElement(thePublisherR),
		mPublisherDescription(thePublisherR.mPublisherDescription),
		mConnectionType(thePublisherR.mConnectionType),
		mNetworkAddress(thePublisherR.mNetworkAddress),
		mAccessPubsByName(thePublisherR.mAccessPubsByName),
		mOnDemandPublishing(thePublisherR.mOnDemandPublishing)
	{
	}

	virtual ~Publisher()
	{
	}

	// Operator overloads 
	Publisher& operator=(const Publisher& thePublisherR)
	{
		ListElement::operator=(thePublisherR);

		mPublisherDescription	= thePublisherR.mPublisherDescription;	
		mConnectionType			= thePublisherR.mConnectionType;		
		mNetworkAddress			= thePublisherR.mNetworkAddress;
		mAccessPubsByName		= thePublisherR.mAccessPubsByName;
		mOnDemandPublishing		= thePublisherR.mOnDemandPublishing;
		return *this;
	}

	bool operator<(const Publisher& thePublisherR) const
	{
		return ( mId < thePublisherR.mId );
	}
	
// Member data
public:
	wstring				mPublisherDescription;	// Description of publisher
	ConnectionTypeEnum	mConnectionType;		// Type of connection
	string				mNetworkAddress;		// Network address of Publisher <IP_string>:<Port>
	bool				mAccessPubsByName;		// Access to publications by name is supported by this publisher.
	bool				mOnDemandPublishing;	// Determines whether this publisher supports ondemand publishing 
};

///////////////////////////////////////////////////////////////////////
// Output function
inline ostream&
operator<<(ostream& os, const Publisher& thePublisherR )
{
	os << (ListElement)thePublisherR;
	os << thePublisherR.mPublisherDescription;
	os << (int)thePublisherR.mConnectionType;
	os << thePublisherR.mNetworkAddress;
	os << thePublisherR.mAccessPubsByName;
	os << thePublisherR.mOnDemandPublishing;
	return os;
}

////////////////////////////////////////////////////////////////////////////////////////
// Publication
// Contains all of the information about a Publication
////////////////////////////////////////////////////////////////////////////////////////
class Publication : public ListElement
{

public:
	
	Publication() :
	  ListElement(),
	  mPublisherId(0),
	  mPublicationDescription(),
	  mPublicationType(DataPool),
	  mSubscriptionNotification(false),
	  mDataLength(0),
	  mDataP(NULL)
	{
	}

	Publication( const Publication& thePublicationR ) :
		ListElement(thePublicationR),
		mPublisherId(thePublicationR.mPublisherId),
		mPublicationDescription(thePublicationR.mPublicationDescription),
		mPublicationType(thePublicationR.mPublicationType),
		mSubscriptionNotification(thePublicationR.mSubscriptionNotification),
		mDataLength(0),
		mDataP(NULL)
	{
		SetData(thePublicationR.mDataP, thePublicationR.mDataLength);
	}

	virtual ~Publication()
	{
		delete [] mDataP;
	}

	// Operator overloads 
	Publication& operator=(const Publication& thePublicationR)
	{
		ListElement::operator=(thePublicationR);

		mPublisherId				= thePublicationR.mPublisherId;	
		mPublicationDescription		= thePublicationR.mPublicationDescription;		
		mPublicationType			= thePublicationR.mPublicationType;
		mSubscriptionNotification	= thePublicationR.mSubscriptionNotification;
		SetData(thePublicationR.mDataP, thePublicationR.mDataLength);
		return *this;
	}

	bool operator<(const Publication& thePublicationR) const
	{
		return ( mId < thePublicationR.mId );
	}

	// Inline functions
	// SetData
	void SetData(const BYTE* theDataP, unsigned short theDataLength)
	{
		if ( theDataP != mDataP ) // don't want to copy onto ourselves
		{
			// Make sure we delete any old data
			delete [] mDataP;  
		
			mDataLength = theDataLength;
			if (mDataLength > 0)
			{
				mDataP = new BYTE [mDataLength];
				memcpy(mDataP, theDataP, theDataLength);
			}
			else
			{
				mDataP = NULL;
			}
		}
	} 
	
// Member data
public:

	unsigned long			mPublisherId;				// Id of publisher
	wstring					mPublicationDescription;	// Description of Publication
	PublicationTypeEnum		mPublicationType;			// Type of publication
	bool					mSubscriptionNotification;	// Determines if publisher notified of subscription
	unsigned short			mDataLength;				// Length of Binary data
	BYTE*					mDataP;						// Binary 
};


///////////////////////////////////////////////////////////////////
// Output function
inline ostream&
operator<<(ostream& os, const Publication& thePublicationR )
{
	os << (ListElement)thePublicationR;
	os << thePublicationR.mPublisherId;
	os << thePublicationR.mPublicationDescription;
	os << (int)thePublicationR.mPublicationType;
	os << thePublicationR.mSubscriptionNotification;
	os << thePublicationR.mDataLength;
	for (int i = 0; i < thePublicationR.mDataLength; i++)
	{
		os << thePublicationR.mDataP[i];
	}
	return os;
}

////////////////////////////////////////////////////////////////////////////////////////
// Subscriber
// Contains all of the information about a Subscriber
////////////////////////////////////////////////////////////////////////////////////////
class Subscriber : public ListElement
{

public:
	
	Subscriber() :
	  ListElement(),
	  mSubscriberDescription(),
	  mConnectionType(PersistentTCP),
	  mNetworkAddress()
	{
	}

	Subscriber( const Subscriber& theSubscriberR ) :
		ListElement(theSubscriberR),
		mSubscriberDescription(theSubscriberR.mSubscriberDescription),
		mConnectionType(theSubscriberR.mConnectionType),
		mNetworkAddress(theSubscriberR.mNetworkAddress)
	{
	}

	virtual ~Subscriber()
	{
	}

	// Operator overloads 
	Subscriber& operator=(const Subscriber& theSubscriberR)
	{
		ListElement::operator=(theSubscriberR);

		mSubscriberDescription	= theSubscriberR.mSubscriberDescription;	
		mConnectionType			= theSubscriberR.mConnectionType;		
		mNetworkAddress			= theSubscriberR.mNetworkAddress;
		return *this;
	}

	bool operator<(const Subscriber& theSubscriberR) const
	{
		return ( mId < theSubscriberR.mId );
	}
	
// Member data
public:
	wstring				mSubscriberDescription;		// Description of Subscriber
	ConnectionTypeEnum	mConnectionType;			// Type of Connection
	string				mNetworkAddress;			// Network address
};

///////////////////////////////////////////////////////////////////////
// Output function
inline ostream&
operator<<(ostream& os, const Subscriber& theSubscriberR )
{
	os << (ListElement)theSubscriberR;
	os << theSubscriberR.mSubscriberDescription;
	os << (int)theSubscriberR.mConnectionType;
	os << theSubscriberR.mNetworkAddress;
	return os;
}

//////////////////////////////////////////////////////////////////////
// Subscription
//////////////////////////////////////////////////////////////////////

class Subscription
{

// Methods
public:

	// Constructors/Destructors
	Subscription() :
	  mPublicationId(0),
	  mSubscriberId(0)

	{
	}

	Subscription(const Subscription& theSubscription)  :
		mPublicationId(theSubscription.mPublicationId),
		mSubscriberId(theSubscription.mSubscriberId)
	{
	}

	virtual ~Subscription()
	{
	}


	// Operator Overloads
	Subscription& operator=(const Subscription& theSubscription)
	{
		mPublicationId = theSubscription.mPublicationId;
		mSubscriberId = theSubscription.mSubscriberId;
		return *this;
	}

	bool operator==(const Subscription& theSubscriptionR) const
	{
		return ( mPublicationId == theSubscriptionR.mPublicationId &&
				 mSubscriberId == theSubscriptionR.mSubscriberId);
	}

	bool operator<(const Subscription& theSubscription) const
	{

		return ( mPublicationId == theSubscription.mPublicationId ?
				 mSubscriberId < theSubscription.mSubscriberId : 
				 mPublicationId < theSubscription.mPublicationId );
	}
	

// Member data
public:

	unsigned long	mSubscriberId;			// Id of Subscriber
	unsigned long	mPublicationId;			// Id of Publication
};

///////////////////////////////////////////////////////////////////////
// Output function

inline ostream&
operator<<(ostream& os, const Subscription& theSubscription )
{
		os << theSubscription.mSubscriberId;
		os << theSubscription.mPublicationId;
		return os;
}

#endif