#ifndef __WON_UDPSOCKET_H__
#define __WON_UDPSOCKET_H__


#include "IPSocket.h"


namespace WONAPI {


class UDPSocket : public IPSocket
{
public:
	class Address : public IPSocket::Address
	{
	public:
		Address()
			:	IPSocket::Address()
		{ }

		Address(const WONMsg::DirEntity& dirEntity)	// extract WON std address from dirEntity
			:	IPSocket::Address(dirEntity)
		{ }

		Address(const sockaddr_in& theAddr)
			:	IPSocket::Address(theAddr)
		{ }

		Address(unsigned long ipaddr, unsigned short port)
			:	IPSocket::Address(ipaddr, port)
		{ }

		Address(const std::string& hostStr, unsigned short port)
			:	IPSocket::Address(hostStr, port)
		{ }

		Address(const char* hostStr, unsigned short port)
			:	IPSocket::Address(hostStr, port)
		{ }

		Address(const std::string& hostAndPortStr)
			:	IPSocket::Address(hostAndPortStr)
		{ }

		Address(const char* hostAndPortStr)
			:	IPSocket::Address(hostAndPortStr)
		{ }

		Address(unsigned short port)	// broadcast
			:	IPSocket::Address(INADDR_BROADCAST, port)
		{ }
	};


	UDPSocket(const IPSocket::Address& addr = Address(), int recvBufSize = 8192, int sendBufSize = 8192)
		:	IPSocket(addr, SOCK_DGRAM, IPPROTO_UDP, true, false, recvBufSize, sendBufSize)
	{ }

	~UDPSocket()
	{
		PrepareForDestruction();
	}

	const Address& GetRemoteAddress()
	{ return *(const Address*)&(IPSocket::GetRemoteAddress()); };

	const Address& GetLocalAddress()
	{ return *(const Address*)&(IPSocket::GetLocalAddress()); };

	virtual Type GetType();
};

};

#endif
