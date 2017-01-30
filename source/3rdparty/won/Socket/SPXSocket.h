#ifndef __WON_SPXSOCKET_H__
#define __WON_SPXSOCKET_H__


#include "PXSocket.h"
#include "SDKCommon/Stream.h"
#include <iostream>


#pragma warning(disable : 4786)


namespace WONAPI {


class SPXSocket : public PXSocket, public Stream
{
public:	
	using PXSocket::Address;

	SPXSocket(const Address& addr = PXSocket::Address(), bool closeSendOnRecvClose = true, int recvBufSize = 0x0000FFFF, int sendBufSize = 0x0000FFFF)
	:	PXSocket(addr, SOCK_STREAM, NSPROTO_SPX, false, closeSendOnRecvClose, recvBufSize, sendBufSize)
	{ }

	~SPXSocket()
	{
		PrepareForDestruction();
	}

	virtual Type GetType();

	virtual unsigned long Write(unsigned long count, const void* buffer);
	virtual unsigned long Read(unsigned long count, void* buffer);

	SPXSocket* Accept(SPXSocket* socketToConnect = 0, long timeout = -1, bool async = false, const CompletionContainer<const AcceptResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	SPXSocket* AcceptEx(SPXSocket* socketToConnect, long timeout, bool async, void (*f)(const AcceptResult&, privsType), privsType t)
	{ return Accept(socketToConnect, timeout, async, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }
};


};

#endif
