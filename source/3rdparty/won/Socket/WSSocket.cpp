
#include "WSSocket.h"
#include <iostream>
#include "wondll.h"

using namespace std;

#ifdef _LINUX
#include <pthread.h>
#include <sys/time.h>
#elif defined(WIN32)
#include <process.h>
typedef int socklen_t;
#endif


using namespace WONAPI;
using namespace WONCommon;


#ifdef _MTAPI

#ifdef WIN32
DWORD WSSocket::spinThreadID = -1;
DWORD WSSocket::completionThreadID = -1;
HANDLE WSSocket::completionThreadHandle = NULL;
HANDLE WSSocket::spinThreadHandle = NULL;
#elif defined(_LINUX)
pthread_t WSSocket::spinThread;
pthread_t WSSocket::completionThread;
#endif

#endif



bool WSSocket::AutoConstructStatics::alloced = false;

bool WSSocket::selecting = false;
SOCKET WSSocket::interruptSocket = INVALID_SOCKET;
bool WSSocket::interrupting = false;
bool WSSocket::killSpinThread = false;
WSSocket::SocketOp* WSSocket::firstCompleted = 0;
WSSocket::SocketOp* WSSocket::lastCompleted = 0;

WSSocket::AutoConstructStatics autoConstructStatics;

// On static destruct, 'statics' will wait for spin thread to die before destructing other statics
WSSocket::Statics* WSSocket::statics;


// Zero-initialized static data is guaranteed to be initialized at statup,
// before static initialization of objects
static unsigned long selectRef = 0;
static unsigned long selectCount = 0;
static bool spinningThread = false;
static bool completionThreadStarted;
static WSSocket* completedOnSocket = 0;

static unsigned long allocateStaticsRef = 0;
static CriticalSection* allocateStaticsRefCrit = 0;

// Perform initialization on globals 
static void InitStatics()
{
	selectRef = 0;
	selectCount = 0;
	spinningThread = false;
	completedOnSocket = 0;
}

static bool DoAllocateStatics(bool inc)
{
	static CriticalSection tempCrit;
	tempCrit.Enter();
	if (!allocateStaticsRefCrit)
		allocateStaticsRefCrit = new CriticalSection;
	else if (!inc)
	{
		tempCrit.Leave();
		return false;
	}
	tempCrit.Leave();

	allocateStaticsRefCrit->Enter();
	
	if (!allocateStaticsRef++)
	{
		InitStatics();
		WSSocket::statics = new WSSocket::Statics;
	}

	allocateStaticsRefCrit->Leave();
	return true;
}


static bool DoDeallocateStatics()
{
	bool result = false;
	if (allocateStaticsRefCrit)
	{
		allocateStaticsRefCrit->Enter();

		if (!--allocateStaticsRef)
		{
			result = true;
			delete WSSocket::statics;
			allocateStaticsRefCrit->Leave();
			delete allocateStaticsRefCrit;
			allocateStaticsRefCrit = 0;
		}
		else
			allocateStaticsRefCrit->Leave();
	}
	return result;
}


void WONInitialize()
{
	DoAllocateStatics(true);
}


BOOL WONTerminate()
{
	return DoDeallocateStatics() ? TRUE : FALSE;
}


bool WSSocket::AllocateStatics(bool inc)
{
	return DoAllocateStatics(inc);
}


bool WSSocket::DeallocateStatics()
{
	return DoDeallocateStatics();
}


#ifdef WIN32

Winsock* WSSocket::GetWinsock()
{
	autoConstructStatics.DoAlloc();
	return &(statics->winsock);
}
#endif


// Compares timeouts, safe if GetTickCount() loops past 0
bool WSSocket::ltAsyncOp::operator()(const AsyncOp* asyncOp1, const AsyncOp* asyncOp2) const
{
	unsigned long now = GetTickCount();

	unsigned long t1 = now - asyncOp1->timeStarted;
	t1 = (t1 < asyncOp1->timeout) ? asyncOp1->timeout - t1 : 0;

	unsigned long t2 = now - asyncOp2->timeStarted;
	t2 = (t2 < asyncOp2->timeout) ? asyncOp2->timeout - t2 : 0;

	return t1 < t2;
}


WSSocket::WSSocket(const Address& destAddress, int theAddressFamily, int theType, int theProtocol, bool cnnctionless, bool closeSendOnRecvClosure, int recvBufSize, int sendBufSize)
	:	sock(INVALID_SOCKET),
		closeSendOnRecvClose(closeSendOnRecvClosure),
		addressFamily(theAddressFamily),
		type(theType),
		protocol(theProtocol),
		pendingRecvBytes(0),
		opening(false),
		closing(false),
		listening(false),
		recvClosing(false),
		sendClosing(false),
		recvClosed(true),
		sendClosed(true),
		closeCatcherOp(NULL),
		closeRecvCatcherOp(NULL),
		recvCatcherOp(NULL),
		acceptCatcherOp(NULL),
		numCompletesPending(0),
		completionTracker(true, true),
		connectionless(cnnctionless),
		recvCatcherInstalled(false),
		sendBufferSize(sendBufSize),
		recvBufferSize(recvBufSize),
		destAddr(destAddress),
		alreadyPreparedForDelete(false),
		recvCatcherQueued(false),
		acceptCatcherQueued(false),
		closeCatcherQueued(false),
		closeRecvCatcherQueued(false)
{
	autoConstructStatics.DoAlloc();

	sendPendingItor = statics->pendingSends.end();
	recvPendingItor = statics->pendingRecvs.end();
	openPendingItor = statics->pendingOpens.end();
}

/*void CheckMem();



void WSSocket::CheckCompleted()
{
	//CheckMem();
	SocketOp* cur = firstCompleted;
	while (cur)
	{
		WSSocket* obj = cur->obj;
		unsigned long pending = obj->numCompletesPending;
		unsigned long realPending = 0;
		
		SocketOp* tmp = firstCompleted;
		while (tmp)
		{
			if (tmp->obj == obj)
				realPending++;

			tmp = tmp->nextCompleted;
		}
		if (pending != realPending)
			pending = pending;

		cur = cur->nextCompleted;
	}
}
*/

void WSSocket::PrepareForDestruction()
{
	if (!alreadyPreparedForDelete)
	{
		AutoCrit autoCrit(statics->crit);

		alreadyPreparedForDelete = true;

		Close(0);	// force close

		CatchRecvable();

#ifdef _MTAPI

#ifdef WIN32
		DWORD curThreadID = GetCurrentThreadId();
		if (curThreadID == completionThreadID)
#elif defined(_LINUX)
		pthread_t thisThread = pthread_self();
		if (pthread_equal(thisThread, completionThread))
#endif
		{
			WSSocket* savedSocket = completedOnSocket;
			while (firstCompleted && numCompletesPending)
			{
				SocketOp* completedSocketOp = firstCompleted;
				firstCompleted = firstCompleted->nextCompleted;
				completedOnSocket = completedSocketOp->obj;
				completedOnSocket->numCompletesPending--;

//				CheckCompleted();

				autoCrit.Leave();
				completedSocketOp->Complete();
				autoCrit.Enter();

//				CheckCompleted();

				if (completedOnSocket && !completedOnSocket->numCompletesPending)
					completedOnSocket->completionTracker.Set();
			}
			completedOnSocket = savedSocket;
			if (completedOnSocket == this)
				completedOnSocket = 0;
//			CheckCompleted();
		}
		else if (numCompletesPending)
		{
			autoCrit.Leave();
			completionTracker.WaitFor();
		}
#else
		if (numCompletesPending)
			PumpUntil(completionTracker);
#endif
	}
}


WSSocket::~WSSocket()
{
	PrepareForDestruction();

	// Must have completed, since all ops on this socket have completed
	if (acceptCatcherOp)
	{
		if (acceptCatcherOp->triggered)	// Unless, of course, the catcher caused the delete
			acceptCatcherOp->autoDelete = true;
		else
			delete acceptCatcherOp;
	}
	if (closeCatcherOp)
	{
		if (closeCatcherOp->triggered)
			closeCatcherOp->autoDelete = true;
		else
			delete closeCatcherOp;
	}
	if (closeRecvCatcherOp)
	{
		if (closeRecvCatcherOp->triggered)
			closeRecvCatcherOp->autoDelete = true;
		else
			delete closeRecvCatcherOp;
	}
	if (recvCatcherOp)
	{
		if (recvCatcherOp->triggered)
			recvCatcherOp->autoDelete = true;
		else
			delete recvCatcherOp;
	}
}


WSSocket::SocketOp::~SocketOp()
{
}


void WSSocket::SocketOp::Trigger()
{
	if (deferCompletion)
	{
		if (obj)
		{
			obj->completionTracker.Reset();
			obj->numCompletesPending++;
		}

		if (firstCompleted)
			lastCompleted->nextCompleted = this;
		else
			firstCompleted = this;
		lastCompleted = this;
		nextCompleted = 0;
		statics->completionThreadEvent.Set();
	}
	triggered = true;
	doneEvent.Set();
}


void WSSocket::RecvCatcherOp::Trigger()
{
	deferCompletion = autoDelete || !catcherCompletion.empty();
	if (deferCompletion)
		obj->recvCatcherQueued = true;
	statics->pendingRecvs.erase(obj->recvPendingItor);
	obj->recvPendingItor = statics->pendingRecvs.end();	// debug
	obj->recvCatcherInstalled = false; // used to determine if socket needs to be added to select
	SocketOp::Trigger();
	if (!deferCompletion)
		triggered = false;
}


void WSSocket::AcceptCatcherOp::Trigger()
{
	deferCompletion = autoDelete || !catcherCompletion.empty();	
	// Must be the only thing on the recv queue...
	if (deferCompletion)
		obj->acceptCatcherQueued = true;
	statics->pendingRecvs.erase(obj->recvPendingItor);
	obj->recvPendingItor = statics->pendingRecvs.end();	// debug
	obj->recvCatcherInstalled = false; // used to determine if socket needs to be added to select
	SocketOp::Trigger();
	if (!deferCompletion)
		triggered = false;
}


void WSSocket::CatcherOp::Trigger()
{
	deferCompletion = autoDelete || !catcherCompletion.empty();
	SocketOp::Trigger();
}


void WSSocket::CloseCatcherOp::Trigger()
{
	deferCompletion = autoDelete || !catcherCompletion.empty();
	if (deferCompletion)
		obj->closeCatcherQueued = true;
	SocketOp::Trigger();
	if (!deferCompletion)
		triggered = false;
}

void WSSocket::CloseRecvCatcherOp::Trigger()
{
	deferCompletion = autoDelete || !catcherCompletion.empty();
	if (deferCompletion)
		obj->closeRecvCatcherQueued = true;
	SocketOp::Trigger();
	if (!deferCompletion)
		triggered = false;
}


void WSSocket::AsyncOp::Trigger()
{
	if (timeout >= 0)
		statics->timeoutSet.erase(timeoutItor);

	SocketOp::Trigger();
}

void WSSocket::OpenOp::Trigger()
{
	obj->opening = false;

	statics->pendingOpens.erase(obj->openPendingItor);
	obj->openPendingItor = statics->pendingOpens.end();	// debug

	if (error == Error_Success)
	{
		if (obj->recvCatcherOp)
		{
			obj->recvPendingItor = statics->pendingRecvs.insert(PendingMap::value_type(obj->sock, obj)).first;
			obj->recvCatcherInstalled = true;
		}
		else
			obj->recvCatcherInstalled = false;
	}
	else
	{
		obj->recvClosed = true;
		obj->CloseAllRecvs();

		obj->sendClosed = true;
		obj->CloseAllSends();

		WON_SOCKET_CALL(closesocket)(obj->sock);
		obj->sock = INVALID_SOCKET;
	}

	deferCompletion = autoDelete || !openCompletion.empty();
	AsyncOp::Trigger();
}

void WSSocket::AcceptOp::Trigger()
{
	deferCompletion = autoDelete || !acceptCompletion.empty();
	obj->recvQueue.erase(queueItor);
	if (obj->recvQueue.empty())
	{
		if (obj->acceptCatcherOp)
			obj->recvCatcherInstalled = true;
		else
		{
			statics->pendingRecvs.erase(obj->recvPendingItor);
			obj->recvPendingItor = statics->pendingRecvs.end();	// debug
		}
	}
	AsyncOp::Trigger();
}

void WSSocket::SendOp::Trigger()
{
	deferCompletion = autoDelete || !recvSendCompletion.empty();
	obj->sendQueue.erase(queueItor);
	if (obj->sendQueue.empty())
	{
		statics->pendingSends.erase(obj->sendPendingItor);
		obj->sendPendingItor = statics->pendingSends.end();	// debug
	}
	if (ownsBuffer)
	{
		delete (char*)actualBuffer;
		buffer = 0;
		actualBuffer = 0;
		ownsBuffer = false;
	}
	AsyncOp::Trigger();
}

void WSSocket::RecvOp::Trigger()
{
	deferCompletion = autoDelete || !recvSendCompletion.empty();
	obj->pendingRecvBytes -= (bufSize - bufUsed);
	obj->recvQueue.erase(queueItor);

	if (obj->recvQueue.empty())
	{
		if (!(obj->closeQueue.empty()) && obj->sendClosed)
			obj->SocketClosed();	//?
		else
		{
			if (!obj->recvCatcherOp || obj->recvCatcherQueued)
			{
				statics->pendingRecvs.erase(obj->recvPendingItor);
				obj->recvPendingItor = statics->pendingRecvs.end();	// debug
			}
			else
			{
				obj->recvCatcherInstalled = true;
				if (!obj->recvCatcherQueued && obj->Available())
				{
					RecvCatcherOp* catcher = obj->recvCatcherOp;
					if (catcher->autoDelete)
						obj->recvCatcherOp = 0;
					catcher->Trigger();
				}
			}
		}
	}
	AsyncOp::Trigger();
}

void WSSocket::CloseOp::Trigger()
{
	deferCompletion = autoDelete || !closeCompletion.empty();
	obj->closeQueue.erase(queueItor);
	AsyncOp::Trigger();
}

void WSSocket::CloseSendOp::Trigger()
{
	deferCompletion = autoDelete || !closeCompletion.empty();
	obj->sendQueue.erase(queueItor);

	AsyncOp::Trigger();
}

void WSSocket::CloseRecvOp::Trigger()
{
	deferCompletion = autoDelete || !closeCompletion.empty();
	obj->recvQueue.erase(queueItor);
	// recvQueue will be emptied, since the recv channel is closed
	AsyncOp::Trigger();
}


void WSSocket::SocketOp::Complete()
{
	triggered = false;

	if (autoDelete)
		delete this;
}


void WSSocket::TimerOp::Complete()
{
	timerCompletion.Complete();
	SocketOp::Complete();
}


// Note: If the connection closes, the closeCatcher WILL complete before an open could possible complete
// So, there should never be an attempt to trigger a triggered-uncompleted catcher
void WSSocket::CatcherOp::Complete()
{
	if (!catcherCompletion.empty())
		catcherCompletion.Complete(Result(obj));

	AutoCrit autoCrit(crit);
	// The CatchXxxx() function depends on the 'triggered' variable
	// never being set outside of a crit.  

	SocketOp::Complete();
}

void WSSocket::OpenOp::Complete()
{
	openCompletion.Complete(OpenResult(obj, error));
	SocketOp::Complete();
}

void WSSocket::AcceptOp::Complete()
{
	acceptCompletion.Complete(AcceptResult(obj, resultSocket));
	SocketOp::Complete();
}

void WSSocket::TransmitOp::Complete()
{
	recvSendCompletion.Complete(TransmitResult(obj, buffer, bufSize, bufUsed, closed));

	if (ownsBuffer)
		delete (char*)buffer;	// Should only get deleted from RecvOp, SendOp uses actualBuffer
	SocketOp::Complete();
}

void WSSocket::CloseOp::Complete()
{
	closeCompletion.Complete(Result(obj));
	SocketOp::Complete();
}


void WSSocket::RecvCatcherOp::Complete()
{
	obj->recvCatcherQueued = false;
	CatcherOp::Complete();
}


void WSSocket::AcceptCatcherOp::Complete()
{
	obj->acceptCatcherQueued = false;
	CatcherOp::Complete();
}


void WSSocket::CloseCatcherOp::Complete()
{
	obj->closeCatcherQueued = false;
	CatcherOp::Complete();
}


void WSSocket::CloseRecvCatcherOp::Complete()
{
	obj->closeRecvCatcherQueued = false;
	CatcherOp::Complete();
}


void WSSocket::CloseAllSends()
{
	if (IsSending())
	{
		selectRef++;	// Invalidate any Spin()
		
		OpList::iterator sendQueueItor = sendQueue.begin();
		while (sendQueueItor != sendQueue.end())
		{
			SocketOp* thisOp = (*sendQueueItor);

			thisOp->closed = true;
			thisOp->Trigger();

			sendQueueItor = sendQueue.begin();
		}
	}
}

	
void WSSocket::CloseAllRecvs()
{
	if (IsRecving())
	{
		selectRef++;	// Invalidate any Spin()

		OpList::iterator recvQueueItor = recvQueue.begin();
		while (recvQueueItor != recvQueue.end())
		{
			SocketOp* thisOp = (*recvQueueItor);
			
			thisOp->closed = true;
			thisOp->Trigger();

			recvQueueItor = recvQueue.begin();
		}
	}
}


// Clean up, and/or close, after a connection drop
void WSSocket::SocketClosed()
{
	if (sock != INVALID_SOCKET)	// prevent SocketClosed() from being called twice
	{
		if (!recvClosed)
		{
			// Recv channel can't possibly close a second time before the first catcher completes
			recvClosed = true;
			CloseAllRecvs();

			if (closeRecvCatcherOp && !closeRecvCatcherQueued)	// No need to worry about the catcher crit, we've got the static crit
			{
				CatcherOp* catcherOp = closeRecvCatcherOp;
				if (closeRecvCatcherOp->autoDelete)
					closeRecvCatcherOp = 0;
				catcherOp->Trigger();
			}
		}
		if (!sendClosed)
		{
			sendClosed = true;
			CloseAllSends();
		}

		if (opening)
		{
			pendingOpenOp->error = Error_ConnectionClosed;
			pendingOpenOp->Trigger();
			// Don't bother telling people it closed if it didn't open
		}
		else
		{
			WON_SOCKET_CALL(closesocket)(sock);
			sock = INVALID_SOCKET;

			if (closeCatcherOp && !closeCatcherQueued)
			{
				CatcherOp* theCloseCatcherOp = closeCatcherOp;
				if (closeCatcherOp->autoDelete)
					closeCatcherOp = 0;
				theCloseCatcherOp->Trigger();
			}
		}

		if (closing)
		{
			closing = false;

			OpList::iterator closeQueueItor = closeQueue.begin();
			while (closeQueueItor != closeQueue.end())
			{
				SocketOp* thisOp = (*closeQueueItor);						
				thisOp->Trigger();
				closeQueueItor = closeQueue.begin();
			}
		}
	}
}


void WSSocket::SendChannelClosed()
{
	if (!sendClosed)
	{
		if (recvClosed || (recvQueue.empty() && !(closeQueue.empty())))
			SocketClosed();
		else
		{
			sendClosed = true;
			CloseAllSends();
		}
	}
}


void WSSocket::RecvChannelClosed()
{
	if (!recvClosed)
	{
		// Recv channel can't possible close a second time before the first catcher completes

		if (sendClosed || closeSendOnRecvClose)
			SocketClosed();
		else
		{
			recvClosed = true;
			CloseAllRecvs();

			if (closeRecvCatcherOp && !closeRecvCatcherQueued)	// No need to worry about the catcher crit, we've got the static crit
			{
				CatcherOp* catcherOp = closeRecvCatcherOp;
				if (closeRecvCatcherOp->autoDelete)
					closeRecvCatcherOp = 0;
				catcherOp->Trigger();
			}
		}

	}
}


bool WSSocket::IsSendOpen()
{
	AutoCrit autoCrit(statics->crit);

	if (sock == INVALID_SOCKET || opening)
		return false;

	return !sendClosed;
}


bool WSSocket::IsRecvOpen()
{
	AutoCrit autoCrit(statics->crit);

	if (sock == INVALID_SOCKET || opening)
		return false;

	if (!recvClosed)
	{
		if (GetType() == datagram)
			return true;

#if defined(macintosh) && (macintosh == 1)
		return true;	// MSG_PEEK isn't yet implemented by GUSI   :/
#else
		char buf;	// single byte buffer, for peek
		int i = WON_SOCKET_CALL(recv)(sock, &buf, 1, MSG_PEEK);
		
		if (i == SOCKET_ERROR)
		{
			int err = WON_SOCKET_CALL(WSAGetLastError)();
			switch (err)
			{
			case WSAESHUTDOWN:			// recv is shut down!
				i = 0;
				break;
			// These cases might imply that the socket is still open
			case WSAEINPROGRESS:		// inconclusive... No idea why this would happen
			case WSAENOTCONN:	//?		// Might be a perfectly valid addressless datagram socket
			case WSAEWOULDBLOCK:		// implies recv channel is open
			case WSAEMSGSIZE:			// implies recv channel is open, and UDP msg is >1 byte
				return true;
			// case WSAENOTSOCK			// cannot happen, unless we've lost the socket somehow
			// case WSAEOPNOTSUPP		// cannot happen, socket setup issue
			// case WSAENETRESET		// cannot be open
			// case WSANOTINITIALISED	// cannot be open
			// case WSAENETDOWN			// cannot be open)
			// case WSAEFAULT			// can't happen, our buf is right here
			// case WSAEINTR			// cannot happen, we're non-blocking
			// case WSAEINVAL			// funky... no idea why we'd ever see this error
			// case WSAECONNABORTED		// socket hosed, needs to be closed
			// case WSAETIMEDOUT		// socket hosed, needs to be closed
			// case WSAECONNRESET		// hard or abortive close, close socket
			default: // Unexpected errors indicate socket is completely closed
				SocketClosed();
				InterruptCompletionThread();	// trigger completions
				return false;
			}
		}
		if (i == 0)
		{
			RecvChannelClosed();
			InterruptCompletionThread();
		}
#endif
	}

	return !recvClosed;
}


bool WSSocket::IsOpen()
{
	AutoCrit autoCrit(statics->crit);

	if (sock == INVALID_SOCKET || opening)
		return false;

	// Check send channel first
	if (IsSendOpen())
		return true;

	if (IsRecvOpen())
		return true;
	
	// Invalidate socket
	SocketClosed();
	InterruptCompletionThread();

	return false;
}

Error WSSocket::Open(const Address* addrParam, long timeout, bool async, const CompletionContainer<const OpenResult&>& completion)
{
	const Address* addr = addrParam ? addrParam : &destAddr;

	Error error = Error_InvalidState;

	AutoCrit autoCrit(statics->crit);

	if ((addr == &destAddr || *addr == destAddr) && sock != INVALID_SOCKET)
		error = Error_Success;	// Implicit success
#ifdef WIN32
	else if (type == SOCK_RAW && WON_SOCKET_CALL(GetVersion)() & 0x00FF < 0x0002)
		error = Error_Winsock2Required;
#endif
	else if (!ValidateInterruptSocket())
		error = Error_NetFailed;
	else if (sock == INVALID_SOCKET)
		//  Open can only occur when the sock is closed and invalid
	{
		error = Error_NetResourceUnavailable;
		// Create socket
		sock = WON_SOCKET_CALL(socket)(addressFamily, type, protocol);
		if (sock != INVALID_SOCKET)
		{
			error = Error_GeneralFailure;
			// Make socket non-blocking
#ifdef WIN32
			unsigned long argp = 1;
			int i = WON_SOCKET_CALL(ioctlsocket)(sock, FIONBIO, &argp);
#else//if defined(_LINUX)
			int i = fcntl(sock, F_SETFL, O_NONBLOCK);
#endif
			if (i == SOCKET_ERROR)
			{
				//int err = WON_SOCKET_CALL(WSAGetLastError)();
				//switch (err)
				//{
				//case WSANOTINITIALISED:		// shouldn't happen
				//case WSAENETDOWN:			// net is hosed
				//case WSAEINPROGRESS:			// no idea why that would happen
				//case WSAENOTSOCK:			// shouldn't happen
				//case WSAEFAULT:				// shouldn't happen




				//default:
					WON_SOCKET_CALL(closesocket)(sock);// Leave sock invalid on failure
					sock = INVALID_SOCKET;
				//	break;
				//}


			}
			else
			{
				i = WON_SOCKET_CALL(setsockopt)(sock, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, sizeof(int)); 
				if (i == SOCKET_ERROR)
				{
					//int err = WON_SOCKET_CALL(WSAGetLastError)();
					//switch (err)
					//{
					//case WSANOTINITIALISED:// A successful WSAStartup must occur before using this function. 
					//case WSAENETDOWN:// The network subsystem has failed. 
					//case WSAEFAULT:// optval is not in a valid part of the process address space or optlen parameter is too small. 
					//case WSAEINPROGRESS:// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
					//case WSAEINVAL:// level is not valid, or the information in optval is not valid. 
					//case WSAENETRESET:// Connection has timed out when SO_KEEPALIVE is set. 
					//case WSAENOPROTOOPT:// The option is unknown or unsupported for the specified provider. 
					//case WSAENOTCONN:// Connection has been reset when SO_KEEPALIVE is set. 
					//case WSAENOTSOCK:// The descriptor is not a socket. 
					//default:
					//	break;
					//}
					WON_SOCKET_CALL(closesocket)(sock);// Leave sock invalid on failure
					sock = INVALID_SOCKET;
				}
				else
				{
					i = WON_SOCKET_CALL(setsockopt)(sock, SOL_SOCKET, SO_RCVBUF, (char*)&recvBufferSize, sizeof(int)); 




					if (i == SOCKET_ERROR)
					{
						//int err = WON_SOCKET_CALL(WSAGetLastError)();
						//switch (err)
						//{
						//case WSANOTINITIALISED:// A successful WSAStartup must occur before using this function. 
						//case WSAENETDOWN:// The network subsystem has failed. 
						//case WSAEFAULT:// optval is not in a valid part of the process address space or optlen parameter is too small. 
						//case WSAEINPROGRESS:// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
						//case WSAEINVAL:// level is not valid, or the information in optval is not valid. 
						//case WSAENETRESET:// Connection has timed out when SO_KEEPALIVE is set. 
						//case WSAENOPROTOOPT:// The option is unknown or unsupported for the specified provider. 
						//case WSAENOTCONN:// Connection has been reset when SO_KEEPALIVE is set. 
						//case WSAENOTSOCK:// The descriptor is not a socket. 
						//default:
						//	break;
						//}
						WON_SOCKET_CALL(closesocket)(sock);// Leave sock invalid on failure
						sock = INVALID_SOCKET;
					}
					else
					{
						int enableBroadcast = 1;
						i = WON_SOCKET_CALL(setsockopt)(sock, SOL_SOCKET, SO_BROADCAST, (char*)&enableBroadcast, sizeof(int)); 
						if (i == SOCKET_ERROR && type == SOCK_DGRAM)
						{
							//int err = WON_SOCKET_CALL(WSAGetLastError)();
							//switch (err)
							//{
							//case WSANOTINITIALISED:// A successful WSAStartup must occur before using this function. 
							//case WSAENETDOWN:// The network subsystem has failed. 
							//case WSAEFAULT:// optval is not in a valid part of the process address space or optlen parameter is too small. 
							//case WSAEINPROGRESS:// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
							//case WSAEINVAL:// level is not valid, or the information in optval is not valid. 
							//case WSAENETRESET:// Connection has timed out when SO_KEEPALIVE is set. 
							//case WSAENOPROTOOPT:// The option is unknown or unsupported for the specified provider. 
							//case WSAENOTCONN:// Connection has been reset when SO_KEEPALIVE is set. 
							//case WSAENOTSOCK:// The descriptor is not a socket. 
							//default:


							//	break;
							//}
							WON_SOCKET_CALL(closesocket)(sock);// Leave sock invalid on failure
							sock = INVALID_SOCKET;
						}
						else
						{
							//i = 0;
							//if (sendQueue.empty()) // to be continued; add support for delaying re-open until after sends complete
							{
								destAddr = *addr;
								i = WON_SOCKET_CALL(connect)(sock, &(addr->Get()), sizeof(SOCKADDR));
								if (i == SOCKET_ERROR)
								{
									int err = WON_SOCKET_CALL(WSAGetLastError)();
									switch (err)
									{
									case WSAEWOULDBLOCK:		// expected
#ifndef WIN32           
					                case EINPROGRESS:
#endif

										i = 0;
										break;
									case WSAENOBUFS:			// open fails... out of resources/memory
										error = Error_NetResourceUnavailable;
										break;
									case WSAEAFNOSUPPORT:		// open fails... bad address
									case WSAEFAULT:				// open fails... bad address
									case WSAEINVAL:				// open fails... bad address, probably
										error = Error_BadAddress;
										break;
									case WSAEADDRNOTAVAIL:		// open fails... Simple enough
									case WSAENETUNREACH:		// open fails... bad address(?)
									case WSAECONNREFUSED:		// open fails... Simple ehough
										error = Error_HostUnreachable;
										break;
									//case WSAEADDRINUSE:		// no idea... Is OUR address in use?
									//case WSAEINPROGRESS:		// open fails... No idea why this would happen
									//case WSAEINTR:			// can't happen, not blocking
									//case WSAEALREADY;			// can't happen, we just created this socket
									//case WSAENOTSOCK:			// can't happen, we just create this socket, fresh!
									//case WSAEACCES:			// shouldn't happen, internal socket setup issue
									//case WSAETIMEDOUT:		// shouldn't happen, sock is non-blocking
									//case WSAEISCONN:			// can't happen, we just created this socket
									//----
									//case WSANOTINITIALISED:	// open fails
									//case WSAENETDOWN:		// open fails
									default:
										error = Error_NetFailed;
										// fall through, and invalidate socket
										break;
									}
								}
								else
								{
									error = Error_Success; // Already opened
									sendClosed = false;
									recvClosed = false;
								}
							}
							sockaddr saddr;
							if (!i)
							{
								socklen_t addrSize = sizeof(sockaddr);
								i = WON_SOCKET_CALL(getsockname)(sock, &saddr, &addrSize);
								//if (i == SOCKET_ERROR)
								//{
									// Fall through to a failure...
									//int err = WON_SOCKET_CALL(WSAGetLastError)();
									//switch (err)
									//{
									//case WSANOTINITIALISED:		// shouldn't happen
									//case WSAENETDOWN:				// net is hosed
									//case WSAEINPROGRESS:			// no idea why that would happen
									//case WSAENOTSOCK:				// shouldn't happen
									//case WSAEFAULT:				// shouldn't happen
									//default:
									//}
								//}
								if (!i)
									ourAddr.Set(saddr);

								if (error != Error_Success)
								{
									OpenOp* openOp = new OpenOp(*this);
									if (openOp)
									{
										auto_ptr<OpenOp> auto_SockOp(openOp);

										opening = true;								// Used to block other operations
										sendClosed = false;
										recvClosed = false;

										openOp->timeStarted = GetTickCount();
										openOp->timeout = timeout;
										openOp->openCompletion = async ? completion : CompletionContainer<const OpenResult&>() ;
										openOp->autoDelete = async;
										
										if (timeout >= 0)
											openOp->timeoutItor = statics->timeoutSet.insert((AsyncOp*)openOp);

										pendingOpenOp = openOp;
										openPendingItor = statics->pendingOpens.insert(PendingMap::value_type(sock, this)).first;

										if (!async)
										{
											autoCrit.Leave();
											//SpinOrInterruptSpinThread(timeout, openOp);
											InterruptSpin();
											PumpUntil(openOp->doneEvent, timeout);
											error = openOp->error;
											completion.Complete(OpenResult(this, error));
											return error;
										}
										InterruptSpin();
										auto_SockOp.release();
										return Error_Pending;
									}
									// else !openOp  // fall through, and invalidate socket
									// Leave sock invalid on failure
									WON_SOCKET_CALL(closesocket)(sock);
									sock = INVALID_SOCKET;
								}
							}
							else
							{
								// Leave sock invalid on failure
								WON_SOCKET_CALL(closesocket)(sock);
								sock = INVALID_SOCKET;
							}
						}
					}
				}
			}
		}
	}
	autoCrit.Leave();
	completion.Complete(OpenResult(this, error));
	return error;
}


Error WSSocket::Listen(const Address& addr)
{
	Error listenError = Error_InvalidState;

	AutoCrit autoCrit(statics->crit);

#ifdef WIN32
	if (type == SOCK_RAW && WON_SOCKET_CALL(GetVersion)() & 0x00FF < 0x0002)
		listenError = Error_Winsock2Required;
	else
#endif
	if (!ValidateInterruptSocket())
		listenError = Error_NetFailed;
	else if (sock == INVALID_SOCKET)
	{
		// Like Open, Listen can only occur when the socket is invalid
		listenError = Error_NetResourceUnavailable;
		// Create socket
		sock = WON_SOCKET_CALL(socket)(addressFamily, type, protocol);
		if (sock != INVALID_SOCKET)
		{
			listenError = Error_GeneralFailure;
			// Make socket non-blocking
#ifdef WIN32
			unsigned long argp = 1;
			int i = WON_SOCKET_CALL(ioctlsocket)(sock, FIONBIO, &argp);
#else//if defined(_LINUX)
			int i = fcntl(sock, F_SETFL, O_NONBLOCK);
#endif
			if (i == SOCKET_ERROR)
			{
				//int err = WON_SOCKET_CALL(WSAGetLastError)();
				//switch (err)
				//{
				//case WSANOTINITIALISED:	// shouldn't happen
				//case WSAENETDOWN:			// net is hosed
				//case WSAEINPROGRESS:		// no idea why that would happen
				//case WSAENOTSOCK:			// shouldn't happen
				//case WSAEFAULT:			// shouldn't happen
				//default:
				//	break;
				//}
				WON_SOCKET_CALL(closesocket)(sock);// Leave sock invalid on failure
				sock = INVALID_SOCKET;
			}
			else
			{
				int enableBroadcast = 1;
				i = WON_SOCKET_CALL(setsockopt)(sock, SOL_SOCKET, SO_BROADCAST, (char*)&enableBroadcast, sizeof(int)); 
				if (i == SOCKET_ERROR && type == SOCK_DGRAM)
				{
					int err = WON_SOCKET_CALL(WSAGetLastError)();
					//switch (err)
					//{
					//case WSANOTINITIALISED:// A successful WSAStartup must occur before using this function. 
					//case WSAENETDOWN:// The network subsystem has failed. 
					//case WSAEFAULT:// optval is not in a valid part of the process address space or optlen parameter is too small. 
					//case WSAEINPROGRESS:// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
					//case WSAEINVAL:// level is not valid, or the information in optval is not valid. 
					//case WSAENETRESET:// Connection has timed out when SO_KEEPALIVE is set. 
					//case WSAENOPROTOOPT:// The option is unknown or unsupported for the specified provider. 
					//case WSAENOTCONN:// Connection has been reset when SO_KEEPALIVE is set. 
					//case WSAENOTSOCK:// The descriptor is not a socket. 
					//default:
					//	break;
					//}
					WON_SOCKET_CALL(closesocket)(sock);// Leave sock invalid on failure
					sock = INVALID_SOCKET;
				}
				else
				{
					i = WON_SOCKET_CALL(bind)(sock, &(addr.Get()), sizeof(SOCKADDR));
					if (i == SOCKET_ERROR)
					{
						int err = WON_SOCKET_CALL(WSAGetLastError)();
						switch (err)
						{
						case WSAEFAULT:				// bad address?
						case WSAEADDRINUSE:			// Someone's already using that port?
							listenError = Error_AddressInUse;
							break;
						case WSAENOBUFS:			// Too many connections, out of resources!
							listenError = Error_NetResourceUnavailable;
							break;
						//case WSANOTINITIALISED:	// shouldn't happen
						//case WSAENETDOWN:			// net is hosed
						//case WSAEINPROGRESS:		// no idea why that would happen
						//case WSAENOTSOCK:			// shouldn't happen
						//case WSAEINVAL:			// Already bound
						default:
							break;
						}
						WON_SOCKET_CALL(closesocket)(sock);// Leave sock invalid on failure
						sock = INVALID_SOCKET;
					}
					else
					{
						sockaddr saddr;
						socklen_t addrSize = sizeof(sockaddr_in);
						i = WON_SOCKET_CALL(getsockname)(sock, &saddr, &addrSize);
						if (i == SOCKET_ERROR)
						{
							//int err = WON_SOCKET_CALL(WSAGetLastError)();
							//switch (err)
							//{
							//case WSANOTINITIALISED:		// shouldn't happen
							//case WSAENETDOWN:				// net is hosed
							//case WSAEINPROGRESS:			// no idea why that would happen
							//case WSAENOTSOCK:				// shouldn't happen
							//case WSAEFAULT:				// shouldn't happen
							//default:
								WON_SOCKET_CALL(closesocket)(sock);// Leave sock invalid on failure
								sock = INVALID_SOCKET;
							//}
						}
						else
						{
							ourAddr.Set(saddr);
							if (connectionless)
							{
								listenError = Error_Success;
								listening = true;
								sendClosed = false;
								recvClosed = false;

								if (recvCatcherOp)
								{
									recvCatcherInstalled = true;
									recvPendingItor = statics->pendingRecvs.insert(PendingMap::value_type(sock, this)).first;
									InterruptSpin();
								}
							}
							else
							{
								i = WON_SOCKET_CALL(listen)(sock, SOMAXCONN);
								if (i == SOCKET_ERROR)
								{
									int err = WON_SOCKET_CALL(WSAGetLastError)();
									switch (err)
									{
									case WSAENOBUFS:// No buffer space is available. 
									case WSAEMFILE:// No more socket descriptors are available. 
										listenError = Error_NetResourceUnavailable;
										break;
									case WSAEADDRINUSE:// An attempt has been made to listen on an address in use. 
										listenError = Error_AddressInUse;
										break;
									case WSAEOPNOTSUPP:// The referenced socket is not of a type that supports the listen operation. 
										listenError = Error_InvalidState;
									//case WSANOTINITIALISED:// A successful WSAStartup must occur before using this function. 
									//case WSAENETDOWN:// The network subsystem has failed. 
									//case WSAEINPROGRESS:// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
									//case WSAEINVAL:// The socket has not been bound with bind. 
									//case WSAEISCONN:// The socket is already connected. 
									//case WSAENOTSOCK:// The descriptor is not a socket. 
									default:
										break;
									}
									WON_SOCKET_CALL(closesocket)(sock);// Leave sock invalid on failure
									sock = INVALID_SOCKET;
								}
								else
								{
									listenError = Error_Success;

									listening = true;

									if (acceptCatcherOp)
									{
										recvCatcherInstalled = true;
										recvPendingItor = statics->pendingRecvs.insert(PendingMap::value_type(sock, this)).first;
									}
									else
										recvCatcherInstalled = false;

									InterruptSpin();
								}
							}
						}
					}
				}
			}
		}
	}
	return listenError;
}


WSSocket* WSSocket::Accept(WSSocket& socketToConnect, long timeout, bool async, const CompletionContainer<const AcceptResult&>& completion)
{
	if (!ValidateInterruptSocket())
	{
		completion.Complete(AcceptResult(this, 0));
		return 0;
	}
	if (!connectionless)
	{
		socketToConnect.Close();

		AutoCrit autoCrit(statics->crit);

		if (listening)
		{
			AcceptOp* acceptOp = new AcceptOp(*this);
			if (acceptOp)
			{
				auto_ptr<AcceptOp> auto_SockOp(acceptOp);

				acceptOp->timeStarted = GetTickCount();
				acceptOp->timeout = timeout;
				acceptOp->acceptCompletion = async ? completion : CompletionContainer<const AcceptResult&>() ;
				acceptOp->autoDelete = async;
				acceptOp->acceptToSocket = &socketToConnect;
				acceptOp->resultSocket = 0;
				
				if (timeout >= 0)
					acceptOp->timeoutItor = statics->timeoutSet.insert(acceptOp);

				// cases :
				// No accept catcher & no other accepts - add socket
				// Accept catcher, but it triggered, and no other accepts - add socket

				// No accept catcher, and other accepts - don't bother
				// Accept catcher, waiting w/others - don't bother
				// Accept catcher, waiting wo/others- don't bother
				// Accept catcher, triggered, but other accepts - don't bother
				if (!IsAccepting() && !recvCatcherInstalled)
					recvPendingItor = statics->pendingRecvs.insert(PendingMap::value_type(sock, this)).first;
				acceptOp->queueItor = recvQueue.insert(recvQueue.end(), acceptOp);
				if (!async)
				{
					autoCrit.Leave();
					//SpinOrInterruptSpinThread(timeout, acceptOp);
					InterruptSpin();
					PumpUntil(acceptOp->doneEvent, timeout);
					WSSocket* resultSocket = acceptOp->resultSocket;
					completion.Complete(AcceptResult(this, resultSocket));
					return resultSocket;
				}
				InterruptSpin();
				auto_SockOp.release();
				return 0;
			}
		}
		autoCrit.Leave();
	}
	completion.Complete(AcceptResult(this, 0));
	return 0;
}


void WSSocket::CloseSend(long timeout, bool async, const CompletionContainer<const Result&>& completion)
{
	if (!ValidateInterruptSocket())
	{
		completion.Complete(Result(this));
		return;
	}

	AutoCrit autoCrit(statics->crit);

	if (opening || IsSendOpen())
	{
		if (opening || IsSending())
		{
			if (timeout != 0)	// Graceful close, possibly with a timeout until forceful close
			{
				unsigned long now = GetTickCount();

				if (sendClosing)	// Existing close, update timeout, and post our own
				{
					if (pendingCloseSendOp->timeout > 0)
						sendQueue.erase(pendingCloseSendOp->queueItor);
					pendingCloseSendOp->timeStarted = now;
					pendingCloseSendOp->timeout = timeout;
					if (timeout > 0)
						pendingCloseSendOp->timeoutItor = statics->timeoutSet.insert(pendingCloseSendOp);
				}
				
				CloseSendOp* closeSendOp = new CloseSendOp(*this);
				if (closeSendOp)
				{
					auto_ptr<CloseSendOp> auto_SockOp(closeSendOp);
					
					closeSendOp->timeStarted = now;
					closeSendOp->closeCompletion = async ? completion : CompletionContainer<const Result&>() ;
					closeSendOp->autoDelete = async;
					closeSendOp->timeout = timeout;

					if (!sendClosing)
					{
						// Only add this one to timeout list, etc., if there's not already one there
						pendingCloseSendOp = closeSendOp;
						if (timeout > 0)
							closeSendOp->timeoutItor = statics->timeoutSet.insert(closeSendOp);
					}
					sendClosing = true;

					// Put in queue
					closeSendOp->queueItor = sendQueue.insert(sendQueue.end(), closeSendOp);

					if (!async)
					{
						autoCrit.Leave();
						//SpinOrInterruptSpinThread(timeout, closeSendOp);
						InterruptSpin();
						PumpUntil(closeSendOp->doneEvent, timeout);
						completion.Complete(Result(this));
						return;
					}
					InterruptSpin();
					auto_SockOp.release();
					return;
				}
				// else // fall through to synchronous close, if unable to allocate socketOp
			}
			// else if (timeout == 0) // forceful close
		}
		// else if (!IsSending()) // No pending data anyway, close it now!
		int i = WON_SOCKET_CALL(shutdown)(sock, SD_SEND);
		if (i == SOCKET_ERROR)
		{
			int err = WON_SOCKET_CALL(WSAGetLastError)();
			switch (err)
			{
			case WSAENOTCONN:			// not connected(?), might imply send channel is already closed
				break;
			// case WSANOTINITIALISED:	// shouldn't happen
			// case WSAENETDOWN:		// net hosed
			// case WSAEINVAL:			// wrong type of socket(?)
			// case WSAEINPROGRESS:		// no idea
			// case WSAENOTSOCK:		// bad socket
			default:
				SocketClosed();	// Socket must be hosed, close it completely
				break;
			}
		}
		SendChannelClosed();
		InterruptCompletionThread();
	}
	autoCrit.Leave();
	completion.Complete(Result(this));
}


void WSSocket::CloseRecv(long timeout, bool async, const CompletionContainer<const Result&>& completion)
{
	if (!ValidateInterruptSocket())
	{
		completion.Complete(Result(this));
		return;
	}

	AutoCrit autoCrit(statics->crit);

	if (opening || IsRecvOpen())
	{
		if (opening || IsRecving())
		{
			if (timeout != 0)	// Graceful close, possibly with a timeout until forceful close
			{
				unsigned long now = GetTickCount();

				if (recvClosing)	// Existing close, update timeout, and post our own
				{					
					if (pendingCloseRecvOp->timeout > 0 && !closeRecvCatcherOp)
						recvQueue.erase(pendingCloseRecvOp->queueItor);
					pendingCloseRecvOp->timeStarted = now;
					pendingCloseRecvOp->timeout = timeout;
					if (timeout > 0)
						pendingCloseRecvOp->timeoutItor = statics->timeoutSet.insert(pendingCloseRecvOp);
				}
				
				CloseRecvOp* closeRecvOp = new CloseRecvOp(*this);
				if (closeRecvOp)
				{
					auto_ptr<CloseRecvOp> auto_SockOp(closeRecvOp);
					
					closeRecvOp->timeStarted = now;
					closeRecvOp->closeCompletion = async ? completion : CompletionContainer<const Result&>() ;
					closeRecvOp->autoDelete = async;
					closeRecvOp->timeout = timeout;

					if (!recvClosing)
					{
						// Only add this one to timeout list, etc., if there's not already one there
						pendingCloseRecvOp = closeRecvOp;
						if (timeout > 0)
							closeRecvOp->timeoutItor = statics->timeoutSet.insert(closeRecvOp);
					}
					recvClosing = true;

					// Put in queue
					closeRecvOp->queueItor = recvQueue.insert(recvQueue.end(), closeRecvOp);

					if (!async)
					{
						autoCrit.Leave();
						//SpinOrInterruptSpinThread(timeout, closeRecvOp);
						InterruptSpin();
						PumpUntil(closeRecvOp->doneEvent, timeout);
						completion.Complete(Result(this));
						return;
					}
					InterruptSpin();
					auto_SockOp.release();
					return;
				}
				// else // fall through to synchronous close, if unable to allocate socketOp
			}
			// else if (timeout == 0) // forceful close
		}
		// else if (!IsRecving()) // No pending data anyway, close it now!
		int i = WON_SOCKET_CALL(shutdown)(sock, SD_RECEIVE);
		if (i == SOCKET_ERROR)
		{
			int err = WON_SOCKET_CALL(WSAGetLastError)();
			switch (err)
			{
			case WSAENOTCONN:			// not connected(?), might imply recv channel is already closed
				break;
			// case WSANOTINITIALISED:	// shouldn't happen
			// case WSAENETDOWN:		// net hosed
			// case WSAEINVAL:			// wrong type of socket(?)
			// case WSAEINPROGRESS:		// no idea
			// case WSAENOTSOCK:		// bad socket
			default:
				SocketClosed();	// Socket must be hosed, close it completely
				break;
			}
		}
		RecvChannelClosed();
		InterruptCompletionThread();
	}
	autoCrit.Leave();
	completion.Complete(Result(this));
}


void WSSocket::Close(long timeout, bool async, const CompletionContainer<const Result&>& completion)
{
	if (!ValidateInterruptSocket())
	{
		completion.Complete(Result(this));
		return;
	}

	AutoCrit autoCrit(statics->crit);

	if (listening && !connectionless)
	{
		listening = false;
		CloseAllRecvs();

		SocketClosed();
		InterruptCompletionThread();
	}
	else if (timeout == 0 || (!IsSending() && !IsRecving()))	// forceful close
	{
		SocketClosed();
		InterruptCompletionThread();
	}
	else if (sock != INVALID_SOCKET)
	{
		CloseOp* closeOp = new CloseOp(*this);
		if (!closeOp)
		{
			SocketClosed();
			InterruptCompletionThread();
		}
		else
		{
			auto_ptr<CloseOp> auto_SockOp(closeOp);

			closeOp->timeStarted = GetTickCount();
			closeOp->timeout = timeout;
			closeOp->autoDelete = async;
			closeOp->closeCompletion = async ? completion : CompletionContainer<const Result&>() ;

			if (closing)	// Existing close, update timeout, and post our own
			{
				OpList::iterator closeItor = closeQueue.begin();
				while (closeItor != closeQueue.end())
				{
					AsyncOp* closeOp = *closeItor;
					closeOp->timeout = timeout;
					closeOp->timeStarted = closeOp->timeStarted;
					closeItor++;
				}
			}
			else
				closing = true;

			closeOp->queueItor = closeQueue.insert(closeQueue.end(), closeOp);

			if (timeout > 0)
				closeOp->timeoutItor = statics->timeoutSet.insert(closeOp);

			CloseSend(timeout, true);
			// Graceful shutdown, close sends, then wait for recvs to close by itself

			if (connectionless && listening)
				CloseRecv(timeout, true);

			if (!async)
			{
				autoCrit.Leave();
				//SpinOrInterruptSpinThread(timeout, closeOp);
				InterruptSpin();
				PumpUntil(closeOp->doneEvent, timeout);
				completion.Complete(Result(this));
				return;
			}
			InterruptSpin();
			auto_SockOp.release();
			return;
		}
	}
	autoCrit.Leave();
	completion.Complete(Result(this));
}

	
unsigned long WSSocket::_Send(unsigned long count, const void* buffer, const Address* addr, long timeout, bool async, bool copyData, const CompletionContainer<const TransmitResult&>& completion)
{
	if (!ValidateInterruptSocket())
	{
		completion.Complete(TransmitResult(this, (void*)buffer, count, 0, true));
		return 0;
	}

	static const sockaddr bogusAddr = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	const Address* useAddr = 0;
	if (addr || (connectionless && listening))
		useAddr = addr ? addr : &destAddr;

	int sent = 0;
	bool closed = false;

	AutoCrit autoCrit(statics->crit);

	if (!sendClosing && buffer && (opening || IsSendOpen()))
	{
		if (!opening)
		{
			if (!IsSending() && (count || GetType() == datagram))
			{
				if (useAddr)
					sent = WON_SOCKET_CALL(sendto)(sock, (char*)buffer, count, 0, &(useAddr->Get()), sizeof(sockaddr)); 
				else
					sent = WON_SOCKET_CALL(send)(sock, (char*)buffer, count, 0);
				
				if (sent == SOCKET_ERROR)
				{
					sent = 0;
					int err = WON_SOCKET_CALL(WSAGetLastError)();
					switch (err)
					{
					case WSAEWOULDBLOCK:		// expected
					case WSAEINPROGRESS:		// no idea why this would happen
					case WSAENOBUFS:			// no (internal) buffers available, close socket(?)
						break;
					case WSAEHOSTUNREACH:		// Can't talk to remote host, close socket(?)
					case WSAEMSGSIZE:			// too big for a datagram!  Refuse it!
					case WSAENETUNREACH:
					case WSAEDESTADDRREQ:
					case WSAEAFNOSUPPORT:
					case WSAEADDRNOTAVAIL:
						autoCrit.Leave();
						completion.Complete(TransmitResult(this, (void*)buffer, count, 0, false));
						return 0;
						// Abort bad sendto(?), lose datagram
					case WSAESHUTDOWN:		// We closed send channel, or they closed recv channel
						SendChannelClosed();
						InterruptCompletionThread();
						autoCrit.Leave();
						completion.Complete(TransmitResult(this, (void*)buffer, count, 0, true));
						return 0;
					//case WSANOTINITIALISED:	// net hosed, close socket
					//case WSAENETDOWN:			// net hosed, close socket
					//case WSAEACCES:			// bad address, can't send to it.  close socket(?)
					//case WSAEINTR:			// shouldn't happen, we're non-blocking
					//case WSAEFAULT:			// user specified invalid buffer!
					//case WSAENETRESET:		// net hosed, close socket
					//case WSAENOTCONN:			// not connected, close socket
					//case WSAENOTSOCK:			// bad socket?  I hope not...
					//case WSAEOPNOTSUPP:		// shouldn't happen, socket setup issue
					//case WSAEINVAL:			// doh
					//case WSAECONNABORTED:		// socket hosed, close it
					//case WSAECONNRESET:		// hard or abortive close, close socket
					//case WSAETIMEDOUT:		// connection gone, close socket
					default:
						SocketClosed();
						InterruptCompletionThread();
						autoCrit.Leave();
						completion.Complete(TransmitResult(this, (void*)buffer, count, 0, true));
						return 0;

					}
				}
			}
		}
		if (sent != count || (!count && GetType() != datagram))
		{
			SendOp* sendOp = new SendOp(*this);
			if (sendOp)
			{
				auto_ptr<SendOp> auto_SockOp(sendOp);
				sendOp->ownsBuffer = copyData;

				// Copy only what hasn't been sent
				if (copyData)
				{
					unsigned long remaining = count - sent;
					sendOp->actualBuffer = new char[remaining];
					if (sendOp->actualBuffer)
					{
						memcpy(sendOp->actualBuffer, buffer, remaining);
						sendOp->buffer = (void*)((char*)sendOp->actualBuffer - sent);	// Back up, to fix offsets
						sendOp->ownsBuffer = true;
					}
					else
					{		// Couldn't copy buffer...
						autoCrit.Leave();
						completion.Complete(TransmitResult(this, (void*)buffer, count, sent, false));
						return sent;
					}
				}
				else
				{
					sendOp->buffer = (void*)buffer;
					sendOp->ownsBuffer = false;
				}

				sendOp->bufUsed = sent;
				sendOp->bufSize = count;
				sendOp->timeStarted = GetTickCount();
				sendOp->timeout = timeout;
				sendOp->recvSendCompletion = async ? completion : CompletionContainer<const TransmitResult&>() ;
				sendOp->autoDelete = async;
				sendOp->addr = useAddr;
					
				if (timeout >= 0)
					sendOp->timeoutItor = statics->timeoutSet.insert(sendOp);

				if (!IsSending())
					sendPendingItor = statics->pendingSends.insert(PendingMap::value_type(sock, this)).first;
				sendOp->queueItor = sendQueue.insert(sendQueue.end(), sendOp);

				if (!async)
				{
					autoCrit.Leave();
					//SpinOrInterruptSpinThread(timeout, sendOp);
					InterruptSpin();
					PumpUntil(sendOp->doneEvent, timeout);
					sent = sendOp->bufUsed;
					completion.Complete(TransmitResult(this, (void*)buffer, count, sent, false));
					return sent;
				}
				InterruptSpin();
				auto_SockOp.release();
				return 0;
			}
		}
	}
	else
		closed = true;
	if (!completion.empty())
	{
		if (async)	// Defer completion to prevent stack overflow
		{
			SendOp* sendOp = new SendOp(*this);
			if (sendOp)
			{
				if (copyData)
					sendOp->buffer = 0;
				else
					sendOp->buffer = (void*)buffer;
				sendOp->ownsBuffer = false;
				sendOp->bufUsed = sent;
				sendOp->bufSize = count;
				sendOp->recvSendCompletion = completion;
				sendOp->autoDelete = true;
				sendOp->closed = closed;
				sendOp->SocketOp::Trigger();
				InterruptSpin();
				return sent;
			}
		}
		autoCrit.Leave();
		completion.Complete(TransmitResult(this, (void*)buffer, count, sent, closed));
	}
	return sent;
}


unsigned long WSSocket::Send(unsigned long count, const void* buffer, long timeout, bool async, bool copyData, const CompletionContainer<const TransmitResult&>& completion)
{
	return _Send(count, buffer, 0, timeout, async, copyData, completion);
}

unsigned long WSSocket::SendTo(unsigned long count, const void* buffer, const Address& sendToAddr, long timeout, bool async, bool copyData, const CompletionContainer<const TransmitResult&>& completion)
{
	return _Send(count, buffer, &sendToAddr, timeout, async, copyData, completion);
}


unsigned long WSSocket::_Recv(unsigned long count, void* buffer, Address* addr, long timeout, bool async, const CompletionContainer<const TransmitResult&>& completion)
{
	if (!ValidateInterruptSocket())
	{
		completion.Complete(TransmitResult(this, buffer, count, 0, true));
		return 0;
	}

	char* buf = (char*)buffer;
	int recved = 0;
	bool closed = false;

	AutoCrit autoCrit(statics->crit);

	if (!recvClosing && (opening || IsRecvOpen()))
	{
		if (!buf)
			buf = new char[count > 0 ? count : 2];

		if (buf)
		{
			if (!IsRecving() && count > 0)
			{
				if (!opening)
				{
					if (addr)
					{
						socklen_t addrsize = sizeof(sockaddr);
						recved = WON_SOCKET_CALL(recvfrom)(sock, (char*)buf, count, 0, &(addr->Get()), &addrsize);
					}
					else
						recved = WON_SOCKET_CALL(recv)(sock, (char*)buf, count, 0); 

					if (recved == 0)
					{
						if (GetType() != datagram)
						{
							RecvChannelClosed();
							InterruptCompletionThread();
						}
						autoCrit.Leave();
						if (!buffer)
							delete buf;
						completion.Complete(TransmitResult(this, 0, count, 0, false));
						return 0;
					}
					if (recved == SOCKET_ERROR)
					{
						recved = 0;
						int err = WON_SOCKET_CALL(WSAGetLastError)();
						switch (err)
						{ 
						case WSAEWOULDBLOCK:		// implies recv channel is open
						case WSAEINPROGRESS:		// inconclusive... No idea why this would happen
							break;
						case WSAESHUTDOWN:			// recv is shut down!
							RecvChannelClosed();
							InterruptCompletionThread();
							// fall through
						case WSAEMSGSIZE:			// implies recv channel is open, and UDP msg is >buf
							// What to do it buf isn't large enough for datagram
							// Indicate this with a completed recv of 0 bytes
							autoCrit.Leave();
							if (!buffer)
								delete buf;
							completion.Complete(TransmitResult(this, 0, count, 0, false));
							return 0;
						//case WSANOTINITIALISED:	// cannot be open
						//case WSAENETDOWN:			// cannot be open
						//case WSAEFAULT:			// can't happen, our buf is right here
						//case WSAENOTCONN:			// cannot be open... Duh.
						//case WSAEINTR:			// cannot happen, we're non-blocking
						//case WSAENETRESET:		// cannot be open
						//case WSAENOTSOCK:			// cannot happen, unless we've lost the socket somehow
						//case WSAEOPNOTSUPP:		// cannot happen, socket setup issue
						//case WSAEINVAL:			// funky... no idea why we'd ever see this error
						//case WSAECONNABORTED:		// socket hosed, needs to be closed
						//case WSAETIMEDOUT:		// socket hosed, needs to be closed
						//case WSAECONNRESET:		// hard or abortive close, close socket
						default:
							SocketClosed();
							InterruptCompletionThread();
							autoCrit.Leave();
							if (!buffer)
								delete buf;
							completion.Complete(TransmitResult(this, 0, count, 0, true));
							return 0;
						}
					}
				}
			}
			if (recved == count && count > 0)
			{
				if (recvCatcherOp && !recvCatcherOp->triggered && !recvCatcherInstalled)	// Issued a direct recv, let recvCatcherOp check for more data
				{
					recvPendingItor = statics->pendingRecvs.insert(PendingMap::value_type(sock, this)).first;
					recvCatcherInstalled = true;
					InterruptSpin();
				}
			}
			else if ((!recved || GetType() != datagram || !count) && (timeout != 0))
			{
				RecvOp* recvOp = new RecvOp(*this);
				if (recvOp)
				{
					pendingRecvBytes += count - recved;
					auto_ptr<RecvOp> auto_SockOp(recvOp);
					recvOp->buffer = buf;
					recvOp->ownsBuffer = !buffer && async;
					recvOp->bufUsed = recved;
					recvOp->bufSize = count;
					recvOp->timeStarted = GetTickCount();
					recvOp->timeout = timeout;
					recvOp->recvSendCompletion = async ? completion : CompletionContainer<const TransmitResult&>() ;
					recvOp->autoDelete = async;
					recvOp->addr = addr;
						
					if (timeout >= 0)
						recvOp->timeoutItor = statics->timeoutSet.insert(recvOp);

					if (!IsRecving() && !recvCatcherInstalled)// duplicate insert should be ignored
						recvPendingItor = statics->pendingRecvs.insert(PendingMap::value_type(sock, this)).first;
					recvOp->queueItor = recvQueue.insert(recvQueue.end(), recvOp);
					if (!async)
					{
						autoCrit.Leave();
						//SpinOrInterruptSpinThread(timeout, recvOp);
						InterruptSpin();
						PumpUntil(recvOp->doneEvent, timeout);
						recved = recvOp->bufUsed;
						completion.Complete(TransmitResult(this, buf, count, recved, false));
						if (!buffer)
							delete buf;
						return recved;
					}
					InterruptSpin();
					auto_SockOp.release();
					return 0;
				}
			}
		}
	}
	else
		closed = true;
	if (!completion.empty())
	{
		if (async)	// Defer completion to prevent stack overflow
		{
			RecvOp* recvOp = new RecvOp(*this);
			if (recvOp)
			{
				recvOp->buffer = buf;
				recvOp->ownsBuffer = !buffer;
				recvOp->bufUsed = recved;
				recvOp->bufSize = count;
				recvOp->recvSendCompletion = completion;
				recvOp->autoDelete = true;
				recvOp->closed = closed;
				recvOp->SocketOp::Trigger();
				InterruptSpin();
				return 0;
			}
		}
		autoCrit.Leave();
		completion.Complete(TransmitResult(this, buf, count, recved, closed));
		if (!buffer)
			delete buf;
	}
	return recved;
}


unsigned long WSSocket::Recv(unsigned long count, void* buffer, long timeout, bool async, const CompletionContainer<const TransmitResult&>& completion)
{
	return _Recv(count, buffer, 0, timeout, async, completion);
}


unsigned long WSSocket::RecvFrom(unsigned long count, void* buffer, Address* recvFromAddr, long timeout, bool async, const CompletionContainer<const TransmitResult&>& completion)
{
	return _Recv(count, buffer, recvFromAddr, timeout, async, completion);
}


unsigned long WSSocket::Available()
{
	unsigned long availBytes = 0;

	AutoCrit autoCrit(statics->crit);

	if (IsRecvOpen())
	{
		unsigned long l = 0;
		int i = WON_SOCKET_CALL(ioctlsocket)(sock, FIONREAD, &l);
		if (i == SOCKET_ERROR)
		{
			SocketClosed();	// All errors returned by ioctlsocket indicate connection failure
			InterruptCompletionThread();
		}
		else if (l > pendingRecvBytes)
			availBytes = l - pendingRecvBytes;
	}

	return availBytes;
}


bool WSSocket::IsRecving()
{
	return !recvQueue.empty();
}


bool WSSocket::IsSending()
{
	return !sendQueue.empty();
}


bool WSSocket::AsyncOp::Process()
{
	Trigger();
	return true;
}


bool WSSocket::AcceptOp::Process()
{
	sockaddr remoteAddr;
#if defined(macintosh) && (macintosh == 1)
	unsigned int remoteAddrSize = sizeof(remoteAddr);
#else
	int remoteAddrSize = sizeof(remoteAddr);
#endif // macintosh
	SOCKET sock = WON_SOCKET_CALL(accept)(obj->sock, &remoteAddr, &remoteAddrSize);
	if (sock == INVALID_SOCKET)
	{
		int i = WON_SOCKET_CALL(WSAGetLastError)();
		switch (i)	// Really only two cases; no, or not yet
		{
		case WSAEWOULDBLOCK://The socket is marked as nonblocking and no connections are present to be accepted.
#ifdef _LINUX           
		case EINPROGRESS:
#endif
			return false;	// must have gotten them all, recheck for sendable socket
			break;
		//case WSAENOBUFS://No buffer space is available.
		//case WSANOTINITIALISED://A successful WSAStartup must occur before using this FUNCTION. 
		//case WSAENETDOWN://The network subsystem has failed. 
		//case WSAEFAULT://The addrlen parameter is too small or addr is not a valid part of the user address space. 
		//case WSAEINTR://The (blocking) call was canceled through WSACancelBlockingCall. 
		//case WSAEINPROGRESS://A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
		//case WSAEINVAL://The listen function was not invoked prior to accept. 
		//case WSAEMFILE://The queue is nonempty upon entry to accept and there are no descriptors available. 
		//case WSAENOTSOCK://The descriptor is not a socket. 
		//case WSAEOPNOTSUPP://The referenced socket is not a type that supports connection-oriented service. 
		default:
			break;
		}
	}
	else
	{
		int i = WON_SOCKET_CALL(setsockopt)(sock, SOL_SOCKET, SO_SNDBUF, (char*)&obj->sendBufferSize, sizeof(int)); 
		if (i == SOCKET_ERROR)
		{
			//int err = WON_SOCKET_CALL(WSAGetLastError)();
			//switch (err)
			//{
			//case WSANOTINITIALISED:// A successful WSAStartup must occur before using this function. 
			//case WSAENETDOWN:// The network subsystem has failed. 
			//case WSAEFAULT:// optval is not in a valid part of the process address space or optlen parameter is too small. 
			//case WSAEINPROGRESS:// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
			//case WSAEINVAL:// level is not valid, or the information in optval is not valid. 
			//case WSAENETRESET:// Connection has timed out when SO_KEEPALIVE is set. 
			//case WSAENOPROTOOPT:// The option is unknown or unsupported for the specified provider. 
			//case WSAENOTCONN:// Connection has been reset when SO_KEEPALIVE is set. 
			//case WSAENOTSOCK:// The descriptor is not a socket. 
			//default:
			//	break;
			//}
			WON_SOCKET_CALL(closesocket)(sock);// Leave sock invalid on failure
			return false;
		}
		i = WON_SOCKET_CALL(setsockopt)(sock, SOL_SOCKET, SO_RCVBUF, (char*)&obj->recvBufferSize, sizeof(int)); 
		if (i == SOCKET_ERROR)
		{
			//int err = WON_SOCKET_CALL(WSAGetLastError)();
			//switch (err)
			//{
			//case WSANOTINITIALISED:// A successful WSAStartup must occur before using this function. 
			//case WSAENETDOWN:// The network subsystem has failed. 
			//case WSAEFAULT:// optval is not in a valid part of the process address space or optlen parameter is too small. 
			//case WSAEINPROGRESS:// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
			//case WSAEINVAL:// level is not valid, or the information in optval is not valid. 
			//case WSAENETRESET:// Connection has timed out when SO_KEEPALIVE is set. 
			//case WSAENOPROTOOPT:// The option is unknown or unsupported for the specified provider. 
			//case WSAENOTCONN:// Connection has been reset when SO_KEEPALIVE is set. 
			//case WSAENOTSOCK:// The descriptor is not a socket. 
			//default:
			//	break;
			//}
			WON_SOCKET_CALL(closesocket)(sock);// Leave sock invalid on failure
			return false;
		}
		acceptToSocket->sock = sock;
		acceptToSocket->destAddr = remoteAddr;
		acceptToSocket->sendClosed = false;
		acceptToSocket->recvClosed = false;
		resultSocket = acceptToSocket;
	}
	return AsyncOp::Process();
}


bool WSSocket::RecvOp::Process()
{
//	if (bufSize)
//	{
		int i;
		if (addr)
		{
			socklen_t addrsize = sizeof(sockaddr);
			i = WON_SOCKET_CALL(recvfrom)(obj->sock, ((char*)buffer) + bufUsed, bufSize - bufUsed, 0, &(addr->Get()), &addrsize); 
		}
		else
			i = WON_SOCKET_CALL(recv)(obj->sock, ((char*)buffer) + bufUsed, bufSize - bufUsed, 0);

		if (i == 0)
		{
			if (obj->GetType() != datagram)	
			{
				obj->RecvChannelClosed();
				return true;
			}
		}
		if (i == SOCKET_ERROR)
		{
			i = 0;
			int err = WON_SOCKET_CALL(WSAGetLastError)();
			switch (err)
			{
			case WSAEWOULDBLOCK:		// implies recv channel is open
			case WSAEINPROGRESS:		// inconclusive... No idea why this would happen
				break;
			case WSAEMSGSIZE:			// implies recv channel is open, and UDP msg is >buffer
				// What to do it buffer isn't large enough for datagram
				// Indicate this with a completed recv of 0 bytes
				bufSize = bufUsed;
				break;
			case WSAESHUTDOWN:		// recv is shut down!
				obj->RecvChannelClosed();
				return true;
			//case WSANOTINITIALISED:	// cannot be open
			//case WSAENETDOWN:			// cannot be open
			//case WSAEFAULT:			// can't happen, our buf is right here
			//case WSAENOTCONN:			// cannot be open... Duh.
			//case WSAEINTR:			// cannot happen, we're non-blocking
			//case WSAENETRESET:		// cannot be open
			//case WSAENOTSOCK:			// cannot happen, unless we've lost the socket somehow
			//case WSAEOPNOTSUPP:		// cannot happen, socket setup issue
			//case WSAEINVAL:			// funky... no idea why we'd ever see this error
			//case WSAECONNABORTED:		// socket hosed, needs to be closed
			//case WSAETIMEDOUT:		// socket hosed, needs to be closed
			//case WSAECONNRESET:		// hard or abortive close, close socket
			default:
				obj->SocketClosed();
				return true;
			}
		}
		else
		{
			bufUsed += i;
			obj->pendingRecvBytes -= i;
		}
		if (obj->GetType() != datagram && bufSize != bufUsed)	
			return false;
//	}
	return AsyncOp::Process();
}


bool WSSocket::SendOp::Process()
{
//	if (bufSize)
//	{
		int i;
		if (addr)
			i = WON_SOCKET_CALL(sendto)(obj->sock, (char*)(buffer) + bufUsed, bufSize - bufUsed, 0, &(addr->Get()), sizeof(sockaddr)); 
		else
			i = WON_SOCKET_CALL(send)(obj->sock, (char*)(buffer) + bufUsed, bufSize - bufUsed, 0);
		if (i == SOCKET_ERROR)
		{
			int err = WON_SOCKET_CALL(WSAGetLastError)();
			switch (err)
			{
			case WSAEWOULDBLOCK:		// expected
			case WSAEINPROGRESS:		// no idea why this would happen
			case WSAENOBUFS:			// no (internal) buffers available, close socket(?)
				i = 0; // valid send of 0
				break;
			case WSAEHOSTUNREACH:		// Can't talk to remote host, close socket(?)
			case WSAEMSGSIZE:			// too big for a datagram!  Refuse it!
			case WSAENETUNREACH:
			case WSAEDESTADDRREQ:
			case WSAEAFNOSUPPORT:
			case WSAEADDRNOTAVAIL:
				i = 0;
				bufSize = bufUsed;
				// Abort bad sendto(?), lose datagram
				break;
			case WSAESHUTDOWN:		// We closed send channel, or they closed recv channel
				obj->SendChannelClosed();
				return true;
				// else fall through
			//case WSANOTINITIALISED:	// net hosed, close socket
			//case WSAENETDOWN:			// net hosed, close socket
			//case WSAEACCES:			// bad address, can't send to it.  close socket(?)
			//case WSAEINTR:			// shouldn't happen, we're non-blocking
			//case WSAEFAULT:			// user specified invalid buffer!
			//case WSAENETRESET:		// net hosed, close socket
			//case WSAENOTCONN:			// not connected, close socket
			//case WSAENOTSOCK:			// bad socket?  I hope not...
			//case WSAEOPNOTSUPP:		// shouldn't happen, socket setup issue
			//case WSAEINVAL:			// doh
			//case WSAECONNABORTED:		// socket hosed, close it
			//case WSAECONNRESET:		// hard or abortive close, close socket
			//case WSAETIMEDOUT:		// connection gone, close socket
			default:
				obj->SocketClosed();
				return true;
			}
		}
		bufUsed += i;

		if (bufUsed != bufSize)
			return false;
//	}
	return AsyncOp::Process();
}


bool WSSocket::CloseSendOp::Process()
{
	int i = WON_SOCKET_CALL(shutdown)(obj->sock, SD_SEND);
	if (i == SOCKET_ERROR)
	{
		int err = WON_SOCKET_CALL(WSAGetLastError)();
		switch (err)
		{
		case WSAENOTCONN:			// not connected(?), might imply send channel is already closed
			break;
		// case WSANOTINITIALISED:	// shouldn't happen
		// case WSAENETDOWN:		// net hosed
		// case WSAEINVAL:			// wrong type of socket(?)
		// case WSAEINPROGRESS:		// no idea
		// case WSAENOTSOCK:		// bad socket
		default:
			obj->SocketClosed();	// Socket must be hosed, close it completely
			break;
		}
	}
	obj->SendChannelClosed();
	return true;
}


bool WSSocket::CloseRecvOp::Process()
{
	int i = WON_SOCKET_CALL(shutdown)(obj->sock, SD_RECEIVE);
	if (i == SOCKET_ERROR)
	{
		int err = WON_SOCKET_CALL(WSAGetLastError)();
		switch (err)
		{
		case WSAENOTCONN:			// not connected(?), might imply channel is already closed
			break;
		// case WSANOTINITIALISED:	// shouldn't happen
		// case WSAENETDOWN:		// net hosed
		// case WSAEINVAL:			// wrong type of socket(?)
		// case WSAEINPROGRESS:		// no idea
		// case WSAENOTSOCK:		// bad socket
		default:
			obj->SocketClosed();	// Socket must be hosed, close it completely
			break;
		}
	}
	obj->RecvChannelClosed();
	return true;
}


// timeout of -1, and socketOp of NULL, implies loop forever
void WSSocket::SpinOrInterruptSpinThread(long timeout, SocketOp* socketOp)
{
#ifdef _MTAPI
	InterruptSpin();
#else
	Spin(timeout, socketOp);
#endif
}

/*void CheckMem()
{
	// Smart heap checks mem on allocation/deallocation.
	// Pepper code with CheckMem() to trigger this checking
	int* i = new int;
	delete i;
}*/


// timeout of -1, and socketOp of NULL, implies loop forever
void WSSocket::Spin(long timeout, SocketOp* socketOp, bool justOnce)
{
	ValidateInterruptSocket();

	AutoCrit autoCrit(statics->crit);

	if (timeout < 0 && spinningThread && killSpinThread
#ifdef _MTAPI
#ifdef WIN32
	&& GetCurrentThreadId() == spinThreadID
#elif defined(_LINUX)
	&& pthread_equal(pthread_self(), spinThread)
#endif
#endif
	)
		return;

	// If someone else is calling Spin(), we need to wait until they're done
	
	InterruptSpin();	// abort spin in any other thread
	++selectRef;

	unsigned long startTime = GetTickCount();
	unsigned long now = startTime;

	for (;;)	// Loop in case requested socketOp doesn't complete
	{
//		CheckMem();
		AutoCrit autoSpinCrit(statics->spinCrit);

		// If not multi-threaded, handle completions in spin

		if (socketOp && socketOp->triggered)
			break;

		// cases:
		// if timeout == 0, call select with no timeout
		// if timeout == -1, wait for max timeout.  Check op timeouts.  Must not be infinite, because of lock-up potential if UDP rehup is lost.
		// if timeout > 0, wait for max timeout.  Check op timeouts.   re-loop.  

		unsigned long masterTimeout = (unsigned long)timeout;	// -1 becomes unsigned 0xFFFFFFF

		if (timeout >= 0)	// Only -1 in SpinThread, so only exit if done and not in spin thread
		{
			if (statics->pendingRecvs.size() == 0)
			if (statics->pendingSends.size() == 0)
			if (statics->pendingOpens.size() == 0)
				break; // Nothing to do!

			unsigned long masterElapsed = now - startTime;
			if (masterElapsed > masterTimeout)
				break; // Requested spin() time elapsed
			masterTimeout -= masterElapsed;
		}
		//CheckMem();
		if (masterTimeout != 0)
		{
			// set up timeout
			TimeoutSet::iterator timeoutItor = statics->timeoutSet.begin();
			if (timeoutItor != statics->timeoutSet.end())
			{
				AsyncOp* timeoutOp = (*timeoutItor);
				
				unsigned long elapsed = now - timeoutOp->timeStarted;
				if (elapsed >= timeoutOp->timeout)
					masterTimeout = 0;
				else
				{
					unsigned long remaining = timeoutOp->timeout - elapsed;
					if (remaining < masterTimeout)
						masterTimeout = remaining;
				}
			}
		}

		//CheckMem();;
		int largest = 0;

#ifdef WIN32
		// Dynamically allocate FD_SET large enough
		unsigned long recvFDSize = statics->pendingRecvs.size() + 1;	// plug interrupt socket
		unsigned long sendFDSize = statics->pendingSends.size() + statics->pendingOpens.size() + 1;
		unsigned long exceptFDSize = sendFDSize + 1;

		if (recvFDSize < 64)
			recvFDSize = 64;
		if (sendFDSize < 64)
			sendFDSize = 64;
		if (exceptFDSize < 64)
			exceptFDSize = 64;

		//CheckMem();;
		unsigned long selectSize = ((recvFDSize + sendFDSize + exceptFDSize) * sizeof(SOCKET)) + (sizeof(u_int) * 3);
		// Allocates one big buffer to hold fd sets
		// Sure, it looks messy, but it's faster than using separate buffers
		char* selectBuffer = new char[selectSize];
		if (!selectBuffer)
			continue;	// throw an exception?
		auto_ptr<char> auto_buffer(selectBuffer);

				// Set up pointers
		fd_set* recv_fd   = (fd_set*)selectBuffer;
		fd_set* send_fd   = (fd_set*)(selectBuffer + ((recvFDSize * sizeof(SOCKET)) + sizeof(u_int)));
		fd_set* except_fd = (fd_set*)((char*)send_fd + ((sendFDSize * sizeof(SOCKET)) + sizeof(u_int)));

		// Build recv_fd
		recv_fd->fd_array[0] = interruptSocket;
		recv_fd->fd_count = 1;
		PendingMap::iterator itor1 = statics->pendingRecvs.begin();
		while (itor1 != statics->pendingRecvs.end())
		{
			recv_fd->fd_array[recv_fd->fd_count++] = (*itor1).second->sock;
			itor1++;
		}
		//CheckMem();;

		// Build send_fd
		send_fd->fd_count = 0;
		PendingMap::iterator itor2 = statics->pendingOpens.begin();
		while (itor2 != statics->pendingOpens.end())
		{
			send_fd->fd_array[send_fd->fd_count++] = (*itor2).second->sock;
			itor2++;
		}

		PendingMap::iterator itor3 = statics->pendingSends.begin();
		while (itor3 != statics->pendingSends.end())
		{
			send_fd->fd_array[send_fd->fd_count++] = (*itor3).second->sock;
			itor3++;
		}

		// Build except_fd
		except_fd->fd_array[0] = interruptSocket;	// Hopefully we get an except on closesocket
		except_fd->fd_count = 1;
		PendingMap::iterator itor4 = statics->pendingOpens.begin();
		while (itor4 != statics->pendingOpens.end())
		{
			except_fd->fd_array[except_fd->fd_count++] = (*itor4).second->sock;
			itor4++;
		}
#else//if defined(_LINUX)

		//CheckMem();;
		// Because unix fd_set can only support FD_SETSIZE, sockets beyond the limit
		// might appear to stall, until other sockets stop select()ing
		fd_set _recv_fd;
		fd_set _send_fd;
		fd_set* recv_fd = &_recv_fd;
		fd_set* send_fd = &_send_fd;
		fd_set* except_fd = 0;

		FD_ZERO(recv_fd);
		FD_ZERO(send_fd);


		// Build recv_fd
		FD_SET(interruptSocket, recv_fd);
		largest = interruptSocket;
		PendingMap::iterator itor1 = statics->pendingRecvs.begin();
		int used = 1;
		while (itor1 != statics->pendingRecvs.end())
		{
			SOCKET sock = (*itor1).second->sock;
			if (sock > largest)
				largest = sock;
			FD_SET(sock, recv_fd);
			if (++used == FD_SETSIZE)
				break;
			itor1++;
		}

		// Build send_fd
		//CheckMem();;
		PendingMap::iterator itor2 = statics->pendingSends.begin();
		used = 0;
		while (itor2 != statics->pendingSends.end())
		{
			SOCKET sock =(*itor2).second->sock;
			if (sock > largest)
				largest = sock;
			FD_SET(sock, send_fd);
			if (++used == FD_SETSIZE)
				break;
			itor2++;
		}
		if (used != FD_SETSIZE)
		{
			PendingMap::iterator itor3 = statics->pendingOpens.begin();
			while (itor3 != statics->pendingOpens.end())
			{
				SOCKET sock = (*itor3).second->sock;
				if (sock > largest)
					largest = sock;
				FD_SET(sock, send_fd);
				if (++used == FD_SETSIZE)
					break;
				itor3++;
			}
		}
		//CheckMem();;
#endif

		if (timeout > 0)
		{
			unsigned long aTime = GetTickCount();
			aTime -= now;	// Acount for time it took to build FDs
			if (aTime < masterTimeout)
				masterTimeout -= aTime;
			else
				masterTimeout = 0;
		}
		timeval timeoutval;
		timeoutval.tv_sec = masterTimeout/1000;
		timeoutval.tv_usec = (masterTimeout%1000)*1000;

		if (timeoutval.tv_sec > 2)
			timeoutval.tv_sec = 2;	// 2 seconds max timeout

		DrainInterruptSocket();

		selecting = true;
		unsigned long match = selectRef;

		autoCrit.Leave();

		int result;
		try {
			result = WON_SOCKET_CALL(select)(largest+1,			// ignored in winsock
				recv_fd,						// There will always at least be an interrupt socket here
				send_fd,						//send_fd->fd_count	? send_fd	: NULL,
				except_fd,						//except_fd->fd_count	? except_fd	: NULL,
				&timeoutval); 
		}
		catch (...)
		{
			result = 0;
		}

	
		//CheckMem();;
		selecting = false;
		now = GetTickCount();

		autoSpinCrit.Leave();
		autoCrit.Enter();

		if (timeout < 0 && spinningThread && killSpinThread
#ifdef _MTAPI
#ifdef WIN32
		&& GetCurrentThreadId() == spinThreadID
#elif defined(_LINUX)
		&& pthread_equal(pthread_self(), spinThread)
#endif
#endif
		)
			return;

		//CheckMem();;
		if (result == SOCKET_ERROR)
		{
			int err = WON_SOCKET_CALL(WSAGetLastError)();
			switch (err)
			{
			case WSAEFAULT:				// Resources are running low, try again?
			case WSAENOTSOCK:			// Socket got closed right as select() was called
			case WSAEINTR:				// Shouldn't happen, but seems to under Win 98?!
				continue;
			//case WSANOTINITIALISED:	// Shouldn't happen, we init'ed winsock
			//case WSAENETDOWN:			// Net is gone
			//case WSAEINVAL:			// Weird... abort
			//case WSAEINPROGRESS:		// No idea why this would happen
			default:
				return;
			}
		}

		//CheckMem();
		DrainInterruptSocket();
		// if interrupt socket is recvable, drain it

		bool ignore = (selectRef != match);
		if (!ignore)
		{			
			if (result)	//  got some
			{
#ifdef WIN32
				//CheckMem();
				while (send_fd->fd_count)
				{
					//CheckMem();
					SOCKET sendable_socket = send_fd->fd_array[--send_fd->fd_count];

					PendingMap::iterator itor = statics->pendingOpens.find(sendable_socket);
					if (itor != statics->pendingOpens.end())
					{
						WSSocket* openSocket = (*itor).second;

						openSocket->pendingOpenOp->error = Error_Success;
						openSocket->pendingOpenOp->Trigger();
					}
					else
					{
						PendingMap::iterator itor = statics->pendingSends.find(sendable_socket);
						if (itor != statics->pendingSends.end())
						{
							WSSocket* sendSocket = (*itor).second;
							
							OpList::iterator itor = sendSocket->sendQueue.begin();
							while (itor != sendSocket->sendQueue.end())
							{
								AsyncOp* asyncOp = (*itor);
								if (!asyncOp->Process())
									break;
								itor = sendSocket->sendQueue.begin();
							}
						}
					}
				}
				
				//CheckMem();;
				while (recv_fd->fd_count)
				{
					//CheckMem();
					SOCKET recvable_socket = recv_fd->fd_array[--recv_fd->fd_count];
					
					if (recvable_socket == interruptSocket)
						continue;	// skip it, don't bother with a find

					PendingMap::iterator itor = statics->pendingRecvs.find(recvable_socket);
					if (itor != statics->pendingRecvs.end())
					{
						WSSocket* recvSocket = (*itor).second;

						OpList::iterator itor = recvSocket->recvQueue.begin();
						if (itor == recvSocket->recvQueue.end())
						{
							if (recvSocket->listening && !recvSocket->connectionless)
							{
								// if (sendSocket->acceptCatcherOp) // Must be...
								//{
									if (!recvSocket->acceptCatcherQueued)
									{
										AcceptCatcherOp* catcher = recvSocket->acceptCatcherOp;
										if (catcher->autoDelete)
											recvSocket->acceptCatcherOp = 0;
										catcher->Trigger();
									}
								//}
							}
							else // if (recvSocket->recvCatcherOp) // Must be...
							{
								if (!recvSocket->recvCatcherQueued)
								{
									RecvCatcherOp* catcher = recvSocket->recvCatcherOp;
									if (catcher->autoDelete)
										recvSocket->recvCatcherOp = 0;
									catcher->Trigger();
								}
							}
						}
						else do
						{
							AsyncOp* asyncOp = (*itor);
							if (!asyncOp->Process())
								break;
							itor = recvSocket->recvQueue.begin();
							//CheckMem();
						} while (itor != recvSocket->recvQueue.end());
					}
				}

				//CheckMem();;
				while (except_fd->fd_count)
				{
					//CheckMem();
					SOCKET except_socket = except_fd->fd_array[--except_fd->fd_count];

					PendingMap::iterator itor = statics->pendingOpens.find(except_socket);
					if (itor != statics->pendingOpens.end())
					{
						WSSocket* openSocket = (*itor).second;
						
						openSocket->pendingOpenOp->error = Error_HostUnreachable;
						openSocket->pendingOpenOp->Trigger();
					}
				}

#else//if defined(_LINUX)
				//CheckMem();;
				PendingMap::iterator opItor = statics->pendingOpens.begin();
				while (opItor != statics->pendingOpens.end())
				{
					//CheckMem();
					WSSocket* openSocket = (*opItor).second;
					opItor++;
					if (FD_ISSET(openSocket->sock, send_fd))
					{
						FD_CLR(openSocket->sock, send_fd);

						long anError = 0;
						socklen_t aLen = 4;

						int getsockoptresult = WON_SOCKET_CALL(getsockopt)(openSocket->sock,SOL_SOCKET,SO_ERROR,(char*)&anError,&aLen);
						
						openSocket->pendingOpenOp->error = (anError == 0) ? Error_Success : Error_HostUnreachable;
						openSocket->pendingOpenOp->Trigger();
					}
				}

				//CheckMem();;
				opItor = statics->pendingSends.begin();
				while (opItor != statics->pendingSends.end())
				{
					//CheckMem();
					WSSocket* sendSocket = (*opItor).second;
					opItor++;
					if (FD_ISSET(sendSocket->sock, send_fd))
					{
						OpList::iterator itor = sendSocket->sendQueue.begin();
						while (itor != sendSocket->sendQueue.end())
						{
							AsyncOp* asyncOp = (*itor);
							if (!asyncOp->Process())
								break;
							itor = sendSocket->sendQueue.begin();
						}
					}
				}

				//CheckMem();;
				opItor = statics->pendingRecvs.begin();
				while (opItor != statics->pendingRecvs.end())
				{
					//CheckMem();
					WSSocket* recvSocket = (*opItor).second;
					opItor++;
					
					if (FD_ISSET(recvSocket->sock, recv_fd))
					{
						OpList::iterator itor = recvSocket->recvQueue.begin();
						if (itor == recvSocket->recvQueue.end())
						{
							if (recvSocket->listening && !recvSocket->connectionless)
							{
								// if (sendSocket->acceptCatcherOp) // Must be...
								//{
									if (!recvSocket->acceptCatcherQueued)
									{
										AcceptCatcherOp* catcher = recvSocket->acceptCatcherOp;
										if (catcher->autoDelete)
											recvSocket->acceptCatcherOp = 0;
										catcher->Trigger();
									}
								//}
							}
							else // if (recvSocket->recvCatcherOp) // Must be...
							{
								if (!recvSocket->recvCatcherQueued)
								{
									RecvCatcherOp* catcher = recvSocket->recvCatcherOp;
									if (catcher->autoDelete)
										recvSocket->recvCatcherOp = 0;
									catcher->Trigger();
								}
							}
						}
						else do
						{
							AsyncOp* asyncOp = (*itor);
							if (!asyncOp->Process())
								break;
							itor = recvSocket->recvQueue.begin();
							//CheckMem();
						} while (itor != recvSocket->recvQueue.end());
					}
				}
#endif
			}

			//CheckMem();;
			// Cancel any operations that have timed out
			TimeoutSet::iterator timeoutItor = statics->timeoutSet.begin();
			while (timeoutItor != statics->timeoutSet.end())
			{
				//CheckMem();
				AsyncOp* timeoutOp = (*timeoutItor);
				timeoutItor++;

				unsigned long elapsed = now - timeoutOp->timeStarted;
				if (elapsed >= timeoutOp->timeout)
					timeoutOp->Timeout();
			}

			//CheckMem();

			if (justOnce)
				break;
		}

		now = GetTickCount();
		//CheckMem();
	}

}


void WSSocket::AsyncOp::Timeout()
{
	Trigger();
}


void WSSocket::OpenOp::Timeout()
{
	error = Error_Timeout;
	Trigger();
}


void WSSocket::CloseRecvOp::Timeout()
{
	int i = WON_SOCKET_CALL(shutdown)(obj->sock, SD_RECEIVE);
	if (i == SOCKET_ERROR)
	{
		int err = WON_SOCKET_CALL(WSAGetLastError)();
		switch (err)
		{
		case WSAENOTCONN:			// not connected(?), might imply send channel is already closed
			break;
		// case WSANOTINITIALISED:	// shouldn't happen
		// case WSAENETDOWN:		// net hosed
		// case WSAEINVAL:			// wrong type of socket(?)
		// case WSAEINPROGRESS:		// no idea
		// case WSAENOTSOCK:		// bad socket
		default:
			obj->SocketClosed();	// Socket must be hosed, close it completely
			break;
		}
	}
	obj->RecvChannelClosed();
}

void WSSocket::CloseSendOp::Timeout()
{
	int i = WON_SOCKET_CALL(shutdown)(obj->sock, SD_SEND);
	if (i == SOCKET_ERROR)
	{
		int err = WON_SOCKET_CALL(WSAGetLastError)();
		switch (err)
		{
		case WSAENOTCONN:			// not connected(?), might imply send channel is already closed
			break;
		// case WSANOTINITIALISED:	// shouldn't happen
		// case WSAENETDOWN:		// net hosed
		// case WSAEINVAL:			// wrong type of socket(?)
		// case WSAEINPROGRESS:		// no idea
		// case WSAENOTSOCK:		// bad socket
		default:
			obj->SocketClosed();	// Socket must be hosed, close it completely
			break;
		}
	}
	obj->SendChannelClosed();
}

void WSSocket::CloseOp::Timeout()
{
	obj->SocketClosed();
}


char interruptDummyBuffer = 0;


void WSSocket::InterruptSpin()
{
#ifdef _MTAPI
	AutoCrit autoCrit(statics->crit);

	statics->spinThreadStartEvent.Set();

	if (selecting)
		WON_SOCKET_CALL(send)(interruptSocket, &interruptDummyBuffer, 1, 0);
#endif
}


void WSSocket::InterruptCompletionThread()
{
#ifdef _MTAPI
	if (firstCompleted)
		statics->completionThreadEvent.Set();
#endif
}


void WSSocket::DrainInterruptSocket()
{
	AutoCrit autoCrit(statics->crit);

	do {} while (WON_SOCKET_CALL(recv)(interruptSocket, &interruptDummyBuffer, 1, 0) > 0);
}


bool WSSocket::ValidateInterruptSocket()
{
	AutoCrit autoCrit(statics->crit);

	if (interruptSocket == INVALID_SOCKET)
	{
		// Allocate interrupt socket
		interruptSocket = WON_SOCKET_CALL(socket)(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (interruptSocket != INVALID_SOCKET)
		{
#ifdef WIN32
			sockaddr_in loopback = { AF_INET, 0, { 127, 0, 0, 1 }, 0, 0, 0, 0, 0, 0, 0, 0};
#else//if defined(_LINUX)
			sockaddr_in loopback = { AF_INET, 0, { 0x7F000001 }, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

#if defined(macintosh) && (macintosh == 1)
			int i = WON_SOCKET_CALL(bind)(interruptSocket, (sockaddr*)&loopback, sizeof(sockaddr_in));
#else
			int i = WON_SOCKET_CALL(connect)(interruptSocket, (sockaddr*)&loopback, sizeof(sockaddr_in));
#endif
			if (i == SOCKET_ERROR)
			{
				//int err = WON_SOCKET_CALL(WSAGetLastError)();
				//switch (err)
				//{
				//case WSANOTINITIALISED:		// shouldn't happen
				//case WSAENETDOWN:				// net is hosed
				//case WSAEINPROGRESS:			// no idea why that would happen
				//case WSAENOTSOCK:				// shouldn't happen
				//case WSAEFAULT:				// shouldn't happen
				//default:
					WON_SOCKET_CALL(closesocket)(interruptSocket);// Leave sock invalid on failure
					interruptSocket = INVALID_SOCKET;
					return false;
				//}
			}

 			// Get local port
			socklen_t addrSize = sizeof(sockaddr_in);
			i = WON_SOCKET_CALL(getsockname)(interruptSocket, (sockaddr*)&loopback, &addrSize);
			if (i == SOCKET_ERROR)
			{
				//int err = WON_SOCKET_CALL(WSAGetLastError)();
				//switch (err)
				//{
				//case WSANOTINITIALISED:		// shouldn't happen
				//case WSAENETDOWN:				// net is hosed
				//case WSAEINPROGRESS:			// no idea why that would happen
				//case WSAENOTSOCK:				// shouldn't happen
				//case WSAEFAULT:				// shouldn't happen
				//default:
					WON_SOCKET_CALL(closesocket)(interruptSocket);// Leave sock invalid on failure
					interruptSocket = INVALID_SOCKET;
					return false;
				//}
			}


			// Need to fill in address again, since only port was returned
#ifdef WIN32
			loopback.sin_addr.S_un.S_un_b.s_b1 = 127;
			loopback.sin_addr.S_un.S_un_b.s_b2 = 0;
			loopback.sin_addr.S_un.S_un_b.s_b3 = 0;
			loopback.sin_addr.S_un.S_un_b.s_b4 = 1;
#else//if defined(_LINUX)
			unsigned char* loopbackAddr = (unsigned char*)&(loopback.sin_addr.s_addr);
			*loopbackAddr = 127;
			*(loopbackAddr+1) = 0;
			*(loopbackAddr+2) = 0;
			*(loopbackAddr+3) = 1;
#endif
			
			// Connect again, to change the destination to the same random port
			i = WON_SOCKET_CALL(connect)(interruptSocket, (sockaddr*)&loopback, sizeof(sockaddr_in));
			if (i == SOCKET_ERROR)
			{
				//int err = WON_SOCKET_CALL(WSAGetLastError)();
				//switch (err)
				//{
				//case WSANOTINITIALISED:		// shouldn't happen
				//case WSAENETDOWN:			// net is hosed
				//case WSAEINPROGRESS:			// no idea why that would happen
				//case WSAENOTSOCK:			// shouldn't happen
				//case WSAEFAULT:				// shouldn't happen
				//default:
					WON_SOCKET_CALL(closesocket)(interruptSocket);// Leave sock invalid on failure
					interruptSocket = INVALID_SOCKET;
					return false;
				//}
			}
#ifdef WIN32
			unsigned long argp = 1;
			i = WON_SOCKET_CALL(ioctlsocket)(interruptSocket, FIONBIO, &argp);
#else//if defined(_LINUX)
			i = fcntl(interruptSocket, F_SETFL, O_NONBLOCK);
#endif
			if (i == SOCKET_ERROR)
			{
				//int err = WON_SOCKET_CALL(WSAGetLastError)();
				//switch (err)
				//{
				//case WSANOTINITIALISED:		// shouldn't happen
				//case WSAENETDOWN:			// net is hosed
				//case WSAEINPROGRESS:			// no idea why that would happen
				//case WSAENOTSOCK:			// shouldn't happen
				//case WSAEFAULT:				// shouldn't happen
				//default:
					WON_SOCKET_CALL(closesocket)(interruptSocket);// Leave sock invalid on failure
					interruptSocket = INVALID_SOCKET;
					return false;
				//}
			}
		}
		else
			return false;
	}

	return StartSpinThread();
}


void WSSocket::DeleteInterruptSocket()
{
	AutoCrit autoCrit(statics->crit);

	if (interruptSocket != INVALID_SOCKET)
		WON_SOCKET_CALL(closesocket)(interruptSocket);
}

#ifdef _MTAPI

#ifdef _LINUX
void* WSSocket::CompletionThread(void* unused)
#elif defined(WIN32)
unsigned int __stdcall WSSocket::CompletionThread(void* unused)
#else
#error unknown platform
#endif
{
	AutoCrit autoCrit(statics->crit);
	for (;;)
	{		
		while (firstCompleted)
		{
			SocketOp* completedSocketOp = firstCompleted;
			firstCompleted = firstCompleted->nextCompleted;
			completedOnSocket = completedSocketOp->obj;
			completedSocketOp->nextCompleted = 0;
			completedOnSocket->numCompletesPending--;

//			CheckCompleted();

			autoCrit.Leave();
			completedSocketOp->Complete();
			autoCrit.Enter();
			
//			CheckCompleted();

			if (completedOnSocket&& !completedOnSocket->numCompletesPending)
				completedOnSocket->completionTracker.Set();
		}
		if (killSpinThread && !spinningThread)
			break;
		statics->completionThreadEvent.Reset();
		autoCrit.Leave();
		statics->completionThreadEvent.WaitFor();
		autoCrit.Enter();
	}
	return 0;
}


#ifdef _LINUX
void* WSSocket::SpinThread(void* unused)
#elif defined(WIN32)
unsigned int __stdcall WSSocket::SpinThread(void* unused)
#else
#error unknown platform
#endif
{
	spinningThread = true;
	statics->spinThreadStartEvent.WaitFor();
	if (!killSpinThread)
	{
		Spin(-1, 0);	// wont exit until it's aborted.. Or, a bad winsock error might kill the thread?
	}
	spinningThread = false;
	AutoCrit autoCrit(statics->crit);
	statics->completionThreadEvent.Set();
	return 0;
}

#endif


bool WSSocket::StartSpinThread()
{
#ifdef _MTAPI
	AutoCrit autoCrit(statics->crit);

	if (spinningThread)
		return true;
	
	statics->spinThreadStartEvent.Reset();

#ifdef _LINUX
	int result = pthread_create(&spinThread, 0, WSSocket::SpinThread, 0);
	if (result == 0)
#elif defined(WIN32)
	spinThreadHandle = (HANDLE)_beginthreadex(0, 0, WSSocket::SpinThread, 0, 0, (unsigned int*)&spinThreadID);
	if (spinThreadHandle != 0)
#endif
	{

#ifdef _LINUX
		result = pthread_create(&completionThread, 0, WSSocket::CompletionThread, 0);
		if (result == 0)
#elif defined(WIN32)
		completionThreadHandle = (HANDLE)_beginthreadex(0, 0, WSSocket::CompletionThread, 0, 0, (unsigned int*)&completionThreadID);
		if (completionThreadHandle != 0)
#endif
		{
			spinningThread = true;
			return true;
		}
		else // stop spin thread
		{
			killSpinThread = true;
			statics->spinThreadStartEvent.Set();
#ifdef WIN32
			CloseHandle(spinThreadHandle);
#endif
		}
	}
	return false;

#else	// _MTAPI
	return true;
#endif	// _MTAPI
}


void WSSocket::StopSpinThread()
{
#ifdef _MTAPI
	AutoCrit autoCrit(statics->crit);

	if (spinningThread)
	{

#ifdef WIN32
		if (GetCurrentThreadId() == spinThreadID)
			_endthreadex(0);
#elif defined(_LINUX)
		if (pthread_equal(pthread_self(), spinThread))
			pthread_exit(0);
#endif

		killSpinThread = true;
		InterruptSpin();

		autoCrit.Leave();
		
#ifdef _LINUX
		pthread_join(completionThread, 0);

#elif defined(WIN32)
		WaitForSingleObject(completionThreadHandle, INFINITE);

		CloseHandle(completionThreadHandle);
		CloseHandle(spinThreadHandle);
#endif
		killSpinThread = false;
	}
#endif	// _MTAPI
	if (interruptSocket != INVALID_SOCKET)
	{
		WON_SOCKET_CALL(closesocket)(interruptSocket);
		interruptSocket = INVALID_SOCKET;
	}
}


void WSSocket::CatchClose(const CompletionContainer<const Result&>& completion, bool reuse)
{
	AutoCrit autoCrit(statics->crit);
	
	if (closeCatcherOp)
	{
		closeCatcherOp->crit.Enter();
		// Now, if obj is reusable, and triggering out of the common crit, this will stall it
		// from Complete()ing...  We don't want to delete while it's in the completing list.
		// If it's in the completing list, we'll just mark it as autoDelete...
		if (closeCatcherOp->triggered) // && reuse.  non-reuse catcherOp's will be removed when triggered
		{
			closeCatcherOp->autoDelete = true;
			closeCatcherOp->crit.Leave();
		}
		else // if not triggered
		{
			// if !triggered, and we're in statics->crit, then we know it wont be accessed
			closeCatcherOp->crit.Leave();
			delete closeCatcherOp;
		}
		closeCatcherOp = 0;
	}
	if (!completion.empty())
	{		
		closeCatcherOp = new CloseCatcherOp(*this);  // Out of memory condition not handled... need to fix
		if (closeCatcherOp)
		{
			closeCatcherOp->autoDelete = !reuse;
			closeCatcherOp->catcherCompletion = completion;
			closeCatcherOp->catcherCompletion.OwnCompletion();
		}
	}
}


void WSSocket::CatchAcceptable(const CompletionContainer<const Result&>& completion, bool reuse)
{
	if (connectionless)
		return;

	AutoCrit autoCrit(statics->crit);
	
	if (acceptCatcherOp)
	{
		acceptCatcherOp->crit.Enter();
		// Now, if obj is reusable, and triggering out of the common crit, this will stall it
		// from Complete()ing...  We don't want to delete while it's in the completing list.
		// If it's in the completing list, we'll just mark it as autoDelete...
		if (acceptCatcherOp->triggered) // && reuse.  non-reuse catcherOp's will be removed when triggered
		{
			acceptCatcherOp->autoDelete = true;
			acceptCatcherOp->crit.Leave();
		}
		else // if not triggered
		{
			// if !triggered, and we're in statics->crit, then we know it wont be accessed
			acceptCatcherOp->crit.Leave();
			delete acceptCatcherOp;
		}
		if (listening && !IsAccepting() && recvCatcherInstalled)	// No need to select on socket if we already are
		{
			statics->pendingRecvs.erase(recvPendingItor);
			recvPendingItor = statics->pendingRecvs.end();	// debug
			recvCatcherInstalled = false;
			InterruptSpin();
		}
		acceptCatcherOp = 0;
	}
	if (!completion.empty())
	{
		acceptCatcherOp = new AcceptCatcherOp(*this);  // Out of memory condition not handled... need to fix
		if (acceptCatcherOp)
		{
			acceptCatcherOp->autoDelete = !reuse;
			acceptCatcherOp->catcherCompletion = completion;
			acceptCatcherOp->catcherCompletion.OwnCompletion();
		}
		if (listening && !IsAccepting())	// No need to select on socket if we already are
		{
			recvCatcherInstalled = true;
			recvPendingItor = statics->pendingRecvs.insert(PendingMap::value_type(sock, this)).first;
			InterruptSpin();
		}
	}
}


void WSSocket::CatchCloseRecv(const CompletionContainer<const Result&>& completion, bool reuse)
{
	AutoCrit autoCrit(statics->crit);
	
	if (closeRecvCatcherOp)
	{
		closeRecvCatcherOp->crit.Enter();
		// Now, if obj is reusable, and triggering out of the common crit, this will stall it
		// from Complete()ing...  We don't want to delete while it's in the completing list.
		// If it's in the completing list, we'll just mark it as autoDelete...
		if (closeRecvCatcherOp->triggered) // && reuse.  non-reuse catcherOp's will be removed when triggered
		{
			closeRecvCatcherOp->autoDelete = true;
			closeRecvCatcherOp->crit.Leave();
		}
		else // if not triggered
		{
			// if !triggered, and we're in statics->crit, then we know it wont be accessed
			closeRecvCatcherOp->crit.Leave();
			delete closeRecvCatcherOp;
		}
		closeRecvCatcherOp = 0;
	}
	if (!completion.empty())
	{
		closeRecvCatcherOp = new CloseRecvCatcherOp(*this);  // Out of memory condition not handled... need to fix
		if (closeRecvCatcherOp)
		{
			closeRecvCatcherOp->autoDelete = !reuse;
			closeRecvCatcherOp->catcherCompletion = completion;
			closeRecvCatcherOp->catcherCompletion.OwnCompletion();
		}
	}
}


void WSSocket::CatchRecvable(const CompletionContainer<const Result&>& completion, bool reuse)
{
	AutoCrit autoCrit(statics->crit);
	
	if (recvCatcherOp)
	{
		recvCatcherOp->crit.Enter();
		// Now, if obj is reusable, and triggering out of the common crit, this will stall it
		// from Complete()ing...  We don't want to delete while it's in the completing list.
		// If it's in the completing list, we'll just mark it as autoDelete...
		if (recvCatcherOp->triggered) // && reuse.  non-reuse catcherOp's will be removed when triggered
		{
			recvCatcherOp->autoDelete = true;
			recvCatcherOp->crit.Leave();
		}
		else // if not triggered
		{
			// if !triggered, and we're in statics->crit, then we know it wont be accessed
			recvCatcherOp->crit.Leave();
			delete recvCatcherOp;
		}
		if (!IsRecving() && (!listening || connectionless) && recvCatcherInstalled)
		{
			recvCatcherInstalled = false;
			statics->pendingRecvs.erase(recvPendingItor);
			recvPendingItor = statics->pendingRecvs.end();	// debug
			InterruptSpin();
		}
		recvCatcherOp = 0;
	}
	if (!completion.empty())
	{
		recvCatcherOp = new RecvCatcherOp(*this);  // Out of memory condition not handled... need to fix
		if (recvCatcherOp)
		{
			recvCatcherOp->autoDelete = !reuse;
			recvCatcherOp->catcherCompletion = completion;
			recvCatcherOp->catcherCompletion.OwnCompletion();
		}
		
		if (!IsRecving() && (!listening || connectionless) && IsRecvOpen())	// No need to select on socket if we already are
		{
			recvCatcherInstalled = true;
			recvPendingItor = statics->pendingRecvs.insert(PendingMap::value_type(sock, this)).first;
			InterruptSpin();
		}
	}
}


bool WSSocket::AbortRecv()
{
	AutoCrit autoCrit(statics->crit);

	if (!recvQueue.empty())
	{
		OpList::iterator itor = recvQueue.begin();
		(*itor)->Trigger();
		InterruptSpin();
		return true;
	}
	return false;
}


bool WSSocket::AbortSend()
{
	AutoCrit autoCrit(statics->crit);

	if (!sendQueue.empty())
	{
		OpList::iterator itor = sendQueue.begin();
		(*itor)->Trigger();
		InterruptSpin();
		return true;
	}
	return false;
}


void WSSocket::DoCompletions()
{
	while (firstCompleted)
	{
		SocketOp* completedSocketOp = firstCompleted;
		firstCompleted = firstCompleted->nextCompleted;
		completedOnSocket = completedSocketOp->obj;
		completedOnSocket->numCompletesPending--;
		completedSocketOp->Complete();

		if (!completedOnSocket->numCompletesPending)
			completedOnSocket->completionTracker.Set();
	}
}


void WSSocket::Pump(unsigned long timeout)
{
#ifndef _MTAPI
	DoCompletions();
	Spin(timeout & 0x8FFFFFFF, 0);
	DoCompletions();
#endif
}


#ifdef WIN32

// Warning:  Only use this to wait on a handle that WILL be set by 
bool WSSocket::PumpUntil(HANDLE obj, DWORD timeout)
{
#ifdef _MTAPI
	DWORD curThreadID = GetCurrentThreadId();
	if (curThreadID != completionThreadID)
		return WaitForSingleObject(obj, timeout) != WAIT_TIMEOUT;
#endif
	DWORD startTime = GetTickCount();
	bool result = true;
	DoCompletions();
	DWORD wfso;
	while (wfso = WaitForSingleObject(obj, 0), wfso == WAIT_TIMEOUT)
	{
		Spin(timeout, 0, true);
		DoCompletions();
		if (timeout != INFINITE)
		{
			DWORD curTime = GetTickCount();
			DWORD span = curTime - startTime;
			if (span >= timeout)
			{
				result = false;
				break;
			}
		}
	}
	return result;
}
#endif


bool WSSocket::PumpUntil(Event& evt, DWORD timeout)
{
#ifdef _MTAPI


#ifdef WIN32
	DWORD curThreadID = GetCurrentThreadId();
	if (curThreadID != completionThreadID)
#elif defined(_LINUX)
	pthread_t thisThread = pthread_self();
	if (!pthread_equal(thisThread, completionThread))
#endif
		return evt.WaitFor();

#endif
	DWORD startTime = GetTickCount();
	bool result = true;
	//DoCompletions();
	while (!evt.Test())
	{
		Spin(timeout, 0, true);
		DoCompletions();
		if (timeout != INFINITE)
		{
			DWORD curTime = GetTickCount();
			DWORD span = curTime - startTime;
			if (span >= timeout)
			{
				result = false;
				break;
			}
		}
	}
	return result;
}


void WONPump(unsigned long timeout)
{
	WSSocket::Pump(timeout);
}

#ifdef WIN32

int WONPumpUntil(HANDLE obj, DWORD timeout)
{
	return WSSocket::PumpUntil(obj, timeout) ? 1 : 0;
}

#endif


bool WSSocket::InstallTimer(unsigned long timeout, const CompletionContainer<void>& completion)
{
	bool result;

	if (completion.empty())
		result = true;
	else
	{
		result = false;
		if (ValidateInterruptSocket())
		{
			TimerOp* timerOp = new TimerOp();
			if (timerOp)
			{
				timerOp->timeStarted = GetTickCount();
				timerOp->timeout = timeout;
				timerOp->timerCompletion = completion;
				timerOp->timeoutItor = statics->timeoutSet.insert((AsyncOp*)timerOp);

				result = true;
			}
		}
	}
	return result;
}

