#ifndef __WON_PXSOCKET_H__
#define __WON_PXSOCKET_H__


// Disable debug STL warnings
#pragma warning(disable : 4786)

#include <list>
#include "WSSocket.h"

#ifdef WIN32
#include <wsipx.h>
#endif

// Disable debug STL warnings
#pragma warning(disable : 4786)


namespace WONAPI {


class PXSocket : public WSSocket
{
public:
	class Address : public Socket::Address
	{
	public:
		class NodeNum
		{
		private:
			char val[6];

		public:
			NodeNum(const unsigned char* nodeNum)		{ val[0] = nodeNum[0]; val[1] = nodeNum[1]; val[2] = nodeNum[2]; val[3] = nodeNum[3]; val[4] = nodeNum[4]; val[5] = nodeNum[5]; }
			NodeNum(const char* nodeNum)				{ val[0] = nodeNum[0]; val[1] = nodeNum[1]; val[2] = nodeNum[2]; val[3] = nodeNum[3]; val[4] = nodeNum[4]; val[5] = nodeNum[5]; }
			NodeNum(const NodeNum& nodeNum)				{ val[0] = nodeNum.val[0]; val[1] = nodeNum.val[1]; val[2] = nodeNum.val[2]; val[3] = nodeNum.val[3]; val[4] = nodeNum.val[4]; val[5] = nodeNum.val[5]; }
			NodeNum(BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5, BYTE b6)
														{ val[0] = b1; val[1] = b2; val[2] = b3; val[3] = b4; val[4] = b5; val[5] = b6; }
			NodeNum& operator=(const char* nodeNum)		{ val[0] = nodeNum[0]; val[1] = nodeNum[1]; val[2] = nodeNum[2]; val[3] = nodeNum[3]; val[4] = nodeNum[4]; val[5] = nodeNum[5]; return *this; }
			NodeNum& operator=(const NodeNum& nodeNum)	{ val[0] = nodeNum.val[0]; val[1] = nodeNum.val[1]; val[2] = nodeNum.val[2]; val[3] = nodeNum.val[3]; val[4] = nodeNum.val[4]; val[5] = nodeNum.val[5]; return *this; }
			operator==(const unsigned char* nodeNum)	{ return memcmp(val, nodeNum, 6) == 0; }
			operator==(const char* nodeNum)				{ return memcmp(val, nodeNum, 6) == 0; }
			operator==(const NodeNum& nodeNum)			{ return memcmp(val, &(nodeNum.val), 6) == 0; }
			operator!=(const unsigned char* nodeNum)	{ return memcmp(val, nodeNum, 6) != 0; }
			operator!=(const char* nodeNum)				{ return memcmp(val, nodeNum, 6) != 0; }
			operator!=(const NodeNum& nodeNum)			{ return memcmp(val, &(nodeNum.val), 6) != 0; }
		
			friend class Address;
		};

	private:
		const static sockaddr blank;

		static sockaddr Make_sockaddr_ipx(unsigned short socketNum, const NodeNum& nodeNum, unsigned long netNum)
		{
			sockaddr_ipx ipxaddr;
#ifdef _LINUX
			ipxaddr.sipx_port = socketNum;
			ipxaddr.sipx_node[0] = nodeNum.val[0];
			ipxaddr.sipx_node[1] = nodeNum.val[1];
			ipxaddr.sipx_node[2] = nodeNum.val[2];
			ipxaddr.sipx_node[3] = nodeNum.val[3];
			ipxaddr.sipx_node[4] = nodeNum.val[4];
			ipxaddr.sipx_node[5] = nodeNum.val[5];
			ipxaddr.sipx_network = netNum;
#elif defined(WIN32)
			ipxaddr.sa_socket = socketNum;
			ipxaddr.sa_nodenum[0] = nodeNum.val[0];
			ipxaddr.sa_nodenum[1] = nodeNum.val[1];
			ipxaddr.sa_nodenum[2] = nodeNum.val[2];
			ipxaddr.sa_nodenum[3] = nodeNum.val[3];
			ipxaddr.sa_nodenum[4] = nodeNum.val[4];
			ipxaddr.sa_nodenum[5] = nodeNum.val[5];
			*(unsigned long*)(ipxaddr.sa_netnum) = netNum;
#endif
			return *(sockaddr*)&ipxaddr;
		}

	public:

		Address()
			:	Socket::Address(blank)
		{ }
		Address(const sockaddr_ipx& theAddr)
			:	Socket::Address(*(sockaddr*)&theAddr)
		{ }

		Address(unsigned short socketNum, const NodeNum& nodeNum, unsigned long netNum = 0)
			:	Socket::Address(Make_sockaddr_ipx(socketNum, nodeNum, netNum))
		{ }
		
		sockaddr_ipx& Get()							{ return *(sockaddr_ipx*)&(((Socket::Address*)this)->Get()); };
		sockaddr_ipx Get() const					{ return *(sockaddr_ipx*)&(((Socket::Address*)this)->Get()); };
		void Set(const sockaddr_ipx& theAddr)		{ ((Socket::Address*)this)->Set(*(sockaddr*)&theAddr); }
		void SetSocket(unsigned short socketNum)
		{
#ifdef WIN32
			Get().sa_socket = socketNum;
#elif defined(_LINUX)
			Get().sipx_port = socketNum;
#endif
		}
		void SetNode(const NodeNum& nodeNum)
		{
			sockaddr_ipx& a = Get(); 
#ifdef WIN32
			a.sa_nodenum[0] = nodeNum.val[0];
			a.sa_nodenum[1] = nodeNum.val[1];
			a.sa_nodenum[2] = nodeNum.val[2];
			a.sa_nodenum[3] = nodeNum.val[3];
			a.sa_nodenum[4] = nodeNum.val[4];
			a.sa_nodenum[5] = nodeNum.val[5];
#elif defined(_LINUX)
			a.sipx_node[0] = nodeNum.val[0];
			a.sipx_node[1] = nodeNum.val[1];
			a.sipx_node[2] = nodeNum.val[2];
			a.sipx_node[3] = nodeNum.val[3];
			a.sipx_node[4] = nodeNum.val[4];
			a.sipx_node[5] = nodeNum.val[5];
#endif
		}
		void SetNet(unsigned long netNum)
		{
			sockaddr_ipx& a = Get();
#ifdef WIN32
			*(unsigned long*)(&(a.sa_netnum)) = netNum;
#elif defined(_LINUX)
			a.sipx_network = netNum;
#endif
		}

		unsigned short GetSocket() const
		{
#ifdef WIN32
			return Get().sa_socket;
#elif defined(_LINUX)
			return Get().sipx_port;
#endif
		}
		unsigned long GetNet() const
		{
#ifdef WIN32
			return *(unsigned long*)(&(Get().sa_netnum));
#elif defined(_LINUX)
			return *(unsigned long*)(&(Get().sipx_network));
#endif
		};
		NodeNum GetNode() const
		{
#ifdef WIN32
			return NodeNum(Get().sa_nodenum);
#elif defined(_LINUX)
			return NodeNum(Get().sipx_node);
#endif
		};

	};
	friend class Address;

protected:
	PXSocket(const Address& addr, int theType, int theProtocol, bool cnnctionless, bool closeSendOnRecvClose, int recvBufSize, int sendBufSize)
		:	WSSocket(addr, PF_IPX, theType, theProtocol, cnnctionless, closeSendOnRecvClose, recvBufSize, sendBufSize)
	{ }

public:
	Error Listen(unsigned short socketNum);
	Error Listen(const Address& addr)
	{ return WSSocket::Listen(addr); }

	const Address& GetRemoteAddress()
	{ return *(const Address*)&(PXSocket::GetRemoteAddress()); };

	const Address& GetLocalAddress()
	{ return *(const Address*)&(PXSocket::GetLocalAddress()); };


	Error Open(const Address* addr = 0, long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION )
	{ return WSSocket::Open(addr, timeout, async, completion); }

	Error Open(const Address& addr, long timeout = -1, bool async = false, const CompletionContainer<const OpenResult&>& completion = DEFAULT_COMPLETION )
	{ return Open(&addr, timeout, async, completion); }

	template <class privsType>
	Error OpenEx(const Address* addr, long timeout, bool async, void (*f)(const OpenResult&, privsType), privsType t)
	{ return Open(addr, timeout, async, new CompletionWithContents<const OpenResult&, privsType>(t, f, true)); }

	template <class privsType>
	Error OpenEx(const Address& addr, long timeout, bool async, void (*f)(const OpenResult&, privsType), privsType t)
	{ return Open(&addr, timeout, async, new CompletionWithContents<const OpenResult&, privsType>(t, f, true)); }


	unsigned long SendTo(unsigned long count, const void* buffer, const Address& addr, long timeout = -1, bool async = false, bool copyData = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION )
	{ return WSSocket::SendTo(count, buffer, addr, timeout, async, copyData, completion); }

	template <class privsType>
	unsigned long SendToEx(unsigned long count, const void* buffer, const Address& addr, long timeout, bool async, bool copyData, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return SendTo(count, buffer, timeout, async, copydata, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }


	unsigned long RecvFrom(unsigned long count, void* buffer, Address* addr, long timeout = -1, bool async = false, const CompletionContainer<const TransmitResult&>& completion = DEFAULT_COMPLETION )
	{ return WSSocket::RecvFrom(count, buffer, addr, timeout, async, completion); }

	template <class privsType>
	unsigned long RecvFromEx(unsigned long count, void* buffer, Address* addr, long timeout, bool async, void (*f)(const TransmitResult&, privsType), privsType t)
	{ return RecvFrom(count, buffer, addr, timeout, async, new CompletionWithContents<const TransmitResult&, privsType>(t, f, true)); }
};



};

#endif
