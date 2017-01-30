#ifndef __WON_IPSOCKET_H__
#define __WON_IPSOCKET_H__


// Disable debug STL warnings
#pragma warning(disable : 4786)


#include "WSSocket.h"
#include <string>
#include "msg/Dir/DirEntity.h"
#include "wondll.h"

namespace WONAPI {


class IPSocket : public WSSocket
{
public:
	class Address : public Socket::Address
	{
	protected:
		const static sockaddr blank;
	public:
		Address()
			:	Socket::Address(blank)
		{ }

		Address(const WONMsg::DirEntity& dirEntity)	// extract WON std address from dirEntity
			:	Socket::Address(blank)
		{ Set(dirEntity); }

		Address(const WONIPAddress& ipAddr)
		{ Set(ipAddr); }

		Address(const sockaddr_in& theAddr)
			:	Socket::Address(*(sockaddr*)&theAddr)
		{ }

		Address(const in_addr& ipaddr, unsigned short port)
			:	Socket::Address(blank)
		{ Set(ipaddr.s_addr, port); }

		Address(unsigned long ipaddr, unsigned short port)
			:	Socket::Address(blank)
		{ Set(ipaddr, port); }

		Address(const std::string& hostStr, unsigned short port)
			:	Socket::Address(blank)
		{ Set(hostStr, port); }

		Address(const char* hostStr, unsigned short port)
			:	Socket::Address(blank)
		{ Set(std::string(hostStr), port); }

		Address(const std::string& hostAndPortStr)
			:	Socket::Address(blank)
		{ Set(hostAndPortStr); }

		Address(const char* hostAndPortStr)
			:	Socket::Address(blank)
		{ Set(std::string(hostAndPortStr)); }

		sockaddr_in& Get()									{ return *(sockaddr_in*)&(((Socket::Address*)this)->Get()); };
		const sockaddr_in& Get() const						{ return *(sockaddr_in*)&(((Socket::Address*)this)->Get()); };
		void Set(const sockaddr_in& theAddr)				{ ((Socket::Address*)this)->Set(*(sockaddr*)&theAddr); }

		void Set(const std::string& hostStr, unsigned short port);
		void Set(const std::string& hostAndOptionalPortStr);
		void Set(unsigned long ipaddr, unsigned short port);
		void Set(const WONMsg::DirEntity& dirEntity);	// extract WON std address from dirEntity
		void Set(const WONIPAddress& ipAddr);
		void Set_Port(unsigned short port);
		void SetPort(unsigned short port)	{ Set_Port(port); }

		Address& operator=(const char* hostAndOptionalPortStr)			{ Set(hostAndOptionalPortStr); return *this; }
		Address& operator=(const std::string& hostAndOptionalPortStr)	{ Set(hostAndOptionalPortStr); return *this; }
		Address& operator=(const WONMsg::DirEntity& dirEntity)			{ Set(dirEntity); return *this; }
		Address& operator=(const WONIPAddress& ipAddr)					{ Set(ipAddr); return *this; }


		// inet_addr returns INADDR_NONE on failure
		// Beware:  INADDR_NONE == INADDR_BROADCAST
		bool IsValid() const								{ return Get().sin_addr.s_addr != INADDR_NONE; }

		unsigned short GetPort() const;
		const in_addr& GetAddress() const					{ return Get().sin_addr; }
		std::string GetAddressString(bool addPort = true) const;
		std::string LookupHost(bool addPort = false) const;
		void InitFromLocalIP(unsigned short thePort);
	};
	friend class Address;

protected:
	IPSocket(const Address& addr, int theType, int theProtocol, bool cnnctionless, bool closeSendOnRecvClose = true, int recvBufSize = 8192, int sendBufSize = 8192)
		:	WSSocket(addr, PF_INET, theType, theProtocol, cnnctionless, closeSendOnRecvClose, recvBufSize, sendBufSize)
	{ }
	
public:
	Error Listen(unsigned short port = 0);	// If zero, a local port is assigned

	Error Listen(const Address& addr)
	{ return WSSocket::Listen(addr); }


	const Address& GetRemoteAddress()
	{ return *(const Address*)&(WSSocket::GetRemoteAddress()); };

	const Address& GetLocalAddress()
	{ return *(const Address*)&(WSSocket::GetLocalAddress()); };

	unsigned short GetLocalPort()
	{ return GetLocalAddress().GetPort(); }


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
