///////////////////////////////////////////////////////////////////////////////////
//
// Socket Manager
//
// This class manages TMsgSocket API sockets and provides like functionality for
// PersistentTCP, TransientTCP, and UDP communication with the Observation server.
// 
// For TransientTCP mFreeSocketQueue is a pool of sockets to use to connect to
// multiple client/servers at once.  The pool grows when there are sockets that
// are needed but there are no free sockets.  All items in pool at end are deleted.
// This is done for effiecency so we don't keep creating and destroying TMsgSockets 
// each time we want to send a message to the observation server.
//
///////////////////////////////////////////////////////////////////////////////////

#include "SocketMgr.h"
#include <assert.h>

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////////
// Public Methods

SocketMgr::SocketMgr(const IPSocket::Address& theAddressR, const ConnectionTypeEnum theConnectionType) :
	mCriticalSection(), 
	mConnectionType(theConnectionType),
	mAddress(theAddressR),
	mFreeSocketQueueP(NULL),
	mTMsgSocketP(NULL),
	mLocalMonitorTMsgSocketP(NULL),
	mLastError(0),
	mFunctionPointerP(NULL),
	mDataP(NULL),
	mListenSocketDeleteList(),
	mSendSocketDeleteList(),
	mShuttingDown(false),
	mPersistentTCPStarted(false),
	mClientType(ResourcePublisher)
{
	if (mConnectionType == TransientTCP)
	{
		mFreeSocketQueueP = new SocketQueue();
	}
};

SocketMgr::~SocketMgr()
{
	mShuttingDown = true;
	if (mConnectionType == TransientTCP)
	{
		mCriticalSection.Enter();
		// Delete all sockets 
		SocketList::iterator aItr = mSendSocketDeleteList.begin();
		for (; aItr != mSendSocketDeleteList.end(); aItr++)
		{
			//(*aItr)->CloseRecv(); // Causes deadlock
			delete *aItr; // Actual socket is also deleted
		}
		if (mLocalMonitorTMsgSocketP != NULL )
		{
			// Delete all unclosed listening sockets
			for (aItr = mListenSocketDeleteList.begin(); aItr != mListenSocketDeleteList.end(); aItr++)
			{

				delete *aItr; // Actual socket is also deleted
			}
		}
		mCriticalSection.Leave();
	}
	else
	{
		//if (mTMsgSocketP != NULL)
		//	mTMsgSocketP->Close();
		delete mTMsgSocketP;
	}
	if (mLocalMonitorTMsgSocketP != NULL)
		mLocalMonitorTMsgSocketP->Close();

	delete mLocalMonitorTMsgSocketP;
	delete mFreeSocketQueueP;
}

TMsgSocket* 
SocketMgr::GetOpenSocket()
{
	TMsgSocket* aReturnSocketP = NULL;
	if (mConnectionType == TransientTCP)
	{
		mCriticalSection.Enter();
		if (mFreeSocketQueueP->size() == 0)
		{
			// Create new socket
			aReturnSocketP = CreateSendSocket();
			// Put socket in delete list
			mSendSocketDeleteList.push_back(aReturnSocketP);
		}
		else
		{
			// Get socket from pool
			aReturnSocketP = mFreeSocketQueueP->front();
			mFreeSocketQueueP->pop();
		}
		mCriticalSection.Leave();
		if ( (mLastError = ((IPSocket*)aReturnSocketP->GetSocket())->Open(&mAddress, -1, false) ) == Error_Success)
		{
			// Start recv loop
			aReturnSocketP->RecvBaseMsgEx((WONMsg::BaseMessage**)NULL, -1, true, mFunctionPointerP, mDataP);
		}
		else
		{
			mCriticalSection.Enter();
			SocketList::iterator aItr = find(mSendSocketDeleteList.begin(),mSendSocketDeleteList.end(),aReturnSocketP);
			if (aItr != mSendSocketDeleteList.end())
			{
				mSendSocketDeleteList.erase(aItr); // remove from delete list
			}
			mCriticalSection.Leave();

			delete aReturnSocketP;
			aReturnSocketP = NULL;
		}
	}
	else 
	{
		if ( mTMsgSocketP == NULL )
		{
			mTMsgSocketP = CreateSendSocket();
		}

		if (!((IPSocket*)mTMsgSocketP->GetSocket())->IsOpen())
		{
			if (mPersistentTCPStarted)
			{
				ReestablishPersistentTCP();
			}
			else // Open new one
			{
				if ( (mLastError = ((IPSocket*)mTMsgSocketP->GetSocket())->Open(&mAddress, -1, false) ) == Error_Success)
				{
					// Start recv loop
					mTMsgSocketP->RecvBaseMsgEx((WONMsg::BaseMessage**)NULL, -1, true, mFunctionPointerP, mDataP);
					// This now done manually
					//mPersistentTCPStarted = true;
				}
				else // Failed
				{
					delete mTMsgSocketP;
					mTMsgSocketP = NULL;
				}
			}
		}
		// Return the only socket
		aReturnSocketP = mTMsgSocketP;
	}

	return aReturnSocketP;
}

void
SocketMgr::FreeSocket(TMsgSocket* theSocket)
{
	if (mConnectionType == TransientTCP)
	{
		mCriticalSection.Enter();
		// Only keep track of the items in the pool that are not used.
		mFreeSocketQueueP->push(theSocket);
		mCriticalSection.Leave();
	}
}

void
SocketMgr::Listen( unsigned short thePort)
{
	switch (mConnectionType)
	{
		case PersistentTCP:
			mLocalMonitorTMsgSocketP = new TMsgSocket(new TCPSocket, true, 2); // Owns the actual socket...
		break;
	
		case TransientTCP:
			mLocalMonitorTMsgSocketP = new TMsgSocket(new TCPSocket, true, 2); // Owns the actual socket...
		break;

		case UDP:
			mLocalMonitorTMsgSocketP = new TMsgSocket(new UDPSocket, true, 2); // Owns the actual socket...
		break;

		default:
			assert(0);
		break;

	}

	//char msg[100];
	//sprintf(msg, "Listen Socket: %x\n", mLocalMonitorTMsgSocketP);
	//OutputDebugString(msg);
	//sprintf(msg, "Listen Actual Socket: %x\n", mLocalMonitorTMsgSocketP->GetSocket());
	//OutputDebugString(msg);

	if (mConnectionType == TransientTCP) // TCP
	{
		if ( ((TCPSocket*)mLocalMonitorTMsgSocketP->GetSocket())->Listen(thePort) == Error_Success )
		{
			// Need to do an asyncronous accept that accepts 
			((TCPSocket*)mLocalMonitorTMsgSocketP->GetSocket())->AcceptEx((TCPSocket*)NULL, -1, true, (void (*)(const TCPSocket::AcceptResult& result, SocketMgr* This)) TCPAccept, (SocketMgr*)this);
		}
	}
	else // UDP
	{
		((IPSocket*)mLocalMonitorTMsgSocketP->GetSocket())->Listen(thePort);

		// Start recv loop
		mLocalMonitorTMsgSocketP->RecvBaseMsgEx((WONMsg::BaseMessage**)NULL, -1, true, mFunctionPointerP, mDataP);
	}
}

// Send Titan message to Observation Server
Error 
SocketMgr::SendMMsgToServer(BaseMessage& theMsgR)
{
	Error aReturn;

	TMsgSocket* aTMsgSocketP = GetOpenSocket();
	if (aTMsgSocketP != NULL)
	{
		theMsgR.Pack();
		if (aTMsgSocketP->SendRawMsg(theMsgR.GetDataLen(), (void*)theMsgR.GetDataPtr()))
		{	
			// Set persistant tcp to connected if we send start publisher or subscribe messages
			//if (theMsgR.GetMessageType() == ObsMsg_AddPublisher || theMsgR.GetMessageType() == ObsMsg_SubscribeById || theMsgR.GetMessageType() == ObsMsg_SubscribeByName)
			//{
			//	mPersistentTCPStarted = true;
			//}
			//if (theMsgR.GetMessageType() == ObsMsg_RemovePublisher || theMsgR.GetMessageType() == ObsMsg_RemoveSubscriber )
			//{
			//	mPersistentTCPStarted = false;
			//}
			aReturn = Error_Success;
		}
		else
		{
			aReturn = Error_GeneralFailure;
		}
	}
	else
	{
		aReturn = GetLastError();
	}

	return aReturn;
}

bool
SocketMgr::ReestablishPersistentTCP()
{
	bool aSucceeded = false;

#ifdef _DEBUG
	OutputDebugString("ReestablishPersistentTCP");
#endif

	// Send message to observation server telling it you are reestablishing a Persistent TCP connection.
	MMsgObsReestablishPersistentTCP aMsg;
	aMsg.SetClientId(mClientId);
	aMsg.SetClientType(mClientType);
	
	MMsgObsStatusReply* aReplyP = NULL;
	if ( (mLastError = ((IPSocket*)mTMsgSocketP->GetSocket())->Open(&mAddress, -1, false) ) == Error_Success)
	{
		aMsg.Pack();
		mTMsgSocketP->SendRawMsg(aMsg.GetDataLen(), (void*)aMsg.GetDataPtr(), -1, true);

		// do one blocking recv
		aReplyP = (MMsgObsStatusReply*)mTMsgSocketP->RecvBaseMsg((WONMsg::BaseMessage**)NULL, 8000, false); // Wait 2 seconds
		aSucceeded = true;
	}
	
	if (aReplyP != NULL && aSucceeded)
	{
		MMsgObsStatusReply aReply = *aReplyP;

		//// Start new recv loop
		//mTMsgSocketP->RecvBaseMsg((WONMsg::BaseMessage**)NULL, -1, true, mFunctionPointerP, mDataP);
		
		try
		{
			aReply.Unpack();
			aSucceeded = true;
		}
		catch (WONMsg::BadMsgException&)
		{ 
		}

		if (aSucceeded && aReply.GetStatus() == StatusCommon_Success)
		{
			// Start recv loop
			mTMsgSocketP->RecvBaseMsgEx((WONMsg::BaseMessage**)NULL, -1, true, mFunctionPointerP, mDataP);
			aSucceeded = true;
		}
		else // Give up
		{
			delete mTMsgSocketP;
			mTMsgSocketP = NULL;
			mPersistentTCPStarted = false;
		}
	}
	else // Failed
	{
		delete mTMsgSocketP;
		mTMsgSocketP = NULL;
		mPersistentTCPStarted = false;
	}

	return aSucceeded;
}

///////////////////////////////////////////////////////////////////////////////////
// Private Methods

TMsgSocket* 
SocketMgr::CreateSendSocket()
{
	TMsgSocket* aTMsgSocketP = NULL;

	if (mConnectionType != UDP) // TCP
	{
		aTMsgSocketP = new TMsgSocket(new TCPSocket, true, 2); // Owns the actual socket...
		aTMsgSocketP->CatchClose(new CompletionWithContents<const TMsgSocket::Result&, SocketMgr*>(this, OnSocketClosed, true ), true ); // Stay for the life of TMsgSocket
	}
	else // UDP
	{
		aTMsgSocketP = new TMsgSocket(new UDPSocket, true, 2); // Owns the actual socket..
	}

	//char msg[100];
	//sprintf(msg, "Send Socket: %x\n", aTMsgSocketP);
	//OutputDebugString(msg);
	//sprintf(msg, "Send Actual Socket: %x\n", aTMsgSocketP->GetSocket());
	//OutputDebugString(msg);

	return aTMsgSocketP;
}


///////////////////////////////////////////////////////////////////////////////////
// Callback functions
///////////////////////////////////////////////////////////////////////////////////

// Called by socket level when Actual socket closes. 
void SocketMgr::OnSocketClosed(const TMsgSocket::Result& result, SocketMgr* This)
{
	// Add socket back to free socket pool
	if ( This->mConnectionType == TransientTCP )
	{
		if ( !This->mShuttingDown )
		{
			This->FreeSocket((TMsgSocket*)result.theSocket);
		}
	}
	// try to reconnect and recover
	else if ( This->mConnectionType == PersistentTCP )
	{
		// Reestablish persistant tcp removed
	}
	else // UDP - This should never happen
	{
		assert(0);
	}
}

void 
SocketMgr::TCPAccept(const TCPSocket::AcceptResult& result, SocketMgr* This)
{
	if ( result.acceptedSocket != NULL && This->mConnectionType == TransientTCP )
	{
		// Create new TMsgSocket
		TMsgSocket* aTMsgSocketP = new TMsgSocket(result.acceptedSocket, true, 2); // Owns TCP socket

		//char msg[100];
		//sprintf(msg, "Accept Socket: %x\n", aTMsgSocketP);
		//OutputDebugString(msg);
		//sprintf(msg, "Accept Actual Socket: %x\n", aTMsgSocketP->GetSocket());
		//OutputDebugString(msg);

		// Put socket in delete list
		This->mCriticalSection.Enter();
		This->mListenSocketDeleteList.push_back(aTMsgSocketP);
		This->mCriticalSection.Leave();

		// Start recv loop
		aTMsgSocketP->RecvBaseMsgEx((WONMsg::BaseMessage**)NULL, -1, true, This->mFunctionPointerP, This->mDataP);

		// Need to do this so we can delete socket when it closes
		aTMsgSocketP->CatchClose(new CompletionWithContents<const TMsgSocket::Result&, SocketMgr*>(This, OnAcceptedSocketClosed, true ), false ); 

		// Wait for the next one 
		((TCPSocket*)This->mLocalMonitorTMsgSocketP->GetSocket())->AcceptEx((TCPSocket*)NULL, -1, true, TCPAccept, This);
	}
}

void 
SocketMgr::OnAcceptedSocketClosed(const TMsgSocket::Result& result, SocketMgr* This)
{
	This->mCriticalSection.Enter();
	SocketList::iterator aItr = find(This->mListenSocketDeleteList.begin(),This->mListenSocketDeleteList.end(),result.theSocket);
	if (aItr != This->mListenSocketDeleteList.end())
	{
		This->mListenSocketDeleteList.erase(aItr); // remove from delete list
	}
	This->mCriticalSection.Leave();

	delete result.theSocket;
}



