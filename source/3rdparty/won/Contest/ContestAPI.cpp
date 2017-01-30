#include "Authentication/AuthSocket.h"
#include "wondll.h"
#include "msg/Contest/MMsgContestDBCall.h"
#include "msg/Contest/MMsgTypesContest.h"
#include "msg/MServiceTypes.h"
#include "Errors.h"
#include "ContestAPI.h"

using namespace WONAPI;
using namespace WONCommon;
using namespace WONMsg;


static Error ConvertDLLAddrList(const WONIPAddress* serverAddr, unsigned short numServers,
								IPSocket::Address*& addrs)
{
	addrs = 0;
	if (numServers)
	{
		// build server list
		addrs = new IPSocket::Address[numServers];
		if (addrs)
		{
			array_auto_ptr<IPSocket::Address> autoDelAddrs(addrs);
			unsigned short curServer = 0;
			do {
				addrs[curServer].Set(serverAddr[curServer]);
			} while (++curServer < numServers);
			autoDelAddrs.release();
			return Error_Success;
		}
		return Error_OutOfMemory;
	}
	return Error_Success;
}



WONError WONContestDBCall(const WONIPAddress* contestServerAddr, unsigned short numServers,
						  HWONAUTH hWONAuth, long timeout, short procedureNum,
						  const GUID* GameGUID, unsigned short sendSize, void* sendData,
						  unsigned short* recvSize, void* recvData)
{
	unsigned short maxRecvSize = 0;
	if (recvSize)
	{
		maxRecvSize = *recvSize;
		*recvSize = 0;
	}
	Error err = Error_InvalidParams;
	if (numServers && hWONAuth != NULL)
	{
		IPSocket::Address* addrs;
		err = ConvertDLLAddrList(contestServerAddr, numServers, addrs);
		if (err == Error_Success)
		{
			for (int curAddr = 0; curAddr < numServers; curAddr++)
			{
				TCPSocket tcpSocket(addrs[curAddr]);
				AuthSocket authSocket((Identity*)hWONAuth, tcpSocket, 2);

				MMsgContestDBCall req;

				req.SetUserId(((Identity*)hWONAuth)->GetUserId());

				req.SetProcedureNum(procedureNum);
				req.SetGameGUID(*GameGUID);
				req.SetData(sendData, sendSize);

				BaseMessage* reply = authSocket.SendBaseMsgRequest(req, 0, true, true, 2000, timeout);
				if (!reply)
					err = Error_Timeout;
				else
				{
					auto_ptr<BaseMessage> autoDelReply(reply);

					err = Error_InvalidMessage;
					if (reply->GetServiceType() == MiniContestServer && reply->GetMessageType() == ContestDBCallReply)
					{
						try
						{
							MMsgContestDBCallReply msg(*(MiniMessage*)reply);
							
							msg.Unpack();

							err = msg.GetStatus();
							if (err == Error_Success)
							{
								if (recvData && maxRecvSize)
								{
									unsigned short copySize = msg.GetDataLen();
									*recvSize = copySize;
									if (copySize > maxRecvSize)
										copySize = maxRecvSize;
									
									memcpy(recvData, msg.GetData(), copySize);
								}
								break;
							}
						}
						catch (...)
						{
							continue;	// try next server
						}
					}
				}
			}
			delete[] addrs;
		}
	}
	return err;
}


class ContestData
{
public:
	TCPSocket tcpSocket;
	AuthSocket authSocket;
	IPSocket::Address* serverList;
	unsigned short numServers;
	unsigned short curServer;
	MMsgContestDBCall req;
	HWONCOMPLETION hCompletion;
	CompletionContainer<const ContestDBResult&> completion;
	Error err;
	long timeout;
	unsigned short* recvSize;
	void* recvData;
	unsigned short maxRecvSize;
	WONCommon::Event doneEvent;
	bool autoDel;

	ContestData()
		:	authSocket(tcpSocket, 2)
	{ }

	void Done()
	{
		if (serverList)
			delete[] serverList;
		completion.Complete(ContestDBResult(err, recvData, recvSize));
		WONComplete(hCompletion, (void*)err);

		if (autoDel)
			delete this;
		else
			doneEvent.Set();
	}
};


static void DoneReq(const AuthSocket::AuthRecvBaseMsgResult& result, ContestData* contestData);


static void doDBCall(ContestData* contestData)
{
	contestData->authSocket.Close(0);
	contestData->authSocket.ReleaseSession();
	contestData->tcpSocket.SetRemoteAddress(contestData->serverList[contestData->curServer]);

	contestData->authSocket.SendBaseMsgRequestEx(contestData->req, 0, true, true, 2000, contestData->timeout, true, true, DoneReq, contestData);
}


static void tryNextServer(ContestData* contestData)
{
	if (++(contestData->curServer) == contestData->numServers)
		contestData->Done();
	else
		doDBCall(contestData);
}


static void DoneReq(const AuthSocket::AuthRecvBaseMsgResult& result, ContestData* contestData)
{
	if (result.msg)
	{
		auto_ptr<BaseMessage> autoDelReply(result.msg);

		contestData->err = Error_InvalidMessage;
		if (result.msg->GetServiceType() == MiniContestServer && result.msg->GetMessageType() == ContestDBCallReply)
		{
			try
			{
				MMsgContestDBCallReply msg(*(MiniMessage*)(result.msg));
				
				msg.Unpack();

				contestData->err = msg.GetStatus();
				if (contestData->err == Error_Success)
				{
					if (contestData->recvData && contestData->maxRecvSize)
					{
						unsigned short copySize = msg.GetDataLen();
						*(contestData->recvSize) = copySize;
						if (copySize > contestData->maxRecvSize)
							copySize = contestData->maxRecvSize;
						
						memcpy(contestData->recvData, msg.GetData(), copySize);
					}
					contestData->Done();
					return;
				}
			}
			catch (...)
			{
				// try next server
			}
		}
	}
	tryNextServer(contestData);
}


void WONContestDBCallAsync(WON_CONST WONIPAddress* contestServerAddr, unsigned short numServers,
						   HWONAUTH hWONAuth, long timeout, short procedureNum,
						   WON_CONST GUID* GameGUID, unsigned short sendSize, void* sendData,
						   unsigned short* recvSize, void* recvData, HWONCOMPLETION hCompletion)
{
	unsigned short maxRecvSize = 0;
	if (recvSize)
	{
		maxRecvSize = *recvSize;
		*recvSize = 0;
	}
	Error err = Error_InvalidParams;
	if (numServers)
	{
		err = Error_OutOfMemory;
		ContestData* contestData = new ContestData;
		if (contestData)
		{
			auto_ptr<ContestData> autoDelContestData(contestData);
			contestData->serverList = 0;
			err = ConvertDLLAddrList(contestServerAddr, numServers, contestData->serverList);
			if (err == Error_Success)
			{
				contestData->autoDel = true;
				contestData->authSocket.SetIdentity((Identity*)hWONAuth);
				contestData->hCompletion = hCompletion;
				contestData->err = Error_Timeout;
				contestData->timeout = timeout;
				contestData->curServer = 0;
				contestData->numServers = numServers;
				contestData->req.SetProcedureNum(procedureNum);
				contestData->req.SetGameGUID(*GameGUID);
				contestData->req.SetData(sendData, sendSize);
				contestData->recvSize = recvSize;
				contestData->recvData = recvData;
				contestData->maxRecvSize = maxRecvSize;
				autoDelContestData.release();

				doDBCall(contestData);
				return;
			}
		}
	}
	WONComplete(hCompletion, (void*)err);
}


Error WONAPI::ContestDBCall(Identity* ident, const IPSocket::Address* contestServers,
							unsigned int numAddrs, short procedureNum, const GUID* GameGUID,
							unsigned short sendSize, void* sendData, unsigned short* recvSize,
							void* recvData, long timeout, bool async,
							const CompletionContainer<const ContestDBResult&>& completion )
{
	unsigned short maxRecvSize = 0;
	if (recvSize)
	{
		maxRecvSize = *recvSize;
		*recvSize = 0;
	}
	Error err = Error_InvalidParams;
	if (numAddrs)
	{
		err = Error_OutOfMemory;
		ContestData* contestData = new ContestData;
		if (contestData)
		{
			auto_ptr<ContestData> autoDelContestData(contestData);
			contestData->serverList = new IPSocket::Address[numAddrs];
			if (contestData->serverList)
			{
				for (int i = 0; i < numAddrs; i++)
					contestData->serverList[i] = contestServers[i];
				contestData->authSocket.SetIdentity(ident);
				contestData->autoDel = async;
				contestData->hCompletion = 0;
				contestData->completion = completion;
				contestData->err = Error_Timeout;
				contestData->timeout = timeout;
				contestData->curServer = 0;
				contestData->numServers = numAddrs;
				// NOTE: To match the other DBCall calls in here, we should set the request's
				//       UserId field.  However, I have modified the Contest Server so it no
				//       longer uses the UserId in the request.  It always uses the id in the
				//       certificate...
				//       Actually, without that server change, this code would fail even if we did
				//       set the userid field here.  If the user hasn't authenticated their
				//       Identity before this call, ident->GetUserId() will return 0.
				contestData->req.SetProcedureNum(procedureNum);
				contestData->req.SetGameGUID(*GameGUID);
				contestData->req.SetData(sendData, sendSize);
				contestData->recvSize = recvSize;
				contestData->recvData = recvData;
				contestData->maxRecvSize = maxRecvSize;

				autoDelContestData.release();
				doDBCall(contestData);

				err = Error_Pending;
				if (!async)
				{
					WSSocket::PumpUntil(contestData->doneEvent, timeout);
					//contestData->doneEvent.WaitFor();
					err = contestData->err;
					delete contestData;
				}
				return err;
			}
		}
	}
	completion.Complete(ContestDBResult(err, recvData, recvSize));
	return err;
}
