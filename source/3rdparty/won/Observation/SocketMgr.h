#ifndef _SocketMgr_H
#define _SocketMgr_H

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
//
///////////////////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4786 )

#include <list>
#include <queue>
#include "Socket/WONWS.h" // This needs to be put in for now so that we will not get windows winsock.h
#include "common/CriticalSection.h"
#include "Socket/TMsgSocket.h"
#include "Socket/IPSocket.h"
#include "Socket/TCPSocket.h"
#include "Socket/UDPSocket.h"
#include "ObservationMessages.h"
#include "msg/Obs/ObsServerListElements.h"

using namespace std;
using namespace WONMsg;

namespace WONAPI {

class ObservationClientBase;

typedef queue<TMsgSocket*> SocketQueue;
typedef list<TMsgSocket*> SocketList;
typedef void (*RecvLoopFuncPtr)(const TMsgSocket::RecvBaseMsgResult& result, ObservationClientBase*);

///////////////////////////////////////////////////////////////////////////////////
// Socket Manager
///////////////////////////////////////////////////////////////////////////////////

class SocketMgr
{
	
public: 
	SocketMgr( const IPSocket::Address& theAddressR, const ConnectionTypeEnum theConnectionType);
	virtual ~SocketMgr();

	TMsgSocket* GetOpenSocket();
	void FreeSocket(TMsgSocket* theSocket);
	void Listen( unsigned short thePort);
	Error SendMMsgToServer(BaseMessage& theMsgR);
	bool ReestablishPersistentTCP();

	// Accessors
	Error GetLastError()
	{
		return mLastError;
	};

	void SetReceiveFunction(RecvLoopFuncPtr theFunctionPointerP, ObservationClientBase* theDataP)
	{
		mFunctionPointerP = theFunctionPointerP;
		mDataP = theDataP;
	}

	void SetClientId(unsigned long theClientId)
	{
		mClientId = theClientId;
		mPersistentTCPStarted = (mClientId != 0);
	}

	void SetShuttingDown(bool theShuttingDown)
	{
		mShuttingDown = theShuttingDown;
	}

	void SetClientType(ResourceTypeEnum	theClientType)
	{
		mClientType = theClientType;
	}

	// Callback functions
	static void OnSocketClosed(const TMsgSocket::Result& result, SocketMgr* This);
	static void TCPAccept(const TCPSocket::AcceptResult& result, SocketMgr* This);
	static void OnAcceptedSocketClosed(const TMsgSocket::Result& result, SocketMgr* This);

private:
	TMsgSocket* CreateSendSocket();

private:
	WONCommon::CriticalSection		mCriticalSection;
	ConnectionTypeEnum	mConnectionType;
	IPSocket::Address	mAddress;
	SocketQueue*		mFreeSocketQueueP; // Used for Transient TCP
	TMsgSocket*			mTMsgSocketP;	   // Only use one socket for Persistant TCP and UDP (A pool of one)
	TMsgSocket*			mLocalMonitorTMsgSocketP;
	Error				mLastError;
	SocketList			mListenSocketDeleteList;
	SocketList			mSendSocketDeleteList; // Only used for transient TCP

	bool				mShuttingDown; // Used when the socket manager is shutting down to keep if from attempting to open any connections
	bool				mPersistentTCPStarted;

	// Data for Receive function
	RecvLoopFuncPtr mFunctionPointerP;
	ObservationClientBase* mDataP;

	// Needed for Persistent TCP Reconnect
	unsigned long		mClientId;
	ResourceTypeEnum	mClientType;

};

}

#endif