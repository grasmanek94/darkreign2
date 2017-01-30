#include "UDPSocket.h"


using namespace WONAPI;


Socket::Type UDPSocket::GetType()
{
	return datagram;
}