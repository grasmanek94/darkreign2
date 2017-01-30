
///////////////////////////////////////////////////////////////////////////////////
//
// Observation server Client API
//
// Contains classes for Observation Publisher and Subscriber clients
//
///////////////////////////////////////////////////////////////////////////////////

#include <string>
#include "ObservationAPI.h"
#include "msg/Obs/MMsgTypesObs.h"

using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////////
// Constants

///////////////////////////////////////////////////////////////////////////////////
//
// Observation Client Base class
//
// This is an abstract base class.  Instantiate a Publisher or Subscriber Client object
//
///////////////////////////////////////////////////////////////////////////////////

ObservationClientBase::ObservationClientBase(ConnectionTypeEnum theConnectionType) :
	mClientId(0),
	mClientName(),
	mClientDesc(),
	mConnectionType(theConnectionType),
	mObsServerAddress(),
	mLocalAddress(),
	mSocketMgrP(NULL),
	mReplyCompletionList(),
	mCriticalSection()
{
}

ObservationClientBase::~ObservationClientBase() 
{
	if (mSocketMgrP)
	{
		mSocketMgrP->SetShuttingDown(true); // Do this here in case they did not call shutdown
		delete mSocketMgrP;
		mSocketMgrP = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Public Methods


Error
ObservationClientBase::
GetPublisherId(const string& thePublisherNameR, 
			   const CompletionContainer<const IdReply&>& theCompletion,
			   const IdReply* theIdReplyP)
{
	Error aReturn = Error_GeneralFailure;

	if (mSocketMgrP != NULL)
	{
		// Register with Observation server
		MMsgObsGetPublisherId aMsg;
		aMsg.SetPublisherName( thePublisherNameR );
		
		aMsg.Pack(); // May not be neccessary...

		return SendMMsgToServer(aMsg, new CompletionContainer<const IdReply&>(theCompletion), theIdReplyP);
	}

	return aReturn;
}

Error 
ObservationClientBase::
GetPublicationId(const string& thePublicationNameR, 
				 const string& thePublisherNameR,
				 const CompletionContainer<const IdReply&>& theCompletion,
				 const IdReply* theIdReplyP)
{
	Error aReturn = Error_GeneralFailure;

	if (mSocketMgrP != NULL)
	{
		// Register with Observation server
		MMsgObsGetPublicationId aMsg;
		aMsg.SetPublicationName( thePublicationNameR );
		aMsg.SetPublisherName( thePublisherNameR );
		
		aMsg.Pack(); // May not be neccessary...

		return SendMMsgToServer(aMsg, new CompletionContainer<const IdReply&>(theCompletion), theIdReplyP);
	}

	return aReturn;
}

Error 
ObservationClientBase::
GetName(unsigned long theResourceId,
		ResourceTypeEnum theResourceType,
		const CompletionContainer<const NameReply&>& theCompletion,
		const NameReply* theNameReplyP)
{
	Error aReturn = Error_GeneralFailure;

	if (mSocketMgrP != NULL)
	{
		// Register with Observation server
		MMsgObsGetName aMsg;
		aMsg.SetResourceId( theResourceId );
		aMsg.SetResourceType( theResourceType );
		
		aMsg.Pack(); // May not be neccessary...

		return SendMMsgToServer(aMsg, new CompletionContainer<const NameReply&>(theCompletion), theNameReplyP);
	}

	return aReturn;
}

Error 
ObservationClientBase::
GetNumSubscribers(unsigned long thePublicationId, 
				  const CompletionContainer<const NumSubscribersReply&>& theCompletion,
				  const NumSubscribersReply* theNumSubscribersReplyP)
{
	Error aReturn = Error_GeneralFailure;

	if (mSocketMgrP != NULL)
	{
		// Register with Observation server
		MMsgObsGetNumSubscribers aMsg;
		aMsg.SetPublicationId( thePublicationId );
		
		aMsg.Pack(); // May not be neccessary...

		return SendMMsgToServer(aMsg, new CompletionContainer<const NumSubscribersReply&>(theCompletion), theNumSubscribersReplyP);
	}

	return aReturn;
}

Error 
ObservationClientBase::
EnumeratePublications(unsigned long thePublisherId, 
					  const CompletionContainer<const PublicationListReply&>& theCompletion,
					  const PublicationListReply* thePublicationListReplyP)
{
	Error aReturn = Error_GeneralFailure;

	if (mSocketMgrP != NULL)
	{
		// Register with Observation server
		MMsgObsEnumeratePublications aMsg;
		aMsg.SetPublisherId( thePublisherId );
		
		aMsg.Pack(); // May not be neccessary...

		return SendMMsgToServer(aMsg, new CompletionContainer<const PublicationListReply&>(theCompletion), thePublicationListReplyP);
	}

	return aReturn;
}

Error 
ObservationClientBase::
EnumeratePublishers(const CompletionContainer<const PublisherListReply&>& theCompletion,
					const PublisherListReply* thePublisherListReplyP)
{
	Error aReturn = Error_GeneralFailure;

	if (mSocketMgrP != NULL)
	{
		// Register with Observation server
		MMsgObsEnumeratePublishers aMsg;
		
		aMsg.Pack(); // May not be neccessary...

		return SendMMsgToServer(aMsg, new CompletionContainer<const PublisherListReply&>(theCompletion), thePublisherListReplyP);
	}

	return aReturn;
}

Error 
ObservationClientBase::
EnumerateSubscribers(unsigned long thePublisherId, 
					 unsigned long thePublicationId, 
					 const CompletionContainer<const SubscriberListReply&>& theCompletion,
					 const SubscriberListReply* theSubscriberListReplyP)
{
	Error aReturn = Error_GeneralFailure;

	if (mSocketMgrP != NULL)
	{
		// Register with Observation server
		MMsgObsEnumerateSubscribers aMsg;
		aMsg.SetPublisherId( thePublisherId );
		aMsg.SetPublicationId( thePublicationId );
		
		aMsg.Pack(); // May not be neccessary...

		return SendMMsgToServer(aMsg, new CompletionContainer<const SubscriberListReply&>(theCompletion), theSubscriberListReplyP);
	}

	return aReturn;
}

Error 
ObservationClientBase::
EnumerateSubscriptions(unsigned long thePublisherId, 
					   unsigned long thePublicationId, 
					   const CompletionContainer<const SubscriptionListReply&>& theCompletion,
					   const SubscriptionListReply* theSubscriptionListReplyP)
{
	Error aReturn = Error_GeneralFailure;

	if (mSocketMgrP != NULL)
	{
		// Register with Observation server
		MMsgObsEnumerateSubscriptions aMsg;
		aMsg.SetPublisherId( thePublisherId );
		aMsg.SetPublicationId( thePublicationId );
		
		aMsg.Pack(); // May not be neccessary...

		return SendMMsgToServer(aMsg, new CompletionContainer<const SubscriptionListReply&>(theCompletion), theSubscriptionListReplyP);
	}

	return aReturn;
}


// Public utilities
/*
string
ObservationClientBase::GetLocalAddress()
{
	char aBuf[256];
	string aAddress;

	if (gethostname(aBuf,255) == 0)
	{
		HOSTENT* aHostEntP = gethostbyname(aBuf);
		if (aHostEntP != NULL)
		{
			aAddress = inet_ntoa(*(IN_ADDR*)aHostEntP->h_addr_list[0]);
		}
	}

	return aAddress;
}*/

// Callback

// Called whenever a message is received on the Socket.  Processes the message and then
// initiates another async recv with itself as the completion routine.
// Put receive loop for both publisher and subscriber in base class for symplicity
void ObservationClientBase::ReceiveLoop(const TMsgSocket::RecvBaseMsgResult& result, ObservationClientBase* This)
{
	if (result.msg)
	{
		unsigned long aMessageType = result.msg->GetMessageType();
		switch (aMessageType)
		{
			// Reply messages
			case ObsMsg_StatusReply:
			{
				MMsgObsStatusReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleStatusReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_AddPublisherReply:
			{
				MMsgObsAddPublisherReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleAddPublisherReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_PublishReply:
			{
				MMsgObsPublishReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandlePublishReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_SubscribeReply:
			{
				MMsgObsSubscribeReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleSubscribeReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_GetIdReply:
			{
				MMsgObsGetIdReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleGetIdReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_GetNameReply:
			{
				MMsgObsGetNameReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleGetNameReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_GetNumSubscribersReply:
			{
				MMsgObsGetNumSubscribersReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleGetNumSubscribersReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_GetDataPoolReply:
			{
				MMsgObsGetDataPoolReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleGetDataPoolReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_GetDataPoolSizeReply:
			{
				MMsgObsGetDataPoolSizeReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleGetDataPoolSizeReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_EnumeratePublishersReply:
			{
				MMsgObsEnumeratePublishersReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleEnumeratePublishersReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_EnumerateSubscribersReply:
			{
				MMsgObsEnumerateSubscribersReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleEnumerateSubscribersReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_EnumeratePublicationsReply:
			{
				MMsgObsEnumeratePublicationsReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleEnumeratePublicationsReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_EnumerateSubscriptionsReply:
			{
				MMsgObsEnumerateSubscriptionsReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleEnumerateSubscriptionsReply(aReplyMsg);
				}
			}
			break;

			case ObsMsg_AddSubscriberReply:
			{
				MMsgObsAddSubscriberReply aReplyMsg;
				if (This->FillMsgFromResult(aReplyMsg, result))
				{
					This->HandleAddSubscriberReply(aReplyMsg);
				}
			}
			break;
				
			// Notification messages
			case ObsMsg_DataPoolUpdated:
			{
				MMsgObsDataPoolUpdated aNotifyMsg;
				if (This->FillMsgFromResult(aNotifyMsg, result))
				{
					This->HandleDataPoolUpdated(aNotifyMsg);
				}
			}
			break;

			case ObsMsg_BroadcastDataPool:
			{
				MMsgObsBroadcastDataPool aNotifyMsg;
				if (This->FillMsgFromResult(aNotifyMsg, result))
				{
					This->HandleBroadcastDataPool(aNotifyMsg);
				}
			}
			break;

			case ObsMsg_BroadcastStream:
			{
				MMsgObsBroadcastStream aNotifyMsg;
				if (This->FillMsgFromResult(aNotifyMsg, result))
				{
					This->HandleBroadcastStream(aNotifyMsg);
				}
			}
			break;

			case ObsMsg_PublishRequested:
			{
				MMsgObsPublishRequested aNotifyMsg;
				if (This->FillMsgFromResult(aNotifyMsg, result))
				{
					This->HandlePublishRequested(aNotifyMsg);
				}
			}
			break;

			case ObsMsg_SubscriptionCanceled:
			{
				MMsgObsSubscriptionCanceled aNotifyMsg;
				if (This->FillMsgFromResult(aNotifyMsg, result))
				{
					This->HandleSubscriptionCanceled(aNotifyMsg);
				}
			}
			break;

			case ObsMsg_PublicationSubscribedTo:
			{
				MMsgObsPublicationSubscribedTo aNotifyMsg;
				if (This->FillMsgFromResult(aNotifyMsg, result))
				{
					This->HandlePublicationSubscribedTo(aNotifyMsg);
				}
			}
			break;

			default:
				assert(0); // Error unknown message type
				// ** - Output debug message 
			break;
		}
	}
	
	// Don't want to do this is socket is closing or closed
	//if ( result.theSocket->IsRecvOpen() )
	if ( !result.closed )
	{
		result.theSocket->RecvBaseMsgEx((WONMsg::BaseMessage**)NULL, -1, true, ReceiveLoop, This);
	}
	/*else
	{
#ifdef _DEBUG
	OutputDebugString("Receive loop socket closed");
#endif
	}*/

	delete result.msg;
}

//////////////////////////////////////////////////////////////////////////////
// Protected Methods

Error 
ObservationClientBase::ClientStartup(const IPSocket::Address& theServerAddress, const unsigned short theLocalMonitorPort, const string& theClientNameR, const wstring& theClientDescR)
{
	mClientName = theClientNameR;
	mClientDesc = theClientDescR;

	mObsServerAddress = theServerAddress;
	
	if (mSocketMgrP == NULL)
	{
		mSocketMgrP = new SocketMgr( theServerAddress, mConnectionType );

		mSocketMgrP->SetReceiveFunction(ReceiveLoop, this);

		if (theLocalMonitorPort != 0)
		{
			mLocalAddress.InitFromLocalIP(theLocalMonitorPort);
			// Start monitoring
			mSocketMgrP->Listen(mLocalAddress.GetPort());
		}
		else
		{
			mLocalAddress.Set(0,0);
		}
	}
	else
	{
		mSocketMgrP->SetShuttingDown(false);
	}

	return Error_Success;
}

void
ObservationClientBase::ClientShutdown()
{
	if (mSocketMgrP)
		mSocketMgrP->SetShuttingDown(true);
	//delete mSocketMgrP;
	//mSocketMgrP = NULL;
}

//////////////////////////////////////////////////////////////////////////////
// Utility Methods

const string 
ObservationClientBase::CreateAddressString(const string& theAddressR, unsigned int thePort) const
{
	WONCommon::wonstringstream aStrStrm;
	aStrStrm << theAddressR.c_str() << ":" << thePort;
	return aStrStrm.str();
}

// Send Titan message to Observation Server
Error 
ObservationClientBase::SendMMsgToServer(BaseMessage& theMsgR, const CompletionContainerBase* theCompletionP, const void* theReplyResultP)
{
	ReplyCompletionData aReplyCompletionData;
	aReplyCompletionData.mReplyId = GetReplyMsgType(theMsgR.GetMessageType());
	aReplyCompletionData.mCompletionP = const_cast<CompletionContainerBase*>(theCompletionP);
	aReplyCompletionData.mReplyResultP = const_cast<void*>(theReplyResultP);

	mCriticalSection.Enter();
	mReplyCompletionList.push_back(aReplyCompletionData);
	mCriticalSection.Leave();
	return mSocketMgrP->SendMMsgToServer(theMsgR);
}

bool 
ObservationClientBase::FillMsgFromResult(BaseMessage& theMsg, const TMsgSocket::RecvBaseMsgResult& result)
{
	bool aReturn = true;

	theMsg.AppendBytes(result.msg->GetDataLen(), result.msg->GetDataPtr());
	try
	{ 
		theMsg.Unpack(); 
	}
	catch (WONMsg::BadMsgException&)
	{ 
		// Might want to log a message here..
		aReturn = false; 
	}
	return aReturn;
}

unsigned long
ObservationClientBase::GetReplyMsgType(unsigned long theMsgType)
{
	unsigned long aReturn = 0;

	switch (theMsgType)
	{
		case ObsMsg_RemovePublisher:
		case ObsMsg_RemovePublication:
		case ObsMsg_CancelSubscription:
		case ObsMsg_RemoveSubscriber:
		case ObsMsg_UpdateDataPool:
		case ObsMsg_SendStream:
		case ObsMsg_ReestablishPersistentTCP:
			aReturn = ObsMsg_StatusReply;
		break;
		case ObsMsg_AddPublisher:
			aReturn = ObsMsg_AddPublisherReply;
		break;
		case ObsMsg_PublishDataPool:
		case ObsMsg_PublishStream:
			aReturn = ObsMsg_PublishReply;
		break;
		case ObsMsg_GetPublisherId:
		case ObsMsg_GetPublicationId:
			aReturn = ObsMsg_GetIdReply;
		break;
		case ObsMsg_GetName:
			aReturn = ObsMsg_GetNameReply;
		break;
		case ObsMsg_SubscribeById:
		case ObsMsg_SubscribeByName:
			aReturn = ObsMsg_SubscribeReply;
		break;
		case ObsMsg_GetNumSubscribers:
			aReturn = ObsMsg_GetNumSubscribersReply;
		break;
		case ObsMsg_EnumeratePublications:
			aReturn = ObsMsg_EnumeratePublicationsReply;
		break;
		case ObsMsg_EnumeratePublishers:
			aReturn = ObsMsg_EnumeratePublishersReply;
		break;
		case ObsMsg_EnumerateSubscribers:
			aReturn = ObsMsg_EnumerateSubscribersReply;
		break;
		case ObsMsg_EnumerateSubscriptions:
			aReturn = ObsMsg_EnumerateSubscriptionsReply;
		break;
		case ObsMsg_GetDataPoolSize:
			aReturn = ObsMsg_GetDataPoolSizeReply;
		break;
		case ObsMsg_GetDataPool:
			aReturn = ObsMsg_GetDataPoolReply;
		break;
		case ObsMsg_AddSubscriber:
			aReturn = ObsMsg_AddSubscriberReply;
		break;
		default:
			assert(0);
			// Unknown message type
		break;
	}
	return aReturn;
}

// Get first completion routine in the list for this reply type
// This is a FIFO style in that we will get the 
// Remove the completion routine from the list as well 
// Note: if the server sends us a reply that we do not expect
// or does not send us a reply we will get out of sinc...
// Will need to add some mechanism to handle this if it
// gets to be a problem.  We might want to add a function to 
// clean out the list when the socket goes of line.
// For UDP we might want to only allow one completion per reply type.
ObservationClientBase::ReplyCompletionData
ObservationClientBase::GetFirstCompletion(unsigned long theReplyId)
{
	ReplyCompletionList::iterator aItr;
	ReplyCompletionData aReturn;

	mCriticalSection.Enter();
	// Get first instance of completion for this reply type 
	for (aItr = mReplyCompletionList.begin(); aItr != mReplyCompletionList.end(); aItr++)
	{
		if (aItr->mReplyId == theReplyId)
		{
			aReturn = *aItr;
			mReplyCompletionList.erase(aItr); // Remove completion
			break;
		}
	}
	mCriticalSection.Leave();
	return aReturn;
}

//////////////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Message Reply Handlers
//
// Note - Classes derrived from Publisher or Subscriber clients can override 
// these functions to provide their own completion message handlers.
//////////////////////////////////////////////////////////////////////////////

void 
ObservationClientBase::HandleStatusReply(const MMsgObsStatusReply& theReplyMsg)
{
	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_StatusReply);
	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((ServerStatus*)aReplyCompletionData.mReplyResultP) = theReplyMsg.GetStatus();
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const ServerStatus&>*>(aReplyCompletionData.mCompletionP))->Complete(theReplyMsg.GetStatus());
		delete aReplyCompletionData.mCompletionP; 
	}
}

void 
ObservationClientBase::HandleGetIdReply(const MMsgObsGetIdReply& theReplyMsg)
{
	
	IdReply aIdReply;
	aIdReply.mStatus = theReplyMsg.GetStatus();
	aIdReply.mId = theReplyMsg.GetResourceId();
	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_GetIdReply);
	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((IdReply*)aReplyCompletionData.mReplyResultP) = aIdReply;
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const IdReply&>*>(aReplyCompletionData.mCompletionP))->Complete(aIdReply);
		delete aReplyCompletionData.mCompletionP;
	}
}

void 
ObservationClientBase::HandleGetNameReply(const MMsgObsGetNameReply& theReplyMsg)
{
	
	NameReply aNameReply;
	aNameReply.mStatus = theReplyMsg.GetStatus();
	aNameReply.mName = theReplyMsg.GetResourceName();
	aNameReply.mDescription = theReplyMsg.GetResourceDescription();
	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_GetNameReply);
	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((NameReply*)aReplyCompletionData.mReplyResultP) = aNameReply;
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const NameReply&>*>(aReplyCompletionData.mCompletionP))->Complete(aNameReply);
		delete aReplyCompletionData.mCompletionP;
	}
}

void 
ObservationClientBase::HandleGetNumSubscribersReply(const MMsgObsGetNumSubscribersReply& theReplyMsg)
{
	NumSubscribersReply aNumSubscribersReply;
	aNumSubscribersReply.mStatus = theReplyMsg.GetStatus();
	aNumSubscribersReply.mNumSubscribers = theReplyMsg.GetNumSubscribers();
	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_GetNumSubscribersReply);
	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((NumSubscribersReply*)aReplyCompletionData.mReplyResultP) = aNumSubscribersReply;
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const NumSubscribersReply&>*>(aReplyCompletionData.mCompletionP))->Complete(aNumSubscribersReply);
		delete aReplyCompletionData.mCompletionP;
	}
}

void 
ObservationClientBase::HandleEnumeratePublishersReply(const MMsgObsEnumeratePublishersReply& theReplyMsg)
{
	
	PublisherListReply aPublisherListReply;
	aPublisherListReply.mStatus = theReplyMsg.GetStatus();
	aPublisherListReply.mPublisherList = theReplyMsg.GetPublisherList(); 
	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_EnumeratePublishersReply);
	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((PublisherListReply*)aReplyCompletionData.mReplyResultP) = aPublisherListReply;
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const PublisherListReply&>*>(aReplyCompletionData.mCompletionP))->Complete(aPublisherListReply);
		delete aReplyCompletionData.mCompletionP;
	}
}

void 
ObservationClientBase::HandleEnumerateSubscribersReply(const MMsgObsEnumerateSubscribersReply& theReplyMsg)
{
	SubscriberListReply aSubscriberListReply;
	aSubscriberListReply.mStatus = theReplyMsg.GetStatus();
	aSubscriberListReply.mSubscriberList = theReplyMsg.GetSubscriberList();
	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_EnumerateSubscribersReply);
	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((SubscriberListReply*)aReplyCompletionData.mReplyResultP) = aSubscriberListReply;
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const SubscriberListReply&>*>(aReplyCompletionData.mCompletionP))->Complete(aSubscriberListReply);
		delete aReplyCompletionData.mCompletionP;
	}
}

void 
ObservationClientBase::HandleEnumeratePublicationsReply(const MMsgObsEnumeratePublicationsReply& theReplyMsg)
{
	PublicationListReply aPublicationListReply;
	aPublicationListReply.mStatus = theReplyMsg.GetStatus();
	aPublicationListReply.mPublicationList = theReplyMsg.GetPublicationList();
	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_EnumeratePublicationsReply);
	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((PublicationListReply*)aReplyCompletionData.mReplyResultP) = aPublicationListReply;
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const PublicationListReply&>*>(aReplyCompletionData.mCompletionP))->Complete(aPublicationListReply);
		delete aReplyCompletionData.mCompletionP;
	}
}

void 
ObservationClientBase::HandleEnumerateSubscriptionsReply(const MMsgObsEnumerateSubscriptionsReply& theReplyMsg)
{
	SubscriptionListReply aSubscriptionListReply;
	aSubscriptionListReply.mStatus = theReplyMsg.GetStatus();
	aSubscriptionListReply.mSubscriptionList = theReplyMsg.GetSubscriptionList();
	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_EnumerateSubscriptionsReply);
	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((SubscriptionListReply*)aReplyCompletionData.mReplyResultP) = aSubscriptionListReply;
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const SubscriptionListReply&>*>(aReplyCompletionData.mCompletionP))->Complete(aSubscriptionListReply);
		delete aReplyCompletionData.mCompletionP;
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Observation Publisher Client
//
///////////////////////////////////////////////////////////////////////////////////

ObservationPublisherClient::ObservationPublisherClient(ConnectionTypeEnum theConnectionType) :
	ObservationClientBase(theConnectionType)
{
	mPublishRequestedCompletion = CompletionContainer<const string&>();
	mPublicationSubscribedToCompletion = CompletionContainer<const ObservationPublisherClient::PublicationSubscribedTo&>();

	mPublishRequestedDataP = NULL;
	mPublicationSubscribedToNotifyDataP = NULL;
}

ObservationPublisherClient::~ObservationPublisherClient() 
{
}


//////////////////////////////////////////////////////////////////////////////
// Public Methods

//////////////////////////////////////////////////////////////////////////////
// PublisherStartup
//
// Method called to identify publisher client to Observation server and establish
// groundword for communication between them.
//
// Important:	The reply for this function must be returned before any other
//				publisher functions can be called.  Any functions called
//				before the reply has returned will return a general failure error.
//////////////////////////////////////////////////////////////////////////////
Error 
ObservationPublisherClient::
PublisherStartup( const IPSocket::Address& theServerAddress,					// Address and port of Observation server to publish on
				  const unsigned short theLocalMonitorPort,						// Local port which to monitor for communications from server
				  const string& theClientNameR,									// Unique name of client publishing on Observation server
				  const wstring& theClientDescR,								// User readable description of client publishing on server
				  const bool theAccessPubsByName,								// Determines wheather publication names will be unique acros this publisher. 
				  const CompletionContainer<const ServerStatus&>& theCompletion, // Completion for this operation - Function pointer will be called or event will be signaled when this operation completes
				  const ServerStatus* theServerStatusP )
{
	ClientStartup(theServerAddress, theLocalMonitorPort, theClientNameR, theClientDescR);

	if (mSocketMgrP != NULL)
	{
		mSocketMgrP->SetClientType(ResourcePublisher);

		// Register with Observation server
		MMsgObsAddPublisher aMsg;
		aMsg.SetPublisherName( mClientName );
		aMsg.SetPublisherDescription( mClientDesc );
		aMsg.SetConnectionType(	mConnectionType );
		aMsg.SetNetAddress( mLocalAddress.GetAddressString() ); // Our net address IP:Port
		aMsg.SetAccessPubsByName(theAccessPubsByName); 
		aMsg.SetOnDemandPublishing(false); // Don't support this in the API for now

		aMsg.Pack(); // May not be neccessary...

		return SendMMsgToServer(aMsg, new CompletionContainer<const ServerStatus&>(theCompletion), theServerStatusP);
	}
	else
	{
		return Error_GeneralFailure;
	}
}

//////////////////////////////////////////////////////////////////////////////
// PublisherShutdown
//
// Method called to shutdown the Publisher client.  Sends message to Observation
// Server to remove this publisher from it's table of publishers.  All publications
// published by this publisher will also be removed.
//
// Returns success or failure on compleation.
//////////////////////////////////////////////////////////////////////////////
Error
ObservationPublisherClient::
PublisherShutdown( const CompletionContainer<const ServerStatus&>& theCompletion, 
				   const ServerStatus* theServerStatusP) // Completion for this operation - Function pointer will be called or event will be signaled when this operation completes
{
	Error aReturn = Error_Success;

	ClientShutdown();

	if (mClientId != 0) 
	{
		MMsgObsRemovePublisher aMsg;

		aMsg.SetPublisherId(mClientId);
		
		aMsg.Pack();

		aReturn = SendMMsgToServer(aMsg, new CompletionContainer<const ServerStatus&>(theCompletion), theServerStatusP);
		mClientId = 0;
		mSocketMgrP->SetClientId(mClientId);
		//delete mSocketMgrP;
		//mSocketMgrP = NULL;
	}
	else // Complete with error
	{
		aReturn = Error_GeneralFailure;
		theCompletion.Complete(StatusObs_UnknownPublisher);
	}

	return aReturn;
}

//////////////////////////////////////////////////////////////////////////////
// PublishDataPool
//
// Method called to publish a data pool on the observation server.
//
// When the return compleats the status of the transaction and the Id of the
// published data pool will be returned.  This Id is used to identify a data pool
// on the Observation Server and needs to be passed into any method that operates
// on the data pool.
//////////////////////////////////////////////////////////////////////////////
Error
ObservationPublisherClient::
PublishDataPool(const string& theNameR,								// Name of the Data pool
				const wstring& theDescriptionR,						// User readable description of the Data Pool
				const bool theSubscriptionNotification,				// Flag that determines if publisher wants to be notified when client subscibes to data pool
				const BYTE* theDataP,								// Pointer to the data
				const unsigned short theDataLength,					// Length of data
				const CompletionContainer<const IdReply&>& theCompletion,	// Completion for this operation - Function pointer will be called or event will be signaled when this operation completes
				const IdReply* theIdReplyP )
{
	Error aReturn = Error_GeneralFailure;

	if (mClientId != 0) // Might want to wait if add publisher message is pending..
	{
		MMsgObsPublishDataPool aMsg;

		aMsg.SetPublisherId(mClientId);
		aMsg.SetPublicationName(theNameR);
		aMsg.SetPublicationDescription(theDescriptionR);
		aMsg.SetSubscriptionNotification(theSubscriptionNotification); 
		aMsg.SetData(theDataP, theDataLength);

		aMsg.Pack();

		aReturn = SendMMsgToServer(aMsg, new CompletionContainer<const IdReply&>(theCompletion), theIdReplyP);
	}

	return aReturn;
}

//////////////////////////////////////////////////////////////////////////////
// PublishStream
//
// Method called to
//
//////////////////////////////////////////////////////////////////////////////
Error
ObservationPublisherClient::
PublishStream(const string& theNameR,								// Name of the Stream
			  const wstring& theDescriptionR,						// User readable description of the Stream
			  const bool theSubscriptionNotification,				// Flag that determines if publisher wants to be notified when client subscibes to Stream
			  const CompletionContainer<const IdReply&>& theCompletion,	// Completion for this operation - Function pointer will be called or event will be signaled when this operation completes
			  const IdReply* theIdReplyP )
{
	Error aReturn = Error_GeneralFailure;

	if (mClientId != 0) // Might want to wait if add publisher message is pending..
	{
		MMsgObsPublishStream aMsg;

		aMsg.SetPublisherId(mClientId);
		aMsg.SetPublicationName(theNameR);
		aMsg.SetPublicationDescription(theDescriptionR);
		aMsg.SetSubscriptionNotification(theSubscriptionNotification); 

		aMsg.Pack();

		aReturn = SendMMsgToServer(aMsg, new CompletionContainer<const IdReply&>(theCompletion), theIdReplyP);
	}

	return aReturn;
}
	
//////////////////////////////////////////////////////////////////////////////
// PublisherShutdown
//
// Method called to
//
//////////////////////////////////////////////////////////////////////////////	
Error 
ObservationPublisherClient::
UpdateDataPool(unsigned long theDataPoolId,										// Id of data pool to updata
			   const BYTE* theDataP,											// Pointer to the data
			   const unsigned short theDataLength,								// Length of data 
			   const CompletionContainer<const ServerStatus&>& theCompletion,	// Completion for this operation - Function pointer will be called or event will be signaled when this operation completes
			   const ServerStatus* theServerStatusP )
{
	Error aReturn = Error_GeneralFailure;

	if (mClientId != 0) // Might want to wait if add publisher message is pending..
	{
		MMsgObsUpdateDataPool aMsg;

		aMsg.SetDataPoolId(theDataPoolId);
		aMsg.SetData(theDataP, theDataLength);

		aMsg.Pack();

		aReturn = SendMMsgToServer(aMsg, new CompletionContainer<const ServerStatus&>(theCompletion), theServerStatusP);
	}

	return aReturn;
}

//////////////////////////////////////////////////////////////////////////////
// SendStream
//
// Method called to
//
//////////////////////////////////////////////////////////////////////////////	
Error
ObservationPublisherClient::
SendStream(unsigned long theStreamId,										// Id of stream to send
		   const BYTE* theDataP,											// Pointer to the data
		   const unsigned short theDataLength,								// Length of data
		   const bool theSendReply,											// Determines whether the observation server sends a reply for this message
		   const CompletionContainer<const ServerStatus&>& theCompletion,	// Completion for this operation - Function pointer will be called or event will be signaled when this operation completes
		   const ServerStatus* theServerStatusP )
{
	Error aReturn = Error_GeneralFailure;

	if (mClientId != 0) // Might want to wait if add publisher message is pending..
	{
		MMsgObsSendStream aMsg;

		aMsg.SetStreamId(theStreamId);
		aMsg.SetData(theDataP, theDataLength);
		aMsg.SetSendReply(theSendReply);

		aMsg.Pack();

		if (theSendReply)
		{
			aReturn = SendMMsgToServer(aMsg, new CompletionContainer<const ServerStatus&>(theCompletion), theServerStatusP);
		}
		else // Don't expect reply
		{
			mSocketMgrP->SendMMsgToServer(aMsg);
			aReturn = Error_Success;
		}
	}

	return aReturn;
}

//////////////////////////////////////////////////////////////////////////////
// RemovePublication
//
// Method called to
//
//////////////////////////////////////////////////////////////////////////////	
Error
ObservationPublisherClient::
RemovePublication(unsigned long thePublicationId,									// Id of publication (Data pool or Stream) to remove
				  const CompletionContainer<const ServerStatus&>& theCompletion,		// Completion for this operation - Function pointer will be called or event will be signaled when this operation completes
				  const ServerStatus* theServerStatusP )
{
	Error aReturn = Error_GeneralFailure;

	if (mClientId != 0) // Might want to wait if add publisher message is pending..
	{
		MMsgObsRemovePublication aMsg;

		aMsg.SetPublicationId(thePublicationId);

		aMsg.Pack();

		aReturn = SendMMsgToServer(aMsg, new CompletionContainer<const ServerStatus&>(theCompletion), theServerStatusP);
	}

	return aReturn;
}

//////////////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Message Reply Handlers

void 
ObservationPublisherClient::HandleAddPublisherReply(const MMsgObsAddPublisherReply& theReplyMsg)
{
	// Note: may want to fail on already exists in the future...
	if (theReplyMsg.GetStatus() == StatusCommon_Success || theReplyMsg.GetStatus() == StatusCommon_AlreadyExists)
	{
		mClientId = theReplyMsg.GetPublisherId();
		mSocketMgrP->SetClientId(mClientId);  // For Persistent TCP reconnect
	}

	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_AddPublisherReply);
	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((ServerStatus*)aReplyCompletionData.mReplyResultP) = theReplyMsg.GetStatus();
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const ServerStatus&>*>(aReplyCompletionData.mCompletionP))->Complete(theReplyMsg.GetStatus());
		delete aReplyCompletionData.mCompletionP;
	}
}

void 
ObservationPublisherClient::HandlePublishReply(const MMsgObsPublishReply& theReplyMsg)
{
	IdReply aPublishResult;
	aPublishResult.mStatus = theReplyMsg.GetStatus();
	aPublishResult.mId = theReplyMsg.GetPublicationId();
	
	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_PublishReply);
	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((IdReply*)aReplyCompletionData.mReplyResultP) = aPublishResult;
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const IdReply&>*>(aReplyCompletionData.mCompletionP))->Complete(aPublishResult);
		delete aReplyCompletionData.mCompletionP;
	}
}


//////////////////////////////////////////////////////////////////////////////
// Notification Message Handlers

void 
ObservationPublisherClient::HandlePublishRequested(const MMsgObsPublishRequested& theNotifyMsg)
{
	if (mPublishRequestedDataP != NULL)
	{
		*mPublishRequestedDataP = theNotifyMsg.GetPublicationName();
	}
	if (!mPublishRequestedCompletion.empty())
	{
		mPublishRequestedCompletion.Complete(theNotifyMsg.GetPublicationName());
	}
}

void 
ObservationPublisherClient::HandlePublicationSubscribedTo(const MMsgObsPublicationSubscribedTo& theNotifyMsg)
{
	PublicationSubscribedTo aPubData;
	aPubData.mPublicationId = theNotifyMsg.GetPublicationId();
	aPubData.mSubscriberId = theNotifyMsg.GetSubscriberId();
	aPubData.mSubscriberName = theNotifyMsg.GetSubscriberName();
	if (mPublicationSubscribedToNotifyDataP != NULL)
	{
		*mPublicationSubscribedToNotifyDataP = aPubData;
	}
	if (!mPublicationSubscribedToCompletion.empty())
	{
		mPublicationSubscribedToCompletion.Complete(aPubData);
	}
}



///////////////////////////////////////////////////////////////////////////////////
//
// Observation Subscriber Client
//
///////////////////////////////////////////////////////////////////////////////////

ObservationSubscriberClient::ObservationSubscriberClient(ConnectionTypeEnum theConnectionType) :
	ObservationClientBase(theConnectionType)
{
 	mDataPoolUpdatedCompletion = CompletionContainer<const unsigned long&>();					
	mBroadcastDataPoolCompletion = CompletionContainer<const ObservationClientBase::PublicationData&>(); 
	mBroadcastStreamCompletion = CompletionContainer<const ObservationClientBase::PublicationData&>();
	mSubscriptionCanceledCompletion = CompletionContainer<const unsigned long&>();

	mDataPoolUpdatedNotifyDataP = NULL;
	mBroadcastDataPoolNotifyDataP = NULL;
	mBroadcastStreamNotifyDataP = NULL;
	mSubscriptionCanceledNotifyDataP = NULL;
}

ObservationSubscriberClient::~ObservationSubscriberClient() 
{
}

//////////////////////////////////////////////////////////////////////////////
// Public Methods

//////////////////////////////////////////////////////////////////////////////
// SubscriberStartup
//
// Method called to setup the subscriber client and start monitoring the socket
// specified.
//
// Note -	Unlike the Publisher startup method no communication is initiated
//			with the Observation server.  Communication is not initiated with
//			the observation server until one of the other functions is called.
//////////////////////////////////////////////////////////////////////////////
Error 
ObservationSubscriberClient::
SubscriberStartup(const IPSocket::Address& theServerAddress,					// Address and port of Observation server to publish on
				  const unsigned short theLocalMonitorPort,						// Local port which to monitor for communications from server
				  const string& theClientNameR,									// Unique name of client publishing on Observation server
				  const wstring& theClientDescR,								// User readable description of client publishing on server
				  const CompletionContainer<const ServerStatus&>& theCompletion, // Completion for this operation - Function pointer will be called or event will be signaled when this operation completes
				  const ServerStatus* theServerStatusP )
{
	ClientStartup(theServerAddress, theLocalMonitorPort, theClientNameR, theClientDescR);

	if (mSocketMgrP != NULL)
	{
		mSocketMgrP->SetClientType(ResourceSubscriber);

		// Register with Observation server
		MMsgObsAddSubscriber aMsg;
		aMsg.SetSubscriberName( mClientName );
		aMsg.SetSubscriberDescription( mClientDesc );
		aMsg.SetConnectionType(	mConnectionType );
		aMsg.SetNetAddress( mLocalAddress.GetAddressString() ); // Our net address IP:Port
		
		aMsg.Pack(); // May not be neccessary...

		return SendMMsgToServer(aMsg, new CompletionContainer<const ServerStatus&>(theCompletion), theServerStatusP);
	}
	else
	{
		return Error_GeneralFailure;
	}
}

Error
ObservationSubscriberClient::
SubscriberShutdown(const CompletionContainer<const ServerStatus&>& theCompletion,
				   const ServerStatus* theServerStatusP )
{
	Error aReturn = Error_Success;

	ClientShutdown();

	if (mClientId != 0) 
	{
		MMsgObsRemoveSubscriber aMsg;

		aMsg.SetSubscriberId(mClientId);
		
		aMsg.Pack();

		aReturn = SendMMsgToServer(aMsg, new CompletionContainer<const ServerStatus&>(theCompletion), theServerStatusP); 
		mClientId = 0; // In case client starts up again.
		
		mSocketMgrP->SetClientId(mClientId);
		//delete mSocketMgrP;
		//mSocketMgrP = NULL;
	}
	else // Complete with error
	{
		aReturn = Error_GeneralFailure;
		theCompletion.Complete(StatusObs_UnknownSubscriber);
	}

	return aReturn;
}

// Note : The data returned on completion (SubscribeReply.mDataP) is owned by the caller and must be deleated by them
Error 
ObservationSubscriberClient::
SubscribeById(unsigned long thePublicationId, 
			  const CompletionContainer<const SubscribeReply&>& theCompletion,
			  const SubscribeReply* theSubscribeReplyP)
{
	MMsgObsSubscribeById aMsg;
	aMsg.SetPublicationId(thePublicationId);
	aMsg.SetSubscriberId(mClientId);

	if (mClientId == 0) // Need to add subscriber
	{
		aMsg.SetSubscriberName(mClientName);
		aMsg.SetSubscriberDescription(mClientDesc);
		aMsg.SetConnectionType(mConnectionType);
		aMsg.SetSubscriberAddress(mLocalAddress.GetAddressString());	
	}

	aMsg.Pack();

	return SendMMsgToServer(aMsg, new CompletionContainer<const SubscribeReply&>(theCompletion), theSubscribeReplyP);
}

// Note : The data returned on completion (SubscribeReply.mDataP) is owned by the caller and must be deleated by them
Error 
ObservationSubscriberClient::
SubscribeByName(const string& theNameR, 
				unsigned long thePublisherId, 
				const CompletionContainer<const SubscribeReply&>& theCompletion,
				const SubscribeReply* theSubscribeReplyP)
{
	MMsgObsSubscribeByName aMsg;
	aMsg.SetPublicationName(theNameR);
	aMsg.SetPublisherId(thePublisherId);
	aMsg.SetSubscriberId(mClientId);

	if (mClientId == 0) // Need to add subscriber
	{
		aMsg.SetSubscriberName(mClientName);
		aMsg.SetSubscriberDescription(mClientDesc);
		aMsg.SetConnectionType(mConnectionType);
		aMsg.SetSubscriberAddress(mLocalAddress.GetAddressString());
		aMsg.SetRequestPublish(false); // Turn off this feature for now
	}

	aMsg.Pack();

	return SendMMsgToServer(aMsg, new CompletionContainer<const SubscribeReply&>(theCompletion), theSubscribeReplyP);
}

Error 
ObservationSubscriberClient::
CancelSubscription(unsigned long thePublicationId, 
				   const CompletionContainer<const ServerStatus&>& theCompletion,
				   const ServerStatus* theServerStatusP)
{
	Error aReturn = Error_GeneralFailure;

	if (mClientId != 0) 
	{
		MMsgObsCancelSubscription aMsg;

		aMsg.SetPublicationId(thePublicationId);
		aMsg.SetSubscriberId(mClientId);
		
		aMsg.Pack();

		aReturn = SendMMsgToServer(aMsg, new CompletionContainer<const ServerStatus&>(theCompletion), theServerStatusP); 
	}

	return aReturn;
}

Error 
ObservationSubscriberClient::
GetDataPoolSize(unsigned long theDataPoolId, 
				const CompletionContainer<const DataPoolSizeReply&>& theCompletion,
				const DataPoolSizeReply* theDataPoolSizeReplyP)
{
	Error aReturn = Error_GeneralFailure;

	if (mClientId != 0) 
	{
		MMsgObsGetDataPoolSize aMsg;

		aMsg.SetDataPoolId(theDataPoolId);
		
		aMsg.Pack();

		aReturn = SendMMsgToServer(aMsg, new CompletionContainer<const DataPoolSizeReply&>(theCompletion), theDataPoolSizeReplyP); 
	}

	return aReturn;
}

Error 
ObservationSubscriberClient::
GetDataPool(unsigned long theDataPoolId, 
			const CompletionContainer<const DataPoolReply&>& theCompletion,
			const DataPoolReply* theDataPoolReplyP)
{
	Error aReturn = Error_GeneralFailure;

	if (mClientId != 0) 
	{
		MMsgObsGetDataPool aMsg;

		aMsg.SetDataPoolId(theDataPoolId);
		
		aMsg.Pack();

		aReturn = SendMMsgToServer(aMsg, new CompletionContainer<const DataPoolReply&>(theCompletion), theDataPoolReplyP); 
	}

	return aReturn;
}

//////////////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
// Message Reply Handlers

void 
ObservationSubscriberClient::HandleSubscribeReply(const MMsgObsSubscribeReply& theReplyMsg)
{
	SubscribeReply aSubscribeReply;
	aSubscribeReply.mStatus = theReplyMsg.GetStatus();			
	aSubscribeReply.mPublicationId = theReplyMsg.GetPublicationId();
	if (mClientId == 0) 
	{
		mClientId = theReplyMsg.GetSubscriberId();
		mSocketMgrP->SetClientId(mClientId);
	}
	aSubscribeReply.mSubscriberId = mClientId;
	aSubscribeReply.mDataLength = theReplyMsg.GetDataLength();				
	aSubscribeReply.mDataP = new BYTE[theReplyMsg.GetDataLength()]; // Need to reallocate data API caller must delete - Might want to use smart pointer in future
	memcpy(aSubscribeReply.mDataP, theReplyMsg.GetData(), aSubscribeReply.mDataLength);
	aSubscribeReply.mPeerRedirection = theReplyMsg.GetPeerRedirection();

	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_SubscribeReply);

	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((SubscribeReply*)aReplyCompletionData.mReplyResultP) = aSubscribeReply;
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const SubscribeReply&>*>(aReplyCompletionData.mCompletionP))->Complete(aSubscribeReply);
		delete aReplyCompletionData.mCompletionP;
	}
}

void 
ObservationSubscriberClient::HandleGetDataPoolReply(const MMsgObsGetDataPoolReply& theReplyMsg)
{
	DataPoolReply aDataPoolReply;
	aDataPoolReply.mStatus = theReplyMsg.GetStatus();			
	aDataPoolReply.mDataPoolId = theReplyMsg.GetDataPoolId();		
	aDataPoolReply.mDataLength = theReplyMsg.GetDataLength();		
	aDataPoolReply.mDataP = new BYTE[theReplyMsg.GetDataLength()]; // Need to reallocate data API caller must delete - Might want to use smart pointer in future
	memcpy(aDataPoolReply.mDataP, theReplyMsg.GetData(), aDataPoolReply.mDataLength);
	
	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_GetDataPoolReply);

	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((DataPoolReply*)aReplyCompletionData.mReplyResultP) = aDataPoolReply;
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const DataPoolReply&>*>(aReplyCompletionData.mCompletionP))->Complete(aDataPoolReply);
		delete aReplyCompletionData.mCompletionP;
	}
}

void 
ObservationSubscriberClient::HandleGetDataPoolSizeReply(const MMsgObsGetDataPoolSizeReply& theReplyMsg)
{
	DataPoolSizeReply aDataPoolSizeReply;
	aDataPoolSizeReply.mStatus = theReplyMsg.GetStatus();			
	aDataPoolSizeReply.mDataPoolSize = theReplyMsg.GetDataPoolSize();		
	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_GetDataPoolSizeReply);

	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((DataPoolSizeReply*)aReplyCompletionData.mReplyResultP) = aDataPoolSizeReply;
	}	
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const DataPoolSizeReply&>*>(aReplyCompletionData.mCompletionP))->Complete(aDataPoolSizeReply);
		delete aReplyCompletionData.mCompletionP;
	}
}

void 
ObservationSubscriberClient::HandleAddSubscriberReply(const MMsgObsAddSubscriberReply& theReplyMsg)
{
	// Note: may want to fail on already exists in the future...
	if (theReplyMsg.GetStatus() == StatusCommon_Success || theReplyMsg.GetStatus() == StatusCommon_AlreadyExists)
	{
		mClientId = theReplyMsg.GetSubscriberId();
		mSocketMgrP->SetClientId(mClientId);  // For Persistent TCP reconnect
	}

	ReplyCompletionData aReplyCompletionData = GetFirstCompletion(ObsMsg_AddSubscriberReply);
	if (aReplyCompletionData.mReplyResultP != NULL) // Fill in data in structure provided
	{
		*((ServerStatus*)aReplyCompletionData.mReplyResultP) = theReplyMsg.GetStatus();
	}
	if (aReplyCompletionData.mCompletionP != NULL)
	{
		(reinterpret_cast<CompletionContainer<const ServerStatus&>*>(aReplyCompletionData.mCompletionP))->Complete(theReplyMsg.GetStatus());
		delete aReplyCompletionData.mCompletionP;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Notification message handlers
//
// Need to support both pointer to data passed in and one that is newed off
// Passed in pointer is for syncronous use with events
// Newed off data structure is for windows messages

void 
ObservationSubscriberClient::HandleDataPoolUpdated(const MMsgObsDataPoolUpdated& theNotifyMsg)
{
	if (mDataPoolUpdatedNotifyDataP != NULL)
	{
		*mDataPoolUpdatedNotifyDataP = theNotifyMsg.GetDataPoolId();
	}
	if (!mDataPoolUpdatedCompletion.empty())
	{
		mDataPoolUpdatedCompletion.Complete(theNotifyMsg.GetDataPoolId());
	}
}

void 
ObservationSubscriberClient::HandleBroadcastDataPool(const MMsgObsBroadcastDataPool& theNotifyMsg)
{
	PublicationData aPubData;
	aPubData.mDataId = theNotifyMsg.GetDataPoolId();
	aPubData.mDataLength = theNotifyMsg.GetDataLength(); 
	aPubData.mDataP = new BYTE[theNotifyMsg.GetDataLength()]; // Need to reallocate data API caller must delete - Might want to use smart pointer in future
	memcpy(aPubData.mDataP, theNotifyMsg.GetData(), theNotifyMsg.GetDataLength());

	if (mBroadcastDataPoolNotifyDataP != NULL)
	{
		*mBroadcastDataPoolNotifyDataP = aPubData;
	}
	if (!mBroadcastDataPoolCompletion.empty())
	{
		mBroadcastDataPoolCompletion.Complete(aPubData);
	}
}

void 
ObservationSubscriberClient::HandleBroadcastStream(const MMsgObsBroadcastStream& theNotifyMsg)
{
	PublicationData aPubData;
	aPubData.mDataId = theNotifyMsg.GetStreamId();
	aPubData.mDataLength = theNotifyMsg.GetDataLength(); 
	aPubData.mDataP = new BYTE[theNotifyMsg.GetDataLength()]; // Need to reallocate data API caller must delete - Might want to use smart pointer in future
	memcpy(aPubData.mDataP, theNotifyMsg.GetData(), theNotifyMsg.GetDataLength());

	if (mBroadcastStreamNotifyDataP != NULL)
	{
		*mBroadcastStreamNotifyDataP = aPubData;
	}
	if (!mBroadcastStreamCompletion.empty())
	{
		mBroadcastStreamCompletion.Complete(aPubData);
	}
}

void 
ObservationSubscriberClient::HandleSubscriptionCanceled(const MMsgObsSubscriptionCanceled& theNotifyMsg)
{
	if (mSubscriptionCanceledNotifyDataP != NULL)
	{
		*mSubscriptionCanceledNotifyDataP = theNotifyMsg.GetPublicationId();
	}
	if (!mSubscriptionCanceledCompletion.empty())
	{
		mSubscriptionCanceledCompletion.Complete(theNotifyMsg.GetPublicationId());
	}
}



