#ifndef __WON_CONTEST_API_H__
#define __WON_CONTEST_API_H__


#include "Socket/IPSocket.h"
#include "Errors.h"
#include "SDKCommon/Completion.h"
#include "Authentication/Identity.h"


namespace WONAPI {


struct ContestDBResult
{
	Error error;
	void* recvData;
	unsigned short* recvSize;

	ContestDBResult(Error err, void* recvD, unsigned short* recvS)
		:	error(err), recvData(recvD), recvSize(recvS)
	{ }
};


// fail-over address list
Error ContestDBCall(Identity* ident, const IPSocket::Address* contestServers,
					unsigned int numAddrs, short procedureNum, const GUID* GameGUID,
					unsigned short sendSize, void* sendData, unsigned short* recvSize,
					void* recvData, long timeout, bool async,
					const CompletionContainer<const ContestDBResult&>& completion );

inline Error ContestDBCall(Identity* ident, const IPSocket::Address* contestServers,
					unsigned int numAddrs, short procedureNum, const GUID* GameGUID,
					unsigned short sendSize, void* sendData, unsigned short* recvSize,
					void* recvData, long timeout = -1, bool async = false)
{ return ContestDBCall(ident, contestServers, numAddrs, procedureNum, GameGUID, sendSize, sendData, recvSize, recvData, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error ContestDBCallEx(Identity* ident, const IPSocket::Address* contestServers,
							 unsigned int numAddrs, short procedureNum, const GUID* GameGUID,
							 unsigned short sendSize, void* sendData, unsigned short* recvSize,
							 void* recvData, long timeout, bool async,
							 void (*f)(const ContestDBResult&, privsType), privsType privs)
{ return ContestDBCall(ident, contestServers, numAddrs, procedureNum, GameGUID, sendSize,sendData, recvSize, recvData, timeout, async, new CompletionWithContents<const ContestDBResult&, privsType>(f, privs, true));}


// 1 address
inline Error ContestDBCall(Identity* ident, const IPSocket::Address& contestServer,
						   short procedureNum, const GUID* GameGUID, unsigned short sendSize,
						   void* sendData, unsigned short* recvSize, void* recvData,
						   long timeout = -1, bool async = false,
						   const CompletionContainer<const ContestDBResult&>& completion = DEFAULT_COMPLETION )
{ return ContestDBCall(ident, &contestServer, 1, procedureNum, GameGUID, sendSize, sendData, recvSize, recvData, timeout, async, completion); }


template <class privsType>
inline Error ContestDBCallEx(Identity* ident, const IPSocket::Address& contestServer,
							 short procedureNum, const GUID* GameGUID, unsigned short sendSize,
							 void* sendData, unsigned short* recvSize, void* recvData,
							 long timeout, bool async,
							 void (*f)(const ContestDBResult&, privsType), privsType privs)
{ return ContestDBCall(ident, &contestServer, 1, procedureNum, GameGUID, sendSize, sendData, recvSize, recvData, timeout, async, new CompletionWithContents<const ContestDBResult&, privsType>(f, privs, true));}



};



#endif
