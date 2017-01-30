#include "Authentication/AuthSocket.h"
#include "msg/DB/SMsgDBRegisterUserReply.h"
#include "msg/DB/SMsgTypesDB.h"
#include "msg/SServiceTypes.h"
#include "Errors.h"
#include "Prizecentral.h"

using namespace WONAPI;
using namespace WONCommon;
using namespace WONMsg;

class PrizecentralData {
public:
	TCPSocket tcpSocket;
	AuthSocket authSocket;
	IPSocket::Address* serverList;
	unsigned short numServers;
	unsigned short curServer;
	SMsgDBRegisterUser req;
	HWONCOMPLETION hCompletion;
	CompletionContainer<const PrizecentralDBResult&> completion;
	Error err;
	long timeout;
	WONCommon::Event doneEvent;
	bool autoDel;

	PrizecentralData()
		:	authSocket(tcpSocket, 2)
	{ }

	void Done()
	{
		if (serverList)
			delete[] serverList;
		completion.Complete(PrizecentralDBResult(err));
		WONComplete(hCompletion, (void*)err);

		if (autoDel)
			delete this;
		else
			doneEvent.Set();
	}
};


static void DoneReq(const AuthSocket::AuthRecvBaseMsgResult& result, PrizecentralData* PrizecentralData);


static void doRegisterUser(PrizecentralData* PrizecentralData)
{
	PrizecentralData->authSocket.Close(0);
	PrizecentralData->authSocket.ReleaseSession();
	PrizecentralData->tcpSocket.SetRemoteAddress(PrizecentralData->serverList[PrizecentralData->curServer]);

	PrizecentralData->authSocket.SendBaseMsgRequestEx(PrizecentralData->req, 0, true, true, 2000, PrizecentralData->timeout, true, true, DoneReq, PrizecentralData);
}


static void tryNextServer(PrizecentralData* PrizecentralData)
{
	if (++(PrizecentralData->curServer) == PrizecentralData->numServers)
		PrizecentralData->Done();
	else
		doRegisterUser(PrizecentralData);
}


static void DoneReq(const AuthSocket::AuthRecvBaseMsgResult& result, PrizecentralData* PrizecentralData)
{
	if (result.msg)
	{
		auto_ptr<BaseMessage> autoDelReply(result.msg);

		PrizecentralData->err = Error_InvalidMessage;
		if (result.msg->GetServiceType() == SmallCommonService && result.msg->GetMessageType() == DBRegisterUserReply)
		{
			try
			{
				SMsgDBRegisterUserReply msg(*(SmallMessage*)(result.msg));
				
				msg.Unpack();

				PrizecentralData->err = msg.mStatus;
				if (PrizecentralData->err == Error_Success)
				{
					PrizecentralData->Done();
					return;
				}
			}
			catch (...)
			{
				// try next server
			}
		}
	}
	tryNextServer(PrizecentralData);
}

Error WONAPI::PrizecentralRegisterUser(Identity* ident, const IPSocket::Address* contestServers, unsigned int numAddrs,
								 const SMsgDBRegisterUser& theRegisterUserMsg,
                                 long timeout, bool async, const CompletionContainer<const PrizecentralDBResult&>& completion )
{
	Error err = Error_InvalidParams;
	if (numAddrs)
	{
		err = Error_OutOfMemory;
		PrizecentralData* aPrizecentralData = new PrizecentralData;
		if (aPrizecentralData)
		{
			auto_ptr<PrizecentralData> autoDelPrizecentralData(aPrizecentralData);
			aPrizecentralData->serverList = new IPSocket::Address[numAddrs];
			if (aPrizecentralData->serverList)
			{
				for (int i = 0; i < numAddrs; i++)
					aPrizecentralData->serverList[i] = contestServers[i];
				aPrizecentralData->authSocket.SetIdentity(ident);
				aPrizecentralData->autoDel = async;
				aPrizecentralData->hCompletion = 0;
				aPrizecentralData->completion = completion;
				aPrizecentralData->err = Error_Timeout;
				aPrizecentralData->timeout = timeout;
				aPrizecentralData->curServer = 0;
				aPrizecentralData->numServers = numAddrs;
				aPrizecentralData->req = theRegisterUserMsg;

				autoDelPrizecentralData.release();
				doRegisterUser(aPrizecentralData);

				err = Error_Pending;
				if (!async)
				{
					WSSocket::PumpUntil(aPrizecentralData->doneEvent, timeout);
					//aPrizecentralData->doneEvent.WaitFor();
					err = aPrizecentralData->err;
					delete aPrizecentralData;
				}
				return err;
			}
		}
	}
	completion.Complete(PrizecentralDBResult(err));
	return err;
}
