
#include "RawIPSocket.h"


using namespace WONAPI;


Socket::Type RawIPSocket::GetType()
{
	return datagram;
}
