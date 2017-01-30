#ifndef __WON_TCPSOCKET_H__
#define __WON_TCPSOCKET_H__


#include "IPSocket.h"
#include "SDKCommon/Stream.h"


namespace WONAPI {


class TCPSocket : public IPSocket, public Stream
{
public:
	using IPSocket::Address;

	TCPSocket(const Address& addr = IPSocket::Address(), bool closeSendOnRecvClose = true, int recvBufSize = 8192, int sendBufSize = 8192)
	:	IPSocket(addr, SOCK_STREAM, IPPROTO_TCP, false, closeSendOnRecvClose, recvBufSize, sendBufSize)
	{ }

	~TCPSocket()
	{
		PrepareForDestruction();
	}

	virtual Type GetType();

	virtual unsigned long Write(unsigned long count, const void* buffer);
	virtual unsigned long Read(unsigned long count, void* buffer);

	TCPSocket* Accept(TCPSocket* socketToConnect = 0, long timeout = -1, bool async = false, const CompletionContainer<const AcceptResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	TCPSocket* AcceptEx(TCPSocket* socketToConnect, long timeout, bool async, void (*f)(const AcceptResult&, privsType), privsType t)
	{ return Accept(socketToConnect, timeout, async, new CompletionWithContents<const AcceptResult&, privsType>(t, f, true)); }
};


};

#endif
