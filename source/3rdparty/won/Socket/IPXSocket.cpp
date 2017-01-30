#if !defined(macintosh) || (macintosh != 1)


#include "IPXSocket.h"

using namespace WONAPI;


Socket::Type IPXSocket::GetType()
{
	return datagram;
}

#endif
