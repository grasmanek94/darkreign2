#ifndef PRIZECENTRAL_H
#define PRIZECENTRAL_H

#include "Socket/IPSocket.h"
#include "Errors.h"
#include "SDKCommon/Completion.h"
#include "Authentication/Identity.h"
#include "msg/Profile/SMsgProfileMsgs.h"

//////////////////
namespace WONAPI {

//////////////
// Profile2Get

Error Profile2Get(Identity*, const IPSocket::Address*, unsigned int, const WONMsg::SMsgProfile2Get&,
                  long, bool, const CompletionContainer<const WONMsg::SMsgProfile2GetReply&>&);

inline Error Profile2Get(Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs, 
						 const WONMsg::SMsgProfile2Get& aMsg, long timeout =-1, bool async =false)
{ 
	return Profile2Get(ident, profileServers, numAddrs, aMsg, timeout, async, DEFAULT_COMPLETION); 
}

template <class privsType>
inline Error Profile2Get(Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
                                  const WONMsg::SMsgProfile2Get& aMsg,
                                  long timeout, bool async, 
								  void (*f)(const WONMsg::SMsgProfile2GetReply&, privsType), privsType privs)
{ 
	return Profile2Get(ident, profileServers, numAddrs, aMsg, timeout, async, 
		new CompletionWithContents<const WONMsg::SMsgProfile2GetReply&, privsType>(f, privs, true)); 
}

//////////////
// Profile2Set

Error Profile2Set(Identity*, const IPSocket::Address*, unsigned int, const WONMsg::SMsgProfile2Set&,
                  long, bool, const CompletionContainer<const WONMsg::SMsgProfile2SetReply&>&);

inline Error Profile2Set(Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs, 
						 const WONMsg::SMsgProfile2Set& aMsg, long timeout =-1, bool async =false)
{ 
	return Profile2Set(ident, profileServers, numAddrs, aMsg, timeout, async, DEFAULT_COMPLETION); 
}

template <class privsType>
inline Error Profile2Set(Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
                                  const WONMsg::SMsgProfile2Set& aMsg,
                                  long timeout, bool async, 
								  void (*f)(const WONMsg::SMsgProfile2SetReply&, privsType), privsType privs)
{ 
	return Profile2Set(ident, profileServers, numAddrs, aMsg, timeout, async, 
		new CompletionWithContents<const WONMsg::SMsgProfile2SetReply&, privsType>(f, privs, true)); 
}

/////////////////////////
// Profile2GetCommunities

Error Profile2GetCommunities(
	Identity*, const IPSocket::Address*, unsigned int, const WONMsg::SMsgProfile2GetCommunities&,
	long, bool, const CompletionContainer<const WONMsg::SMsgProfile2GetCommunitiesReply&>&);

inline Error Profile2GetCommunities(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs, 
	const WONMsg::SMsgProfile2GetCommunities& aMsg, long timeout =-1, bool async =false)
{ 
	return Profile2GetCommunities(ident, profileServers, numAddrs, aMsg, timeout, async, DEFAULT_COMPLETION); 
}

template <class privsType>
inline Error Profile2GetCommunities(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
    const WONMsg::SMsgProfile2GetCommunities& aMsg, long timeout, bool async, 
	void (*f)(const WONMsg::SMsgProfile2GetCommunitiesReply&, privsType), privsType privs)
{ 
	return Profile2GetCommunities(ident, profileServers, numAddrs, aMsg, timeout, async, 
		new CompletionWithContents<const WONMsg::SMsgProfile2GetCommunitiesReply&, privsType>(f, privs, true)); 
}

/////////////////////////
// Profile2GetNewsletters

Error Profile2GetNewsletters(
	Identity*, const IPSocket::Address*, unsigned int, const WONMsg::SMsgProfile2GetNewsletters&,
	long, bool, const CompletionContainer<const WONMsg::SMsgProfile2GetNewslettersReply&>&);

inline Error Profile2GetNewsletters(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs, 
	const WONMsg::SMsgProfile2GetNewsletters& aMsg, long timeout =-1, bool async =false)
{ 
	return Profile2GetNewsletters(ident, profileServers, numAddrs, aMsg, timeout, async, DEFAULT_COMPLETION); 
}

template <class privsType>
inline Error Profile2GetNewsletters(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
    const WONMsg::SMsgProfile2GetNewsletters& aMsg, long timeout, bool async, 
	void (*f)(const WONMsg::SMsgProfile2GetNewslettersReply&, privsType), privsType privs)
{ 
	return Profile2GetNewsletters(ident, profileServers, numAddrs, aMsg, timeout, async, 
		new CompletionWithContents<const WONMsg::SMsgProfile2GetNewslettersReply&, privsType>(f, privs, true)); 
}

//////////////////////////////////
// Profile2GetCommunityNewsletters

Error Profile2GetCommunityNewsletters(
	Identity*, const IPSocket::Address*, unsigned int, const WONMsg::SMsgProfile2GetCommunityNewsletters&,
	long, bool, const CompletionContainer<const WONMsg::SMsgProfile2GetCommunityNewslettersReply&>&);

inline Error Profile2GetCommunityNewsletters(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs, 
	const WONMsg::SMsgProfile2GetCommunityNewsletters& aMsg, long timeout =-1, bool async =false)
{ 
	return Profile2GetCommunityNewsletters(ident, profileServers, numAddrs, aMsg, timeout, async, DEFAULT_COMPLETION); 
}

template <class privsType>
inline Error Profile2GetCommunityNewsletters(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
    const WONMsg::SMsgProfile2GetCommunityNewsletters& aMsg, long timeout, bool async, 
	void (*f)(const WONMsg::SMsgProfile2GetCommunityNewslettersReply&, privsType), privsType privs)
{ 
	return Profile2GetCommunityNewsletters(ident, profileServers, numAddrs, aMsg, timeout, async, 
		new CompletionWithContents<const WONMsg::SMsgProfile2GetCommunityNewslettersReply&, privsType>(f, privs, true)); 
}

/////////////////////////
// Profile2SetNewsletters

Error Profile2SetNewsletters(
	Identity*, const IPSocket::Address*, unsigned int, const WONMsg::SMsgProfile2SetNewsletters&,
	long, bool, const CompletionContainer<const WONMsg::SMsgProfile2SetNewslettersReply&>&);

inline Error Profile2SetNewsletters(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs, 
	const WONMsg::SMsgProfile2SetNewsletters& aMsg, long timeout =-1, bool async =false)
{ 
	return Profile2SetNewsletters(ident, profileServers, numAddrs, aMsg, timeout, async, DEFAULT_COMPLETION); 
}

template <class privsType>
inline Error Profile2SetNewsletters(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
    const WONMsg::SMsgProfile2SetNewsletters& aMsg, long timeout, bool async, 
	void (*f)(const WONMsg::SMsgProfile2SetNewslettersReply&, privsType), privsType privs)
{ 
	return Profile2SetNewsletters(ident, profileServers, numAddrs, aMsg, timeout, async, 
		new CompletionWithContents<const WONMsg::SMsgProfile2SetNewslettersReply&, privsType>(f, privs, true)); 
}

////////////////////////////////
// Profile2DownloadCommunityTree

Error Profile2DownloadCommunityTree(
	Identity*, const IPSocket::Address*, unsigned int, const WONMsg::SMsgProfile2DownloadCommunityTree&,
	long, bool, const CompletionContainer<const WONMsg::SMsgProfile2DownloadCommunityTreeReply&>&);

inline Error Profile2DownloadCommunityTree(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs, 
	const WONMsg::SMsgProfile2DownloadCommunityTree& aMsg, long timeout =-1, bool async =false)
{ 
	return Profile2DownloadCommunityTree(ident, profileServers, numAddrs, aMsg, timeout, async, DEFAULT_COMPLETION); 
}

template <class privsType>
inline Error Profile2DownloadCommunityTree(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
    const WONMsg::SMsgProfile2DownloadCommunityTree& aMsg, long timeout, bool async, 
	void (*f)(const WONMsg::SMsgProfile2DownloadCommunityTreeReply&, privsType), privsType privs)
{ 
	return Profile2DownloadCommunityTree(ident, profileServers, numAddrs, aMsg, timeout, async, 
		new CompletionWithContents<const WONMsg::SMsgProfile2DownloadCommunityTreeReply&, privsType>(f, privs, true)); 
}

///////////////////////
}; // namespace WONMsg;

#endif // PRIZECENTRAL_H
