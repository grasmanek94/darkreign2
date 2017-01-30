#include <stdio.h>
#include "IPSocket.h"


using namespace WONAPI;
using std::string;


// Sets to INADDR_NONE
const sockaddr IPSocket::Address::blank = { AF_INET, 0, 0, (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF, 0, 0, 0, 0, 0, 0, 0, 0 };


static bool is_hostname(const char* str)
{
	const char* c = str;
	while (*c)
	{
		if ((*c != '.') && ((*c > '9') || (*c < '0')))
			return true;
		c++;
	}
	return false;
}

void IPSocket::Address::Set(const string& hostStr, unsigned short port)
{
	sockaddr_in& theAddr = Get();
	theAddr.sin_family = AF_INET;
	theAddr.sin_port = WON_SOCKET_CALL(htons)(port);

	const char* charPtr = hostStr.c_str();

	bool is_hn = is_hostname(charPtr);

#ifdef WIN32
	theAddr.sin_addr.S_un.S_addr = INADDR_NONE;
	if (!is_hn)
		theAddr.sin_addr.S_un.S_addr = WON_SOCKET_CALL(inet_addr)(charPtr);

	if (theAddr.sin_addr.S_un.S_addr == INADDR_NONE)
#else//if defined(_LINUX)
	theAddr.sin_addr.s_addr = INADDR_NONE;
	if (!is_hn)
		theAddr.sin_addr.s_addr = WON_SOCKET_CALL(inet_addr)(charPtr);

	if (theAddr.sin_addr.s_addr == INADDR_NONE)
//#else
//#error unknown platform
#endif
	{
		HOSTENT *hostEnt = WON_SOCKET_CALL(gethostbyname)(charPtr);
#ifdef WIN32
		if (hostEnt)
			theAddr.sin_addr.S_un.S_addr = *((unsigned long*)(hostEnt->h_addr_list[0]));
#else//if defined(_LINUX)
		if (hostEnt)
			theAddr.sin_addr.s_addr = *((unsigned long*)(hostEnt->h_addr_list[0]));
#endif
	}
}


void IPSocket::Address::Set_Port(unsigned short port)
{
	sockaddr_in& theAddr = Get();
	theAddr.sin_family = AF_INET;
	theAddr.sin_port = WON_SOCKET_CALL(htons)(port);
}


void IPSocket::Address::Set(unsigned long ipaddr, unsigned short port)
{
	sockaddr_in& theAddr = Get();
	theAddr.sin_family = AF_INET;
#ifdef WIN32
	theAddr.sin_addr.S_un.S_addr = ipaddr;
#else//if defined (_LINUX)
	theAddr.sin_addr.s_addr = ipaddr;
#endif
	theAddr.sin_port = WON_SOCKET_CALL(htons)(port);
}


void IPSocket::Address::Set(const string& hostAndOptionalPortStr)
{
	int colonPos = hostAndOptionalPortStr.find(':');
	if (colonPos != hostAndOptionalPortStr.npos)
	{
		unsigned short portNum = (short)atol(hostAndOptionalPortStr.substr(colonPos+1).c_str());
		Set(hostAndOptionalPortStr.substr(0, colonPos), portNum);
	}
	else
	{
		sockaddr_in& theAddr = Get();
		theAddr.sin_family = AF_INET;

		const char* charPtr = hostAndOptionalPortStr.c_str();
		bool is_hn = is_hostname(charPtr);

#ifdef WIN32
		theAddr.sin_addr.S_un.S_addr = INADDR_NONE;
		if (!is_hn)
			theAddr.sin_addr.S_un.S_addr = WON_SOCKET_CALL(inet_addr)(charPtr);
		if (theAddr.sin_addr.S_un.S_addr == INADDR_NONE)
		{
			HOSTENT *hostEnt = WON_SOCKET_CALL(gethostbyname)(charPtr);
			if (hostEnt)
				theAddr.sin_addr.S_un.S_addr = *((unsigned long*)(hostEnt->h_addr_list[0]));
		}
#else//if defined(_LINUX)
		theAddr.sin_addr.s_addr = INADDR_NONE;
		if (!is_hn)
			theAddr.sin_addr.s_addr = WON_SOCKET_CALL(inet_addr)(charPtr);

		if (theAddr.sin_addr.s_addr == INADDR_NONE)
		{
			HOSTENT *hostEnt = WON_SOCKET_CALL(gethostbyname)(charPtr);
			if (hostEnt)
				theAddr.sin_addr.s_addr = *((unsigned long*)(hostEnt->h_addr_list[0]));
		}
#endif
	}
}


void IPSocket::Address::Set(const WONMsg::DirEntity& dirEntity)	// extract WON std address from dirEntity
{
	const unsigned char* addrBuf = dirEntity.mNetAddress.data();
#ifdef WIN32
	Set(((in_addr*)(addrBuf + sizeof(short)))->S_un.S_addr, (unsigned short)(addrBuf[1] + (addrBuf[0] << 8)));
#else//if defined(_LINUX)
	Set(((in_addr*)(addrBuf + sizeof(short)))->s_addr, (unsigned short)(addrBuf[1] + (addrBuf[0] << 8)));
//#else
//#error unknown platform
#endif
}


void IPSocket::Address::Set(const WONIPAddress& ipAddr)
{
#ifdef WIN32
	Set(((in_addr*)&(ipAddr.ipAddress))->S_un.S_addr, ipAddr.portNum);
#else//if defined(_LINUX)
	Set(((in_addr*)&(ipAddr.ipAddress))->s_addr, ipAddr.portNum);
//#else
//#error unknown platform
#endif
}


std::string IPSocket::Address::GetAddressString(bool addPort) const
{
	std::string aRet(WON_SOCKET_CALL(inet_ntoa)(GetAddress()));
	if (addPort)
	{
		char aBuf[7];	// an unsigned short can occupy only up to 5 digits, + 1 NULL, + 1 ":"
		aBuf[0] = ':';
		sprintf(aBuf+1, "%hu", GetPort());
		aRet += aBuf;
	}

	return aRet;
}


unsigned short IPSocket::Address::GetPort() const
{
	return WON_SOCKET_CALL(ntohs)(Get().sin_port);
}


string IPSocket::Address::LookupHost(bool addPort) const
{
	HOSTENT *hostEnt = WON_SOCKET_CALL(gethostbyaddr)((char*)&(GetAddress()), sizeof(unsigned long), AF_INET);
	if (!hostEnt)
		return GetAddressString(addPort);
	string aRet = hostEnt->h_name;
	if (addPort)
	{
		char aBuf[7];	// an unsigned short can occupy only up to 5 digits, + 1 NULL, + 1 ":"
		aBuf[0] = ':';
		sprintf(aBuf+1, "%hu", GetPort());
		aRet += aBuf;
	}

	return aRet;
}


void IPSocket::Address::InitFromLocalIP(unsigned short thePort)
{
	char aHostBuf[_MAX_PATH];
	if (WON_SOCKET_CALL(gethostname)(aHostBuf, _MAX_PATH) == 0)
		Set(aHostBuf, thePort);
}


Error IPSocket::Listen(unsigned short port)
{
	Address addr((unsigned long)INADDR_ANY, port);
	return Listen(addr);
}


#include "wondll.h"


static void internalSetWONIPAddr(WONIPAddress* addr, const IPSocket::Address& src)
{
	addr->portNum = src.GetPort();
	in_addr inaddr = src.GetAddress();
	addr->ipAddress = *(long*)(&inaddr);
}


void WONIPAddressSet(WONIPAddress* addr, unsigned long ipaddr, unsigned short port)
{
	if (addr)
		internalSetWONIPAddr(addr, IPSocket::Address(ipaddr, port));
}

void WONIPAddressSetPort(WONIPAddress* addr, unsigned short port)
{
	if (addr)
		addr->portNum = port;
}

void WONIPAddressSetFromString(WONIPAddress* addr, WON_CONST char* hostNameAndPort)
{
	if (addr)
		internalSetWONIPAddr(addr, IPSocket::Address(hostNameAndPort));
}


void WONIPAddressSetFromStringAndPort(WONIPAddress* addr, WON_CONST char* hostName, unsigned short portNum)
{
	if (addr)
		internalSetWONIPAddr(addr, IPSocket::Address(hostName, portNum));
}


void WONIPAddressSetFromSockaddr(WONIPAddress* addr, WON_CONST struct sockaddr_in* sockAddr)
{
	if (addr && sockAddr)
		internalSetWONIPAddr(addr, IPSocket::Address(*sockAddr));
}


void WONIPAddressSetFromInaddr(WONIPAddress* addr, WON_CONST struct in_addr ipaddr, unsigned short port)
{
	if (addr)
		internalSetWONIPAddr(addr, IPSocket::Address(ipaddr, port));
}


void WONIPAddressGetHostName(WON_CONST WONIPAddress* addr, char* strBuf, unsigned short* strBufLength, BOOL includePort)
{
	if (!addr)
	{
		if (strBuf)
			*strBuf = 0;
		if (strBufLength)
			*strBufLength = 0;
		return;
	}
	if (!strBufLength)
	{
		if (strBuf)
			*strBuf = 0;
		return;
	}

	IPSocket::Address tmpAddr(*addr);
	std::string str = tmpAddr.LookupHost(includePort ? true : false);

	unsigned short maxSize = *strBufLength;
	*strBufLength = str.size();

	if (strBuf && maxSize)
	{
		strBuf[maxSize - 1] = 0;
		strncpy(strBuf, str.c_str(), maxSize - 1);
	}
}


void WONIPAddressGetDottedIP(WON_CONST WONIPAddress* addr, char* strBuf, unsigned short* strBufLength, BOOL includePort)
{
	if (!addr)
	{
		if (strBuf)
			*strBuf = 0;
		if (strBufLength)
			*strBufLength = 0;
		return;
	}
	if (!strBufLength)
	{
		if (strBuf)
			*strBuf = 0;
		return;
	}

	IPSocket::Address tmpAddr(*addr);
	std::string str = tmpAddr.GetAddressString(includePort ? true : false);

	unsigned short maxSize = *strBufLength;
	*strBufLength = str.size();

	if (strBuf && maxSize)
	{
		strBuf[maxSize - 1] = 0;
		strncpy(strBuf, str.c_str(), maxSize - 1);
	}
}


void WONIPAddressGetSockaddr(WON_CONST WONIPAddress* addr, struct sockaddr_in* sockAddr)
{
	if (sockAddr)
	{
		IPSocket::Address tmpAddr;
		if (addr)
			tmpAddr = *addr;
		
		*sockAddr = tmpAddr.Get();
	}
}


