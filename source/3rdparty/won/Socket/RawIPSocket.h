#ifndef __WON_RAWIPSOCKET_H__
#define __WON_RAWIPSOCKET_H__


#include "IPSocket.h"


namespace WONAPI {


class RawIPSocket : public IPSocket
{
public:
	RawIPSocket(int protocol, const Address& addr = Address(), int recvBufSize = 8192, int sendBufSize = 8192)
		:	IPSocket(addr, SOCK_RAW, protocol, true, false, recvBufSize, sendBufSize)
	{ }

	~RawIPSocket()
	{
		PrepareForDestruction();
	}

	virtual Type GetType();
};

};

#endif
