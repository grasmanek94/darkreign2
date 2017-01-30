#ifndef PRIZECENTRAL_H
#define PRIZECENTRAL_H

#include "Socket/IPSocket.h"
#include "Errors.h"
#include "SDKCommon/Completion.h"
#include "Authentication/Identity.h"
#include "msg/DB/SMsgDBRegisterUser.h"

namespace WONAPI {

struct PrizecentralDBResult {
	Error error;

	PrizecentralDBResult(Error err) : error(err)
	{}
};

// fail-over address list
Error PrizecentralRegisterUser(Identity* ident, const IPSocket::Address* contestServers, unsigned int numAddrs,
                         const WONMsg::SMsgDBRegisterUser& theRegisterUserMsg,
                         long timeout, bool async, const CompletionContainer<const PrizecentralDBResult&>& completion);

inline Error PrizecentralRegisterUser(Identity* ident, const IPSocket::Address* contestServers, unsigned int numAddrs,
                         const WONMsg::SMsgDBRegisterUser& theRegisterUserMsg, long timeout =-1, bool async =false)
{ return PrizecentralRegisterUser(ident, contestServers, numAddrs, theRegisterUserMsg, timeout, async, DEFAULT_COMPLETION); }

template <class privsType>
inline Error PrizecentralRegisterUserEx(Identity* ident, const IPSocket::Address* contestServers, unsigned int numAddrs,
                                  const WONMsg::SMsgDBRegisterUser& theRegisterUserMsg,
                                  long timeout, bool async, void (*f)(const PrizecentralDBResult&, privsType), privsType privs)
{ return PrizecentralRegisterUser(ident, contestServers, numAddrs, theRegisterUserMsg, timeout, async, new CompletionWithContents<const PrizecentralDBResult&, privsType>(f, privs, true)); }

}; // namespace WONMsg;

#endif // PRIZECENTRAL_H
