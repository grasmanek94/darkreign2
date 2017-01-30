#include "Profile2API.h"

#include <assert.h>
#include "Authentication/AuthSocket.h"
#include "msg/SServiceTypes.h"
#include "msg/Profile/SMsgTypesProfile.h"
#include "Errors.h"

using namespace WONAPI;
using namespace WONCommon;
using namespace WONMsg;

/////////////////////////
struct BaseProfile2Data {

	BaseProfile2Data(const BaseMessage& aReq, int i) : req(aReq), ReplyType(i), authSocket(tcpSocket, 2) {}
	virtual ~BaseProfile2Data() {}

	void Done() {
		if (serverList) delete[] serverList;
		Complete();
		WONComplete(hCompletion, (void*)err);
		if (autoDel) delete this;
		else doneEvent.Set();
	}

	virtual void DoneReq(const AuthSocket::AuthRecvBaseMsgResult&) = 0;
	virtual void Complete() = 0;

	const BaseMessage& req;
	int ReplyType;

	TCPSocket tcpSocket;
	AuthSocket authSocket;
	IPSocket::Address* serverList;
	unsigned short numServers;
	unsigned short curServer;
	HWONCOMPLETION hCompletion;
	Error err;
	long timeout;
	WONCommon::Event doneEvent;
	bool autoDel;
};

////////////////////////////////////////////////////

static void DoneReq(const AuthSocket::AuthRecvBaseMsgResult& r, BaseProfile2Data* p) { assert(p); p->DoneReq(r); }
static void doRegisterUser(BaseProfile2Data* p) {
	p->authSocket.Close(0); p->authSocket.ReleaseSession(); p->tcpSocket.SetRemoteAddress(p->serverList[p->curServer]);
	p->authSocket.SendBaseMsgRequestEx((BaseMessage&)(p->req), 0, true, true, 2000, p->timeout, true, true, DoneReq, p);
}
static void tryNextServer(BaseProfile2Data* p) { if (++(p->curServer) == p->numServers) p->Done(); else doRegisterUser(p); }


////////////////////////////////////////////////////

////////////////////////////
// i = expected message type
// S = request message
// T = expected message
///////////////////////////////////////////////////////////////////////////
template <int i, class S, class T> struct Profile2Data : BaseProfile2Data {
	Profile2Data(const S& aReq, const CompletionContainer<const T&>& c) : 
		BaseProfile2Data(aReq, i), completion(c) {
			t.SetAPIErrorCode(Error_HostUnreachable);
		}

	const CompletionContainer<const T&>& completion;
	T t;

	void DoneReq(const AuthSocket::AuthRecvBaseMsgResult& result) {
		if (result.msg) {
			auto_ptr<BaseMessage> autoDelReply(result.msg);
			err = Error_InvalidMessage;
			if (result.msg->GetServiceType() == SmallProfileServer && 
				result.msg->GetMessageType() == ReplyType) {
				try {
					t = T(*(SmallMessage*)(result.msg)); // may throw
					err = t.GetAPIErrorCode();
					if (err == Error_Success) { Done(); return; }
				} // try
				catch (...) {} // try next server
			} // if
		} // if 
		tryNextServer(this);
	}

	Error Transfer(Identity* ident, const IPSocket::Address* profileServers, 
				   unsigned int numAddrs, long timeout, bool async)
		{
		err = Error_InvalidParams;
		if (numAddrs)
			{
			err = Error_OutOfMemory;
			if (serverList = new IPSocket::Address[numAddrs])
				{
				for (int i = 0; i < numAddrs; i++) serverList[i] = profileServers[i];
				authSocket.SetIdentity(ident);
				autoDel = async;
				hCompletion = 0;
				err = Error_Timeout;
				timeout = timeout;
				curServer = 0;
				numServers = numAddrs;
				doRegisterUser(this);
				err = Error_Pending;
				if (!async) WSSocket::PumpUntil(doneEvent, timeout);
				return err;
				}
			}
		Complete();
		return err;
	}

	void Complete() {
		t.SetErrorCode((WONMsg::ServerStatus)err);
		completion.Complete(t);
	}
};

////////////////////////////////////////////////////

//////////////
// Profile2Get

Error WONAPI::Profile2Get(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
	const SMsgProfile2Get& aMsg, long timeout, bool async, 
	const CompletionContainer<const SMsgProfile2GetReply&>& completion)
{
	return Profile2Data<Profile2GetReply, SMsgProfile2Get, SMsgProfile2GetReply>(aMsg, completion).
		Transfer(ident, profileServers, numAddrs, timeout, async);
}

//////////////
// Profile2Set

Error WONAPI::Profile2Set(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
	const SMsgProfile2Set& aMsg, long timeout, bool async, 
	const CompletionContainer<const SMsgProfile2SetReply&>& completion)
{
	return Profile2Data<Profile2SetReply, SMsgProfile2Set, SMsgProfile2SetReply>(aMsg, completion).
		Transfer(ident, profileServers, numAddrs, timeout, async);
}

/////////////////////////
// Profile2GetCommunities

Error WONAPI::Profile2GetCommunities(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
	const SMsgProfile2GetCommunities& aMsg, long timeout, bool async, 
	const CompletionContainer<const SMsgProfile2GetCommunitiesReply&>& completion)
{
	return Profile2Data<Profile2GetCommunitiesReply, SMsgProfile2GetCommunities, SMsgProfile2GetCommunitiesReply>
		(aMsg, completion).Transfer(ident, profileServers, numAddrs, timeout, async);
}

/////////////////////////
// Profile2GetNewsletters

Error WONAPI::Profile2GetNewsletters(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
	const SMsgProfile2GetNewsletters& aMsg, long timeout, bool async, 
	const CompletionContainer<const SMsgProfile2GetNewslettersReply&>& completion)
{
	return Profile2Data<Profile2GetNewslettersReply, SMsgProfile2GetNewsletters, SMsgProfile2GetNewslettersReply>
		(aMsg, completion).Transfer(ident, profileServers, numAddrs, timeout, async);
}

//////////////////////////////////
// Profile2GetCommunityNewsletters

Error WONAPI::Profile2GetCommunityNewsletters(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
	const SMsgProfile2GetCommunityNewsletters& aMsg, long timeout, bool async, 
	const CompletionContainer<const SMsgProfile2GetCommunityNewslettersReply&>& completion)
{
	return Profile2Data<Profile2GetCommunityNewslettersReply, SMsgProfile2GetCommunityNewsletters, SMsgProfile2GetCommunityNewslettersReply>
		(aMsg, completion).Transfer(ident, profileServers, numAddrs, timeout, async);
}

/////////////////////////
// Profile2SetNewsletters

Error WONAPI::Profile2SetNewsletters(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
	const SMsgProfile2SetNewsletters& aMsg, long timeout, bool async, 
	const CompletionContainer<const SMsgProfile2SetNewslettersReply&>& completion)
{
	return Profile2Data<Profile2SetNewslettersReply, SMsgProfile2SetNewsletters, SMsgProfile2SetNewslettersReply>
		(aMsg, completion).Transfer(ident, profileServers, numAddrs, timeout, async);
}

////////////////////////////////
// Profile2DownloadCommunityTree

Error WONAPI::Profile2DownloadCommunityTree(
	Identity* ident, const IPSocket::Address* profileServers, unsigned int numAddrs,
	const SMsgProfile2DownloadCommunityTree& aMsg, long timeout, bool async, 
	const CompletionContainer<const SMsgProfile2DownloadCommunityTreeReply&>& completion)
{
	return Profile2Data<Profile2DownloadCommunityTreeReply, SMsgProfile2DownloadCommunityTree, SMsgProfile2DownloadCommunityTreeReply>
		(aMsg, completion).Transfer(ident, profileServers, numAddrs, timeout, async);
}
