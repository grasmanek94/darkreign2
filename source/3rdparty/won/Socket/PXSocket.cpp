#if !defined(macintosh) || (macintosh != 1)

#include "PXSocket.h"


using namespace WONAPI;


const sockaddr PXSocket::Address::blank = { AF_IPX, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };



Error PXSocket::Listen(unsigned short socketNum)
{
	Address addr(socketNum, Address::NodeNum("\0\0\0\0\0\0"));
	return WSSocket::Listen(addr);
}

#endif
