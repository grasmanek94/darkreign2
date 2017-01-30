#ifndef __WON_IPXSOCKET_H__
#define __WON_IPXSOCKET_H__


#include "PXSocket.h"


namespace WONAPI {


#if !defined(macintosh) || (macintosh != 1)


class IPXSocket : public PXSocket
{
public:
	class Address : public PXSocket::Address
	{
	public:
		Address()
			:	PXSocket::Address()
		{ }

		Address(const sockaddr_ipx& theAddr)
			:	PXSocket::Address(theAddr)
		{ }

		Address(unsigned short socketNum)
			:	PXSocket::Address(socketNum, *(NodeNum*)"\xFF\xFF\xFF\xFF\xFF\xFF")	// broadcast
		{ }

		Address(unsigned short socketNum, const NodeNum& nodeNum, unsigned long netNum = 0)
			:	PXSocket::Address(socketNum, nodeNum, netNum)
		{ }
	};

	IPXSocket(const PXSocket::Address& addr = Address(), int recvBufSize = 0x0000FFFF, int sendBufSize = 0x0000FFFF)
		:	PXSocket(addr, SOCK_DGRAM, NSPROTO_IPX, true, false, recvBufSize, sendBufSize)
	{ }

	~IPXSocket()
	{
		PrepareForDestruction();
	}

	virtual Type GetType();

	const Address& GetRemoteAddress()
	{ return *(const Address*)&(PXSocket::GetRemoteAddress()); };

	const Address& GetLocalAddress()
	{ return *(const Address*)&(PXSocket::GetLocalAddress()); };
};


#endif


};

#endif
