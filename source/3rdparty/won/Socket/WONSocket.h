/********************************************************************************

  WONAPI::Socket

	Socket is an abstract base class, implementing most of the functionality of
	network socket communication.  Do not instantiate a Socket directly.  Instead
	instantiate a TCPSocket, UDPSocket, IPXSocket, or SPXSocket, depending on the
	type of socket you need.

Notes:
	- Illegal to issue another Open() if Close() has not completed.
	- Illegal to Accept() when there is already an Accept() pending

	12/30/98	-	Colen Garoutte-Carson	- Initial functionality

********************************************************************************/

#ifndef __WON_SOCKET_H__
#define __WON_SOCKET_H__


#pragma warning(disable : 4786)


#include "common/won.h"
#include "WONWS.h"
#include "SDKCommon/Completion.h"
#include "Errors.h"


namespace WONAPI {


class Socket
{
public:
	// Base Address object, used as a base class for protocol-specific addresses
	// Copied by value, don't derive with member data or virtual members
	class Address
	{
	protected:
		sockaddr addr;
		const static sockaddr blank;

	public:
		Address()
			:	addr(blank)
		{ }
		Address(const sockaddr& theAddr)
			:	addr(theAddr)
		{ }
		Address(const Address& theAddr)
			:	addr(theAddr.addr)
		{ }

		sockaddr& Get()						{ return addr; };
		const sockaddr& Get() const			{ return addr; };
		void Set(const sockaddr& theAddr)	{ addr = theAddr; }
		void Set(const Address& theAddr)	{ addr = theAddr.addr; }

		int Compare(const Address& theOtherR) const
		{ return memcmp(this, &(theOtherR.addr), sizeof(sockaddr)); }

		friend bool operator==(const Address& address1, const Address& address2)
		{ return address1.Compare(address2) == 0; }
		
		friend bool operator!=(const Address& address1, const Address& address2)
		{ return address1.Compare(address2) != 0; }

		friend bool operator>(const Address& address1, const Address& address2)
		{ return address1.Compare(address2) > 0; }

		friend bool operator<(const Address& address1, const Address& address2)
		{ return address1.Compare(address2) < 0; }

		friend bool operator>=(const Address& address1, const Address& address2)
		{ return address1.Compare(address2) >= 0; }

		friend bool operator<=(const Address& address1, const Address& address2)
		{ return address1.Compare(address2) <= 0; }
	};

	virtual ~Socket();

	enum Type {
		stream = 1,
		datagram = 2
	};

	virtual Type GetType() = 0;

	// Result objects are passed to completions, to provide additional information about
	// the IO operation.
	struct Result
	{
		Socket*		theSocket;

		Result(Socket* sock) : theSocket(sock) {}
	};

	struct OpenResult : public Result
	{
		Error	error;

		OpenResult(Socket* sock, Error err) : Result(sock), error(err) {}
	};

	struct TransmitResult : public Result
	{
		void*			buffer;	// would be null if socket owns buffer
		unsigned long	bytesRequested;
		unsigned long	bytesUsed;
		bool			closed;

		TransmitResult(Socket* sock, void* buf, unsigned long req, unsigned long used, bool socketClosed = false) : Result(sock), buffer(buf), bytesRequested(req), bytesUsed(used), closed(socketClosed) {}
	};

public:
	virtual Error Open(const Address* addr = 0, long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION ) = 0;

	Error Open(const Address& addr, long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION )
	{ return Open(&addr, timeout, async, completion); }

	virtual bool IsOpen() = 0;
	virtual bool IsSendOpen() = 0;
	virtual bool IsRecvOpen() = 0;

	virtual void CloseSend(long timeout = -1, bool async = false, const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION ) = 0;

	virtual void CloseRecv(long timeout = -1, bool async = false, const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION ) = 0;

	virtual void Close(long timeout = -1, bool async = false, const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION ) = 0;

	virtual void CatchClose(const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION , bool reuse = false) = 0;
	virtual void CatchCloseRecv(const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION , bool reuse = false) = 0;
	virtual void CatchRecvable(const CompletionContainer<const Result&>& completion = DEFAULT_COMPLETION , bool reuse = false) = 0;


	virtual unsigned long Send(unsigned long count, const void* buffer, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION ) = 0;
	virtual unsigned long SendTo(unsigned long count, const void* buffer, const Address& sendToAddr, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION ) = 0;

	virtual unsigned long Recv(unsigned long count, void* buffer, long timeout = -1, bool async = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION ) = 0;
	virtual unsigned long RecvFrom(unsigned long count, void* buffer, Address* recvFromAddr, long timeout = -1, bool async = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION ) = 0;

	virtual bool AbortRecv() = 0;
	virtual bool AbortSend() = 0;
	virtual unsigned long Available() = 0;

	unsigned long Skip(unsigned long count, long timeout = -1, bool async = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION )
	{ return Recv(count, 0, timeout, async, completion); }

	virtual bool IsRecving() = 0;
	virtual bool IsSending() = 0;

	// synonyms
	template <class privsType>
	Error OpenEx(const Address* addr, long timeout, bool async, void (*f)(const OpenResult&, privsType), privsType t)
	{ return Open(addr, timeout, async, new CompletionWithContents<const OpenResult&, privsType>(t, f, true)); }

	template <class privsType>
	Error OpenEx(const Address& addr, long timeout, bool async, void (*f)(const OpenResult&, privsType), privsType t)
	{ return Open(&addr, timeout, async, new CompletionWithContents<const OpenResult&, privsType>(t, f, true)); }


	template <class privsType>
	void CloseSendEx(long timeout, bool async, void (*f)(const Result&, privsType), privsType t)
	{ CloseSend(timeout, async, new CompletionWithContents<const Result&, privsType>(t, f, true)); }


	template <class privsType>
	void CloseRecvEx(long timeout, bool async, void (*f)(const Result&, privsType), privsType t)
	{ CloseRecv(timeout, async, new CompletionWithContents<const Result&, privsType>(t, f, true)); }


	template <class privsType>
	void CloseEx(long timeout, bool async, void (*f)(const Result&, privsType), privsType t)
	{ Close(timeout, async, new CompletionWithContents<const Result&, privsType>(t, f, true)); }


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
	unsigned long SendToEx(unsigned long count, const void* buffer, const Address& addr, long timeout, bool async, bool copyData, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return SendTo(count, buffer, timeout, async, copyData, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }


	template <class privsType>
	unsigned long RecvEx(unsigned long count, void* buffer, long timeout, bool async, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return Recv(count, buffer, timeout, async, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }

	template <class privsType>
	unsigned long RecvFromEx(unsigned long count, void* buffer, Address* addr, long timeout, bool async, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return RecvFrom(count, buffer, addr, timeout, async, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }


	template <class privsType>
	unsigned long SkipEx(unsigned long count, long timeout, bool async, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return Skip(count, 0, timeout, async, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }
	
};

};


#endif
