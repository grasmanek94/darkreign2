/********************************************************************************

  WON::WSSocket

	WSSocket is an abstract base class, implementing most of the functionality of
	Winsock socket communication.  Do not instantiate a WSSocket directly.  Instead
	instantiate a TCPSocket, UDPSocket, IPXSocket, or SPXSocket, depending on the
	type of socket you need.

Notes:
	- Illegal to issue another Open() if Close() has not completed.
	- Illegal to Accept() when there is already an Accept() pending

	12/30/98	-	Colen Garoutte-Carson	- Initial functionality

********************************************************************************/

#ifndef __WON_WSSOCKET_H__
#define __WON_WSSOCKET_H__


// Disable debug STL warnings
#pragma warning(disable : 4786)


#include <set>
#include <map>
#include <list>
#include "WONWS.h"
#include "WONSocket.h"
#include "SDKCommon/Completion.h"
#include "common/CriticalSection.h"
#include "common/Event.h"
#include "Errors.h"
#ifdef _LINUX
#include "common/linuxGlue.h"
#endif

// Disable debug STL warnings
#pragma warning(disable : 4786)


namespace WONAPI {


class WSSocket : public Socket
{
private:
	bool connectionless;

public:
	struct AcceptResult : public Socket::Result
	{
		WSSocket*		acceptedSocket;

		AcceptResult(WSSocket* sock, WSSocket* acptSock) : Result(sock), acceptedSocket(acptSock) {}
	};


protected:
	void PrepareForDestruction();
public:
	virtual ~WSSocket();
	
	
private:
	// Private Socket data.
	// SocketOp's are elements in an internal queue
	class AsyncOp;
	struct ltAsyncOp
	{
		bool operator()(const AsyncOp* asyncOp1, const AsyncOp* asyncOp2) const;
	};
	typedef std::map<SOCKET, WSSocket*> PendingMap;
	typedef std::multiset<AsyncOp*, ltAsyncOp> TimeoutSet;
	typedef std::list<AsyncOp*> OpList;
	
	class SocketOp
	{
	public:
		SocketOp*			nextCompleted;
		WSSocket*			obj;
		bool				triggered;
		bool				autoDelete;
		bool				deferCompletion;
		bool				closed;
		WONCommon::Event	doneEvent;

		SocketOp(WSSocket* theSocket)	:	autoDelete(false),
											triggered(false),
											deferCompletion(true),
											obj(theSocket),
											closed(false)
		{ }

		virtual ~SocketOp();
		virtual void Trigger();
		virtual void Complete() = 0;
	};
	friend class SocketOp;

	class CatcherOp : public SocketOp
	{
	public:
		WONCommon::CriticalSection			crit;
		CompletionContainer<const Result&>	catcherCompletion;

		CatcherOp(WSSocket& theSocket)
			:	SocketOp(&theSocket)
		{ }

		virtual void Trigger();
		virtual void Complete();
	};
	friend class CatcherOp;

	class RecvCatcherOp : public CatcherOp
	{
	public:
		RecvCatcherOp(WSSocket& theSocket)
			:	CatcherOp(theSocket)
		{ }

		virtual void Trigger();
		virtual void Complete();
	};
	friend class RecvCatcherOp;

	class AcceptCatcherOp : public CatcherOp
	{
	public:
		AcceptCatcherOp(WSSocket& theSocket)
			:	CatcherOp(theSocket)
		{ }

		virtual void Trigger();
		virtual void Complete();
	};
	friend class AcceptCatcherOp;

	class CloseCatcherOp : public CatcherOp
	{
	public:
		CloseCatcherOp(WSSocket& theSocket)
			:	CatcherOp(theSocket)
		{ }

		virtual void Trigger();
		virtual void Complete();
	};
	friend class CloseCatcherOp;

	class CloseRecvCatcherOp : public CatcherOp
	{
	public:
		CloseRecvCatcherOp(WSSocket& theSocket)
			:	CatcherOp(theSocket)
		{ }

		virtual void Trigger();
		virtual void Complete();
	};
	friend class CloseRecvCatcherOp;

	class AsyncOp : public SocketOp	// private data for an single async socket operation
	{
	public:
		unsigned long			timeStarted;
		long					timeout;	// in milliseconds
		TimeoutSet::iterator	timeoutItor;
		OpList::iterator		queueItor;

		AsyncOp(WSSocket* theSocket)
			:	SocketOp(theSocket)
		{ }

		virtual bool Process();
		virtual void Timeout();
		virtual void Trigger();
	};
	friend class AsyncOp;
	class TimerOp : public AsyncOp
	{
	public:
		CompletionContainer<void>	timerCompletion;
		
		TimerOp()	:	AsyncOp(0)
		{
			autoDelete = true;
		}

		virtual void Complete();
	};


	class OpenOp : public AsyncOp
	{
	public:
		Error									error;
		CompletionContainer<const OpenResult&>	openCompletion;

		OpenOp(WSSocket& theSocket)
			:	AsyncOp(&theSocket)
		{ }

		virtual void Trigger();
		virtual void Timeout();
		virtual void Complete();
	};
	friend class OpenOp;

	class AcceptOp : public AsyncOp
	{
	public:
		CompletionContainer<const AcceptResult&>	acceptCompletion;
		WSSocket*									acceptToSocket;
		WSSocket*									resultSocket;
		
		AcceptOp(WSSocket& theSocket)
			:	AsyncOp(&theSocket)
		{ }

		virtual bool Process();
		virtual void Trigger();
		virtual void Complete();
	};
	friend class AcceptOp;

	class TransmitOp : public AsyncOp
	{
	public:
		TransmitOp(WSSocket& theSocket)
			:	AsyncOp(&theSocket)
		{ }

		CompletionContainer<const TransmitResult&>	recvSendCompletion;
		unsigned long								bufSize;
		unsigned long								bufUsed;
		void*										buffer;
		bool										ownsBuffer;


		virtual void Complete();
	};
	friend class TransmitOp;

	class RecvOp : public TransmitOp
	{
	public:
		Address*	addr;

		RecvOp(WSSocket& theSocket)
			:	TransmitOp(theSocket)
		{ }

		virtual void Trigger();
		virtual bool Process();
	};
	friend class RecvOp;

	class SendOp : public TransmitOp
	{
	public:
		const Address*	addr;
		void*			actualBuffer;

		SendOp(WSSocket& theSocket)
			:	TransmitOp(theSocket)
		{ }

		virtual void Trigger();
		virtual bool Process();
	};
	friend class SendOp;

	class CloseOp : public AsyncOp
	{
	public:
		CompletionContainer<const Result&>	closeCompletion;

		CloseOp(WSSocket& theSocket)
			:	AsyncOp(&theSocket)
		{ }

		virtual void Complete();
		virtual void Trigger();
		virtual void Timeout();
	};
	friend class CloseOp;

	class CloseRecvOp : public CloseOp
	{
	public:
		CloseRecvOp(WSSocket& theSocket)
			:	CloseOp(theSocket)
		{ }

		virtual void Trigger();
		virtual void Timeout();
		virtual bool Process();
	};
	friend class CloseRecvOp;

	class CloseSendOp : public CloseOp
	{
	public:
		CloseSendOp(WSSocket& theSocket)
			:	CloseOp(theSocket)
		{ }

		virtual void Trigger();
		virtual void Timeout();
		virtual bool Process();
	};
	friend class CloseSendOp;

	OpenOp*			pendingOpenOp;
	CloseSendOp*	pendingCloseSendOp;
	CloseRecvOp*	pendingCloseRecvOp;

	OpList sendQueue;
	OpList recvQueue;
	OpList closeQueue;

	static bool interrupting;
	static SOCKET interruptSocket;
	static void DrainInterruptSocket();
	static bool ValidateInterruptSocket();
	static void DeleteInterruptSocket();

#ifdef _MTAPI

#ifdef _LINUX
	static void* SpinThread(void* unused);
	static void* CompletionThread(void* unused);
#elif defined(WIN32)
	static unsigned int __stdcall SpinThread(void* unused);
	static unsigned int __stdcall CompletionThread(void* unused);
#else
// lib does not support threading on this platform
#endif

#ifdef WIN32
	static DWORD completionThreadID;
	static DWORD spinThreadID;
	static HANDLE completionThreadHandle;
	static HANDLE spinThreadHandle;
#elif defined(_LINUX)
	static pthread_t spinThread;
	static pthread_t completionThread;
#endif
#endif

	static bool killSpinThread;

	PendingMap::iterator		recvPendingItor;
	PendingMap::iterator		sendPendingItor;
	PendingMap::iterator		openPendingItor;

	AcceptCatcherOp*	acceptCatcherOp;
	CatcherOp*			closeCatcherOp;
	CatcherOp*			closeRecvCatcherOp;
	RecvCatcherOp*		recvCatcherOp;

	bool				recvCatcherQueued;
	bool				acceptCatcherQueued;
	bool				closeCatcherQueued;
	bool				closeRecvCatcherQueued;

	static void InterruptCompletionThread();

	WONCommon::Event completionTracker;
	int numCompletesPending;
	bool alreadyPreparedForDelete;
	
	int sendBufferSize;
	int recvBufferSize;

	static SocketOp*		firstCompleted;
	static SocketOp*		lastCompleted;

	Address destAddr;
	Address ourAddr;

public:
	class Statics	// so all static objects are destructed at the same time
	{
	public:
#ifdef WIN32
		Winsock				winsock;
#endif
		WONCommon::CriticalSection		crit;
		WONCommon::CriticalSection		spinCrit;
		PendingMap			pendingOpens;
		PendingMap			pendingRecvs;
		PendingMap			pendingSends;
		TimeoutSet			timeoutSet;
		OpList				completedQueue;
		WONCommon::Event	completionThreadEvent;
		WONCommon::Event	spinThreadStartEvent;

		// This object will destruct before subordinate statics
		~Statics()
		{
			WSSocket::StopSpinThread();
		}
	};
	friend class Statics;
	static Statics* statics;
	static bool AllocateStatics(bool inc = true);
	static bool DeallocateStatics();

	class AutoConstructStatics
	{
	protected:
		static bool alloced;
	public:
		void DoAlloc()
		{
			if (!alloced)
				alloced = WSSocket::AllocateStatics(false);
		}

		~AutoConstructStatics()
		{
			if (alloced)
				WSSocket::DeallocateStatics();
		}
	};
	friend class AutoConstructStatics;
//	static AutoConstructStatics autoConstructStatics;
protected:

	bool opening;
	bool closing;	// true only if graceful (!force) close() is in progress
	bool listening;
	static bool selecting;
	bool recvCatcherInstalled;	// used by accept catcher as well
//	bool autoDelete;

	bool sendClosing;
	bool recvClosing;

	bool sendClosed;
	bool recvClosed;

	unsigned long pendingRecvBytes;

	SOCKET sock;
	const int addressFamily;
	const int type;
	const int protocol;
	bool closeSendOnRecvClose;

	static void SpinOrInterruptSpinThread(long timeout, SocketOp* socketOp);
	static void DoCompletions();
	static void Spin(long timeout, SocketOp* socketOp, bool justOnce = false);

	void SendChannelClosed();
	void RecvChannelClosed();
	void CloseAllSends();
	void CloseAllRecvs();
	static void Trigger(SocketOp* thisOp);
	static void Complete(OpList& opList);
	void SocketClosed();

	static bool StartSpinThread();		// Returns false on failure (very unlikely)
	static void StopSpinThread();

	// protected contructor.  Don't construct Socket directly.  Use subclasses.
	WSSocket(const Address& destAddress, int theAddressFamily, int theType, int theProtocol, bool cnnctionless, bool closeSendOnRecvClosure, int recvBufSize, int sendBufSize);

	unsigned long _Send(unsigned long count, const void* buffer, const Address* sendToAddr, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION );
	unsigned long _Recv(unsigned long count, void* buffer, Address* recvFromAddr, long timeout = -1, bool async = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION );

public:

  SOCKET GetSocket() { return (sock); }

	static void InterruptSpin();	// kicks select

//	static void CheckCompleted();

#ifdef WIN32
	static Winsock* GetWinsock();
#endif

	void CatchAcceptable(const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION, bool reuse = false);
	// Only one accept completion can be installed at any given time.
	// Calling Accept sets/replaces that completion.

	virtual Error Open(const Address* addr = 0, long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION );

	Error Open(const Address& addr, long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION )
	{ return Open(&addr, timeout, async, completion); }

	

	const Address& GetRemoteAddress()	{ return destAddr; }
	const Address& GetLocalAddress()	{ return ourAddr; }

	void SetRemoteAddress(const Address& addr)	{ destAddr = addr; }

	virtual bool IsOpen();
	virtual bool IsSendOpen();
	virtual bool IsRecvOpen();

	Error Listen(const Address& addr);
	WSSocket* Accept(WSSocket& socketToConnect, long timeout = -1, bool async = false, const CompletionContainer<const AcceptResult&>& completion = DEFAULT_COMPLETION );

	// CloseSend() closes only the send channel.  IsOpen() will return true if recv channel is open
	// CloseRecv() closes only the recv channel.  IsOpen() will return true if send channel is open
	// Close() closes both send and recv channels
	//
	// A forceful (timeout == 0) Close(), CloseSend(), or CloseRecv(), will abort all pending sends
	//		and/or recvs.
	//
	// A graceful (timeout != 0) Close(), CloseSend(), or CloseRecv(), will wait for pending sends
	//		and/or recvs to complete.  Subsequent sends and/or recvs will be refused.
	//		
	// A graceful Close(), CloseSend(), or CloseRecv(), with a timeout >0 will issue an abortive
	//		close if pending sends and/or recvs do not complete before the timeout expires.
	//
	// A graceful Close(), CloseSend(), or CloseRecv(), with a timeout of -1 will not complete until
	//		all pending sends and/or recvs have completed.  (Once local send and/or recv queues have
	//		been drained, graceful close is accomplished by issueing a closesocket().  The Socket object
	//		will be closed, and reusable, and Winsock will assume the responsibility of completing
	//		the rest of the graceful close.)
	//
	// Note: It is possible to close the send channel, and continue to monitor the recv channel for
	//		incoming data.  However, this will cause the remote socket's recv channel to close, which
	//		is usually interpreted as a graceful close initiation.  Some servers may assume the socket
	//		is closing out from under them, and fail to return any more data.  Unless you're certain
	//		of the remote behavior, it is safest to postpone closing of the send channel until all
	//		expected data has been received (at which time, you might as well close both channels).
	//
	//		Closing the recv channel does not cause the remote socket's send channel to close.  If you
	//		close the recv channel, and data subsequently arrives, the socket will be 'reset', causing
	//		the send channel to also close.  Unless you're certain of the remote behavior, it is safest
	//		to postpone the closing of the recv channel until you're done with the send channel (at
	//		which time, you might as well close both channels).
	
	virtual void CloseSend(long timeout = -1, bool async = false, const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION );

	virtual void CloseRecv(long timeout = -1, bool async = false, const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION );

	virtual void Close(long timeout = -1, bool async = false, const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION );
	
	
	// If a graceful close is already in progress, a new graceful close on that channel will only
	// reset the timeout.  Forced closes will always occur immediately.  If the socket is already closed,
	// close operations will complete immediately.

//	void SetAutoDelete(bool setTo = true)	{ autoDelete = setTo; }
//	bool& GetAutoDelete()					{ return autoDelete;  }

	virtual void CatchClose(const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION , bool reuse = false);
	// CatchClose() can be used to establish a single completion to be Complete()'ed once
	// the socket closes (both channels).  Only one such completion can be set.  Specifying
	// a new completion replaces the previously set completion.  Specifying a NULL completion
	// will remove an existing completion.

	virtual void CatchCloseRecv(const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION , bool reuse = false);
	// CatchCloseRecv() can be used to establish a single completion to be Complete()'ed once
	// the recv channel of a socket closes.  Only one such completion can be set.  Specifying
	// a new completion replaces the previously set completion.  Specifying a NULL completion
	// will remove an existing completion.

	virtual void CatchRecvable(const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION , bool reuse = false);
	// CatchRecvable() can be used to establish a single completion to be Complete()'ed once
	// data arrives on the socket, and there are no pending recv operations.  Only one such
	// completion can be set.  Specifying a new completion replaces the previously set
	// completion.  Specifying a NULL completion

	virtual unsigned long Send(unsigned long count, const void* buffer, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION );
	virtual unsigned long SendTo(unsigned long count, const void* buffer, const Address& sendToAddr, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION );

	virtual unsigned long Recv(unsigned long count, void* buffer, long timeout = -1, bool async = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION );
	virtual unsigned long RecvFrom(unsigned long count, void* buffer, Address* recvFromAddr, long timeout = -1, bool async = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION );

	virtual unsigned long Available();

	virtual bool IsRecving();
	virtual bool IsSending();

	bool IsAccepting()	{ return (listening) ? !sendQueue.empty() : false; }	// uses sendQueue on listening socket

	virtual bool AbortRecv();
	virtual bool AbortSend();

	static bool InstallTimer(unsigned long timeout, const CompletionContainer<void>& completion);

	// synonyms
	template <class privsType>
	Error OpenEx(const Address* addr, long timeout, bool async, void (*f)(const OpenResult&, privsType), privsType t)
	{ return Open(addr, timeout, async, new CompletionWithContents<const OpenResult&, privsType>(t, f, true)); }

	template <class privsType>
	Error OpenEx(const Address& addr, long timeout, bool async, void (*f)(const OpenResult&, privsType), privsType t)
	{ return Open(&addr, timeout, async, new CompletionWithContents<const OpenResult&, privsType>(t, f, true)); }


	template <class privsType>
	void CloseEx(long timeout, bool async, void (*f)(const Result&, privsType), privsType t)
	{ Close(timeout, async, new CompletionWithContents<const Result&, privsType>(t, f, true)); }


	template <class privsType>
	void CloseSendEx(long timeout, bool async, void (*f)(const Result&, privsType), privsType t)
	{ CloseSend(timeout, async, new CompletionWithContents<const Result&, privsType>(t, f, true)); }


	template <class privsType>
	void CloseRecvEx(long timeout, bool async, void (*f)(const Result&, privsType), privsType t)
	{ CloseRecv(timeout, async, new CompletionWithContents<const Result&, privsType>(t, f, true)); }


	template <class privsType>
	void CatchCloseEx(void (*f)(const Result&, privsType), privsType t, bool reuse = false)
	{ CatchClose(new CompletionWithContents<const Result&, privsType>(t, f, true), reuse); }


	template <class privsType>
	void CatchCloseRecvEx(void (*f)(const Result&, privsType), privsType t, bool reuse = false)
	{ CatchCloseRecv(new CompletionWithContents<const Result&, privsType>(t, f, true), reuse); }


	template <class privsType>
	void CatchRecvableEx(void (*f)(const Result&, privsType), privsType t, bool reuse = false)
	{ CatchRecvable(new CompletionWithContents<const Result&, privsType>(t, f, true), reuse); }

	template <class privsType>
	unsigned long SendEx(unsigned long count, const void* buffer, long timeout, bool async, bool copyData, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return Send(count, buffer, timeout, async, copyData, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }

	template <class privsType>
	unsigned long SendEx(unsigned long count, const void* buffer, const Address& addr, long timeout, bool async, bool copyData, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return Send(count, buffer, addr, timeout, async, copyData, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }

	template <class privsType>
	unsigned long SendToEx(unsigned long count, const void* buffer, const Address& addr, long timeout, bool async, bool copyData, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return SendTo(count, buffer, addr, timeout, async, copyData, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }

	template <class privsType>
	unsigned long RecvEx(unsigned long count, void* buffer, long timeout, bool async, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return Recv(count, buffer, timeout, async, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }

	template <class privsType>
	unsigned long RecvEx(unsigned long count, void* buffer, Address* addr, long timeout, bool async, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return Recv(count, buffer, addr, timeout, async, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }

	template <class privsType>
	unsigned long RecvFromEx(unsigned long count, void* buffer, Address* addr, long timeout, bool async, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return RecvFrom(count, buffer, addr, timeout, async, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }

	template <class privsType>
	unsigned long SkipEx(unsigned long count, long timeout, bool async, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return Skip(count, 0, timeout, async, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }
	
	template <class privsType>
	unsigned long CatchAcceptableEx(void (*f)(const OpenResult&, privsType), privsType t, bool reuse = false)
	{ return CatchAcceptable(new CompletionWithContents<const OpenResult&, privsType>(t, f, true), reuse); }

	// If using single-thread runtime, must call Pump to make sure async functions are handled
	static void Pump(unsigned long timeout = 0);
	static bool PumpUntil(WONCommon::Event& evt, DWORD timeout = INFINITE);
#ifdef WIN32
	static bool PumpUntil(HANDLE obj, DWORD timeout = INFINITE);
#endif

	bool isConnectionless()	{ return connectionless; }
};

#ifdef WIN32
#define WON_SOCKET_CALL(func) WSSocket::GetWinsock()->func
#else
#define WON_SOCKET_CALL(func) func
#endif


};


#endif
