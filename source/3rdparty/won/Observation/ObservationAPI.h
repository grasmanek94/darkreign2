#ifndef _ObservationAPI_H
#define _ObservationAPI_H

///////////////////////////////////////////////////////////////////////////////////
//
// Observation server Client API
//
// Contains classes for Observation Publisher and Subscriber clients
//
///////////////////////////////////////////////////////////////////////////////////

#include "Socket/WONWS.h" // This needs to be put in for now so that we will not get windows winsock.h
#include "Socket/TMsgSocket.h"
#include "Socket/IPSocket.h"
#include "Socket/TCPSocket.h"
#include "Socket/UDPSocket.h"
#include "msg/ServerStatus.h"
#include "ObservationMessages.h"
#include "SocketMgr.h"

#include <assert.h>
#include <list>

using namespace WONMsg;
using namespace std;

namespace WONAPI {

///////////////////////////////////////////////////////////////////////////////////
// Observation Client Base class
//
// This is an abstract base class.  Instantiate a Publisher or Subscriber Client object
///////////////////////////////////////////////////////////////////////////////////

class ObservationClientBase
{
public:
	// Enums
	enum ClientTypeEnum
	{
		ClientPublisher, 
		ClientSubscriber, 
		ClientBoth			// This type added for future use
	};

	// Internal structures for returns from server communication
	struct PublicationData
	{
		unsigned long	mDataId;
		unsigned short	mDataLength;
		BYTE*			mDataP;
	};

	struct IdReply
	{
		ServerStatus  mStatus;
		unsigned long mId;
	};

	struct NameReply
	{
		ServerStatus	mStatus;		// Request status
		string			mName;			// Name of publisher, publication, or subscriber
		wstring			mDescription;	// Description of publisher, publication, or subscriber
	};

	struct NumSubscribersReply
	{
		ServerStatus	mStatus;				// Request status
		unsigned long	mNumSubscribers;		// Number of subscribers
	};

	struct PublisherListReply 
	{
		ServerStatus mStatus;			// Request status
		PublisherList mPublisherList;	// List of publishers
	};

	struct SubscriberListReply 
	{
		ServerStatus mStatus;			// Request status
		SubscriberList mSubscriberList; // List of subscribers
	};

	struct PublicationListReply 
	{
		ServerStatus mStatus;				// Request status
		PublicationList mPublicationList;	// List of Publications
	};

	struct SubscriptionListReply 
	{
		ServerStatus mStatus;				// Request status
		SubscriptionList mSubscriptionList; // List of Subscriptions
	};

	struct ReplyCompletionData
	{
		ReplyCompletionData() :
			mReplyId(0),
			mCompletionP(NULL),
			mReplyResultP(NULL)
		{};

		unsigned long				mReplyId;		// Id of the reply
		CompletionContainerBase*	mCompletionP;	// The completion to call when finished
		void *						mReplyResultP;	// Pointer to put the data from the reply
	};

	typedef list<ReplyCompletionData> ReplyCompletionList;


// Public methods
public:
	ObservationClientBase(ConnectionTypeEnum theConnectionType = TransientTCP);
	virtual ~ObservationClientBase();

	virtual ClientTypeEnum GetClientType() = 0;

	// Public utilities
	//static string GetLocalAddress();

	//	Server communication functions
	// The last parameter is to pass in object that will be filled in with the reply information when the reply comes back.
	Error GetPublisherId(const string& thePublisherNameR, const CompletionContainer<const IdReply&>& theCompletion = DEFAULT_COMPLETION, const IdReply* theIdReplyP = NULL);
	Error GetPublicationId(const string& thePublicationNameR, const string& thePublisherNameR, const CompletionContainer<const IdReply&>& theCompletion = DEFAULT_COMPLETION, const IdReply* theIdReplyP = NULL);
	Error GetName(unsigned long theResourceId, ResourceTypeEnum mResourceType, const CompletionContainer<const NameReply&>& theCompletion = DEFAULT_COMPLETION, const NameReply* theNameReplyP = NULL);
	Error GetNumSubscribers(unsigned long thePublicationId, const CompletionContainer<const NumSubscribersReply&>& theCompletion = DEFAULT_COMPLETION, const NumSubscribersReply* theNumSubscribersReplyP = NULL);
	Error EnumeratePublications(unsigned long thePublisherId = 0, const CompletionContainer<const PublicationListReply&>& theCompletion = DEFAULT_COMPLETION, const PublicationListReply* thePublicationListReplyP = NULL);
	Error EnumeratePublishers(const CompletionContainer<const PublisherListReply&>& theCompletion = DEFAULT_COMPLETION, const PublisherListReply* thePublisherListReplyP = NULL);
	Error EnumerateSubscribers(unsigned long thePublisherId = 0, unsigned long thePublicationId = 0, const CompletionContainer<const SubscriberListReply&>& theCompletion = DEFAULT_COMPLETION, const SubscriberListReply* theSubscriberListReplyP = NULL);
	Error EnumerateSubscriptions(unsigned long thePublisherId = 0, unsigned long thePublicationId = 0, const CompletionContainer<const SubscriptionListReply&>& theCompletion = DEFAULT_COMPLETION, const SubscriptionListReply* theSubscriptionListReplyP = NULL);
	
	// Templated functions that send messages to server
	template <class privsType>
	Error GetPublisherId( const string& thePublisherNameR, void (*f)(const IdReply&, privsType), privsType t )
	{ return GetPublisherId(thePublisherNameR, new CompletionWithContents<const IdReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error GetPublicationId( const string& thePublicationNameR, void (*f)(const IdReply&, privsType), privsType t )
	{ return GetPublicationId(thePublicationNameR, new CompletionWithContents<const IdReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error GetName( unsigned long theResourceId, ResourceTypeEnum mResourceType, void (*f)(const NameReply&, privsType), privsType t )
	{ return GetName( theResourceId, mResourceType, new CompletionWithContents<const NameReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error GetNumSubscribers( unsigned long thePublicationId, void (*f)(const NumSubscribersReply&, privsType), privsType t )
	{ return GetNumSubscribers(thePublicationId, new CompletionWithContents<const NumSubscribersReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error EnumeratePublications( unsigned long thePublisherId, void (*f)(const PublicationListReply&, privsType), privsType t )
	{ return EnumeratePublications(thePublisherId, new CompletionWithContents<const PublicationListReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error EnumeratePublishers(  void (*f)(const PublisherListReply&, privsType), privsType t )
	{ return EnumeratePublishers( new CompletionWithContents<const PublisherListReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error EnumerateSubscribers( unsigned long thePublisherId, unsigned long thePublicationId, void (*f)(const SubscriberListReply&, privsType), privsType t )
	{ return EnumerateSubscribers( thePublisherId, thePublicationId, new CompletionWithContents<const SubscriberListReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error EnumerateSubscriptions( unsigned long thePublisherId, unsigned long thePublicationId, void (*f)(const SubscriptionListReply&, privsType), privsType t )
	{ return EnumerateSubscriptions(thePublisherId, thePublicationId, new CompletionWithContents<const SubscriptionListReply&, privsType>(t, f, true)); }

	// Callbacks 
	static void ReceiveLoop(const TMsgSocket::RecvBaseMsgResult& result, ObservationClientBase* This);

	// Inlines
	virtual unsigned long GetClientId()
	{
		return mClientId;
	}

// Protected methods
protected:
	Error ClientStartup(const IPSocket::Address& theServerAddress, const unsigned short theLocalMonitorPort, const string& theClientNameR, const wstring& theClientDescR);
	void ClientShutdown();

	// Utility methods
	Error RegisterWithObsSrv(const IPSocket::Address& theServerAddress, const bool theAccessByName);
	const string CreateAddressString(const string& theAddressR, unsigned int thePort) const;
	Error SendMMsgToServer(BaseMessage& theMsgR, const CompletionContainerBase* theCompletionP = NULL, const void* theReplyResultP = NULL);
	bool FillMsgFromResult(BaseMessage& theMsg, const TMsgSocket::RecvBaseMsgResult& result);
	unsigned long GetReplyMsgType(unsigned long theMsgType);
	ReplyCompletionData GetFirstCompletion(unsigned long theReplyId);

// Private methods
private:
	// Message Reply Handlers
	virtual void HandleStatusReply(const MMsgObsStatusReply& theReplyMsg);
	virtual void HandleAddPublisherReply(const MMsgObsAddPublisherReply& theReplyMsg) {assert(0);}
	virtual void HandlePublishReply(const MMsgObsPublishReply& theReplyMsg) {assert(0);}
	virtual void HandleSubscribeReply(const MMsgObsSubscribeReply& theReplyMsg) {assert(0);}
	virtual void HandleGetIdReply(const MMsgObsGetIdReply& theReplyMsg);
	virtual void HandleGetNameReply(const MMsgObsGetNameReply& theReplyMsg);
	virtual void HandleGetNumSubscribersReply(const MMsgObsGetNumSubscribersReply& theReplyMsg);
	virtual void HandleGetDataPoolReply(const MMsgObsGetDataPoolReply& theReplyMsg) {assert(0);}
	virtual void HandleGetDataPoolSizeReply(const MMsgObsGetDataPoolSizeReply& theReplyMsg) {assert(0);}
	virtual void HandleEnumeratePublishersReply(const MMsgObsEnumeratePublishersReply& theReplyMsg);
	virtual void HandleEnumerateSubscribersReply(const MMsgObsEnumerateSubscribersReply& theReplyMsg);
	virtual void HandleEnumeratePublicationsReply(const MMsgObsEnumeratePublicationsReply& theReplyMsg);
	virtual void HandleEnumerateSubscriptionsReply(const MMsgObsEnumerateSubscriptionsReply& theReplyMsg);
	virtual void HandleAddSubscriberReply(const MMsgObsAddSubscriberReply& theReplyMsg) {assert(0);}

	// Notification message handlers
	virtual void HandleDataPoolUpdated(const MMsgObsDataPoolUpdated& theNotifyMsg) {assert(0);}
	virtual void HandleBroadcastDataPool(const MMsgObsBroadcastDataPool& theNotifyMsg){assert(0);}
	virtual void HandleBroadcastStream(const MMsgObsBroadcastStream& theNotifyMsg){assert(0);}
	virtual void HandlePublishRequested(const MMsgObsPublishRequested& theNotifyMsg){assert(0);}
	virtual void HandleSubscriptionCanceled(const MMsgObsSubscriptionCanceled& theNotifyMsg){assert(0);}
	virtual void HandlePublicationSubscribedTo(const MMsgObsPublicationSubscribedTo& theNotifyMsg){assert(0);}

// Member Data
protected:
	unsigned long		mClientId;
	string				mClientName;
	wstring				mClientDesc;
	ConnectionTypeEnum	mConnectionType;

	// Socket stuff
	IPSocket::Address	mObsServerAddress;
	IPSocket::Address	mLocalAddress;

	SocketMgr*			mSocketMgrP;

	// Reply Completion container list
	// This list is a list of pairs.  The return message type and the completion container
	ReplyCompletionList mReplyCompletionList;

	WONCommon::CriticalSection		mCriticalSection;
};

///////////////////////////////////////////////////////////////////////////////////
// Observation Publisher Client
///////////////////////////////////////////////////////////////////////////////////

class ObservationPublisherClient : public ObservationClientBase
{
public:
	// Internal structures for returns from server communication
	struct PublicationSubscribedTo
	{
		unsigned long	mPublicationId;
		unsigned long	mSubscriberId;
		string			mSubscriberName;
	};


// Public methods
public:
	ObservationPublisherClient(ConnectionTypeEnum theConnectionType = PersistentTCP);
	virtual ~ObservationPublisherClient();

	virtual ClientTypeEnum GetClientType()
	{
		return ClientPublisher;
	}

	//	Server communication functions
	// The last parameter is to pass in object that will be filled in with the reply information when the reply comes back.
	Error PublisherStartup( const IPSocket::Address& theServerAddress, const unsigned short theLocalMonitorPort, const string& theClientNameR, const wstring& theClientDescR, const bool theAccessPubsByName, const CompletionContainer<const ServerStatus&>& theCompletion = DEFAULT_COMPLETION, const ServerStatus* theServerStatusP = NULL);
	Error PublisherShutdown( const CompletionContainer<const ServerStatus&>& theCompletion = DEFAULT_COMPLETION, const ServerStatus* theServerStatusP = NULL); 
	Error PublishDataPool(const string& theNameR, const wstring& theDescriptionR, const bool theSubscriptionNotification, const BYTE* theDataP, const unsigned short theDataLength, const CompletionContainer<const IdReply&>& theCompletion = DEFAULT_COMPLETION, const IdReply* theIdReplyP = NULL);
	Error PublishStream(const string& theNameR, const wstring& theDescriptionR, const bool theSubscriptionNotification, const CompletionContainer<const IdReply&>& theCompletion = DEFAULT_COMPLETION, const IdReply* theIdReplyP = NULL);
	Error UpdateDataPool(unsigned long theDataPoolId, const BYTE* theDataP, const unsigned short theDataLength, const CompletionContainer<const ServerStatus&>& theCompletion = DEFAULT_COMPLETION, const ServerStatus* theServerStatusP = NULL);
	Error SendStream(unsigned long theStreamId, const BYTE* theDataP, const unsigned short theDataLength, const bool theSendReply, const CompletionContainer<const ServerStatus&>& theCompletion = DEFAULT_COMPLETION, const ServerStatus* theServerStatusP = NULL);
	Error RemovePublication(unsigned long thePublicationId, const CompletionContainer<const ServerStatus&>& theCompletion = DEFAULT_COMPLETION, const ServerStatus* theServerStatusP = NULL);

	// Templated functions that send messages to server
	template <class privsType>
	Error PublisherStartup( const IPSocket::Address& theServerAddress, const unsigned short theLocalMonitorPort, const string& theClientNameR, const wstring& theClientDescR, void (*f)(const ServerStatus&, privsType), privsType t )
	{ return PublisherStartup(theServerAddress, theLocalMonitorPort, theClientNameR, theClientDescR, new CompletionWithContents<const ServerStatus&, privsType>(t, f, true)); }

	template <class privsType>
	Error PublisherShutdown( void (*f)(const IdReply&, privsType), privsType t )
	{ return PublisherShutdown( new CompletionWithContents<const ServerStatus&, privsType>(t, f, true)); }

	template <class privsType>
	Error PublishDataPool(const string& theNameR, const wstring& theDescriptionR, const bool theSubscriptionNotification, const BYTE* theDataP, const unsigned short theDataLength, void (*f)(const IdReply&, privsType), privsType t)
	{ return PublishDataPool(theNameR, theDescriptionR, theSubscriptionNotification, theDataP, theDataLength, new CompletionWithContents<const IdReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error PublishStream(const string& theNameR, const wstring& theDescriptionR, const bool theSubscriptionNotification, void (*f)(const IdReply&, privsType), privsType t)
	{ return PublishStream(theNameR, theDescriptionR, theSubscriptionNotification, new CompletionWithContents<const IdReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error UpdateDataPool(unsigned long theDataPoolId, const BYTE* theDataP, const unsigned short theDataLength, void (*f)(const ServerStatus&, privsType), privsType t)
	{ return UpdateDataPool(theDataPoolId, theDataP, theDataLength, new CompletionWithContents<const ServerStatus&, privsType>(t, f, true)); }

	template <class privsType>
	Error SendStream(unsigned long theStreamId, const BYTE* theDataP, const unsigned short theDataLength, const bool theSendReply, void (*f)(const ServerStatus&, privsType), privsType t)
	{ return SendStream(theStreamId, theDataP, theDataLength, theSendReply,	new CompletionWithContents<const ServerStatus&, privsType>(t, f, true)); }

	template <class privsType>
	Error RemovePublication(unsigned long thePublicationId, void (*f)(const ServerStatus&, privsType), privsType t)
	{ return RemovePublication(thePublicationId, new CompletionWithContents<const ServerStatus&, privsType>(t, f, true)); }

	// Allow user to pass in completion and data pointer to be filled in when opertatio completes
	Error PublisherStartup( const IPSocket::Address& theServerAddress, const unsigned short theLocalMonitorPort, const string& theClientNameR, const wstring& theClientDescR, const ServerStatus* theResultP, const CompletionContainer<const ServerStatus&>& theCompletion = DEFAULT_COMPLETION );
	
	// Set Notification message completions
	void SetPublishRequestedCompletion( const CompletionContainer<const string&>& theCompletionContainer, string* NotifyDataP = NULL )
	{
		mPublishRequestedCompletion = theCompletionContainer;
		mPublishRequestedCompletion.OwnCompletion();
		mPublishRequestedDataP = NotifyDataP;
	}
	void SetPublicationSubscribedToCompletion( const CompletionContainer<const PublicationSubscribedTo&>& theCompletionContainer, PublicationSubscribedTo* NotifyDataP = NULL )
	{
		mPublicationSubscribedToCompletion = theCompletionContainer;
		mPublicationSubscribedToCompletion.OwnCompletion();
		mPublicationSubscribedToNotifyDataP = NotifyDataP;
	}

// Private Methods
private:
	// Message Reply Handlers
	virtual void HandleAddPublisherReply(const MMsgObsAddPublisherReply& theReplyMsg);
	virtual void HandlePublishReply(const MMsgObsPublishReply& theReplyMsg);
	
	// Notification message handlers
	virtual void HandlePublishRequested(const MMsgObsPublishRequested& theNotifyMsg);
	virtual void HandlePublicationSubscribedTo(const MMsgObsPublicationSubscribedTo& theNotifyMsg);

// Member Data
private:
	// Notification message Compleation Containers
	CompletionContainer<const string&>					mPublishRequestedCompletion;
	CompletionContainer<const PublicationSubscribedTo&> mPublicationSubscribedToCompletion;

	// Notification message Data Pointers
	string*						mPublishRequestedDataP;
	PublicationSubscribedTo*	mPublicationSubscribedToNotifyDataP;
};

///////////////////////////////////////////////////////////////////////////////////
// Observation Subscriber Client
///////////////////////////////////////////////////////////////////////////////////

class ObservationSubscriberClient : public ObservationClientBase
{
public:
	// Internal structures for returns from server communication
	struct SubscribeReply
	{
		ServerStatus	mStatus;			// Request status
		unsigned long	mPublicationId;		// Id of Publication subscribed to
		unsigned long	mSubscriberId;		// Id of subscriber added
		unsigned short  mDataLength;		// Length of data pool to get
		BYTE*			mDataP;				// Data pool data
		string			mPeerRedirection;	// Address of peer to redirect to
	};

	struct DataPoolReply
	{
		ServerStatus	mStatus;		// Request status
		unsigned long	mDataPoolId;
		unsigned short	mDataLength;
		BYTE*			mDataP;
	};

	struct DataPoolSizeReply
	{
		ServerStatus	mStatus;			// Request status
		unsigned long	mDataPoolSize;		// Size of Data Pool
	};


// Public methods
public:
	ObservationSubscriberClient(ConnectionTypeEnum theConnectionType = TransientTCP);
	virtual ~ObservationSubscriberClient();

	virtual ClientTypeEnum GetClientType()
	{
		return ClientSubscriber;
	}

	// Server communication methods
	// The last parameter is to pass in object that will be filled in with the reply information when the reply comes back.
	Error SubscriberStartup( const IPSocket::Address& theServerAddress, const unsigned short theLocalMonitorPort, const string& theClientNameR, const wstring& theClientDescR, const CompletionContainer<const ServerStatus&>& theCompletion = DEFAULT_COMPLETION, const ServerStatus* theServerStatusP = NULL);
	Error SubscriberShutdown( const CompletionContainer<const ServerStatus&>& theCompletion = DEFAULT_COMPLETION, const ServerStatus* theServerStatusP = NULL );
	Error SubscribeById( unsigned long thePublicationId, const CompletionContainer<const SubscribeReply&>& theCompletion = DEFAULT_COMPLETION, const SubscribeReply* theSubscribeReplyP = NULL);
	Error SubscribeByName( const string& theNameR, unsigned long thePublisherId = 0, const CompletionContainer<const SubscribeReply&>& theCompletion = DEFAULT_COMPLETION, const SubscribeReply* theSubscribeReplyP = NULL );
	Error CancelSubscription( unsigned long thePublicationId, const CompletionContainer<const ServerStatus&>& theCompletion = DEFAULT_COMPLETION, const ServerStatus* theServerStatusP = NULL);
	Error GetDataPoolSize( unsigned long theDataPoolId, const CompletionContainer<const DataPoolSizeReply&>& theCompletion = DEFAULT_COMPLETION, const DataPoolSizeReply* theDataPoolSizeReplyP = NULL);
	Error GetDataPool( unsigned long theDataPoolId, const CompletionContainer<const DataPoolReply&>& theCompletion = DEFAULT_COMPLETION, const DataPoolReply* theDataPoolReplyP = NULL);

	// Templated functions that send messages to server
	template <class privsType>
	Error SubscriberStartup( const IPSocket::Address& theServerAddress, const unsigned short theLocalMonitorPort, const string& theClientNameR, const wstring& theClientDescR, void (*f)(const ServerStatus&, privsType), privsType t )
	{ return SubscriberStartup(theServerAddress, theLocalMonitorPort, theClientNameR, theClientDescR, new CompletionWithContents<const ServerStatus&, privsType>(t, f, true)); }

	template <class privsType>
	Error SubscriberShutdown( void (*f)(const ServerStatus&, privsType), privsType t )
	{ return SubscriberShutdown( new CompletionWithContents<const ServerStatus&, privsType>(t, f, true)); }

	template <class privsType>
	Error SubscribeById(  unsigned long thePublicationId, void (*f)(const SubscribeReply&, privsType), privsType t )
	{ return SubscribeById( thePublicationId, new CompletionWithContents<const SubscribeReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error SubscribeByName( const string& theNameR, unsigned long thePublisherId, void (*f)(const SubscribeReply&, privsType), privsType t )
	{ return SubscribeByName(theNameR, thePublisherId, new CompletionWithContents<const SubscribeReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error CancelSubscription( unsigned long thePublicationId, void (*f)(const ServerStatus&, privsType), privsType t )
	{ return CancelSubscription(thePublicationId, new CompletionWithContents<const ServerStatus&, privsType>(t, f, true)); }

	template <class privsType>
	Error GetDataPoolSize( unsigned long theDataPoolId, void (*f)(const DataPoolSizeReply&, privsType), privsType t )
	{ return GetDataPoolSize(theDataPoolId, new CompletionWithContents<const DataPoolSizeReply&, privsType>(t, f, true)); }

	template <class privsType>
	Error GetDataPool(  unsigned long theDataPoolId, void (*f)(const DataPoolReply&, privsType), privsType t )
	{ return GetDataPool( theDataPoolId, new CompletionWithContents<const DataPoolReply&, privsType>(t, f, true)); }

	// Set Notification message completions
	void SetDataPoolUpdatedCompletion( const CompletionContainer<const unsigned long&>& theCompletionContainer, unsigned long* NotifyDataP = NULL )
	{
		mDataPoolUpdatedCompletion = theCompletionContainer;
		mDataPoolUpdatedCompletion.OwnCompletion();
		mDataPoolUpdatedNotifyDataP = NotifyDataP;
	}
	void SetBroadcastDataPoolCompletion( const CompletionContainer<const PublicationData&>& theCompletionContainer, PublicationData* NotifyDataP = NULL )
	{
		mBroadcastDataPoolCompletion = theCompletionContainer;
		mBroadcastDataPoolCompletion.OwnCompletion();
		mBroadcastDataPoolNotifyDataP = NotifyDataP;
	}
	void SetBroadcastStreamCompletion( const CompletionContainer<const PublicationData&>& theCompletionContainer, PublicationData* NotifyDataP = NULL )
	{
		mBroadcastStreamCompletion = theCompletionContainer;
		mBroadcastStreamCompletion.OwnCompletion();
		mBroadcastStreamNotifyDataP = NotifyDataP;
	}
	void SetSubscriptionCanceledCompletion( const CompletionContainer<const unsigned long&>& theCompletionContainer, unsigned long* NotifyDataP = NULL)
	{
		mSubscriptionCanceledCompletion = theCompletionContainer;
		mSubscriptionCanceledCompletion.OwnCompletion();
		mSubscriptionCanceledNotifyDataP = NotifyDataP;
	}

// Private Methods
private:
	// Message Reply Handlers
	virtual void HandleSubscribeReply(const MMsgObsSubscribeReply& theReplyMsg);
	virtual void HandleGetDataPoolReply(const MMsgObsGetDataPoolReply& theReplyMsg);
	virtual void HandleGetDataPoolSizeReply(const MMsgObsGetDataPoolSizeReply& theReplyMsg);
	virtual void HandleAddSubscriberReply(const MMsgObsAddSubscriberReply& theReplyMsg);

	// Notification message handlers
	virtual void HandleDataPoolUpdated(const MMsgObsDataPoolUpdated& theNotifyMsg);
	virtual void HandleBroadcastDataPool(const MMsgObsBroadcastDataPool& theNotifyMsg);
	virtual void HandleBroadcastStream(const MMsgObsBroadcastStream& theNotifyMsg);
	virtual void HandleSubscriptionCanceled(const MMsgObsSubscriptionCanceled& theNotifyMsg);

// Member Data
private:
	// Notification message Compleation Containers
	CompletionContainer<const unsigned long&>	mDataPoolUpdatedCompletion;
	CompletionContainer<const PublicationData&>	mBroadcastDataPoolCompletion;
	CompletionContainer<const PublicationData&>	mBroadcastStreamCompletion;
	CompletionContainer<const unsigned long&>	mSubscriptionCanceledCompletion;

	// Notification message Data Pointers
	unsigned long*		mDataPoolUpdatedNotifyDataP;
	PublicationData*	mBroadcastDataPoolNotifyDataP;
	PublicationData*	mBroadcastStreamNotifyDataP;
	unsigned long*		mSubscriptionCanceledNotifyDataP;
};

}

#endif