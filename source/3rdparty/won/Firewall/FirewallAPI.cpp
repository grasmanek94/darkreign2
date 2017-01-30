#include "FirewallAPI.h"
#include "Socket/TCPSocket.h"
#include "Socket/TMsgSocket.h"
#include "msg/Firewall/SMsgFirewallDetect.h"
#include "msg/Firewall/SMsgFirewallStatusReply.h"


#include <iostream>

using namespace WONAPI;
using namespace WONMsg;
using namespace WONCommon;


class DetectFirewallData
{
public:
	Error error;
	TCPSocket listenSocket;
	TCPSocket tcpSocket;
	TCPSocket acceptedSocket;
	TMsgSocket tMsgSocket;
	TMsgSocket tMsgAcceptedSocket;
	bool autoDelete;
	Event doneEvent;
	bool tempIsBehindFirewall;
	bool* behindFirewall;
	long timeout;
	CompletionContainer<const DetectFirewallResult&> completion;
	SMsgFirewallDetect msg;
	CriticalSection crit;
	int stageCount;

	DetectFirewallData()
		:	tMsgSocket(&tcpSocket, false, 2),
			tMsgAcceptedSocket(&acceptedSocket, false, 2),
			stageCount(0)
	{ }

	void Done()
	{
		if (error != Error_Success)
			*(behindFirewall) = false;

		completion.Complete(DetectFirewallResult(error, behindFirewall));

		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
	}
};


void DetectFirewallAcceptedDoneRecv(const TMsgSocket::RecvBaseMsgResult& result, DetectFirewallData* detectFirewallData)
{
	AutoCrit crit(detectFirewallData->crit);

	if (!result.msg)
	{
		if (result.closed)
			detectFirewallData->error = Error_ConnectionClosed;
	}
	else
	{
		try
		{
			SMsgFirewallStatusReply reply(*(SmallMessage*)(result.msg));

			reply.Unpack();

			if (reply.GetStatus() == 2100) // StatusFirewallSuccessConnect
				detectFirewallData->error = Error_Success;
			else
				detectFirewallData->error = Error_InvalidMessage;
		}
		catch (...)
		{
			detectFirewallData->error = Error_InvalidMessage;
		}

		delete result.msg;
	}

	// Might have gotten this before recving a message on the accepted socket
	if (detectFirewallData->stageCount == 2)
	{
		crit.Leave();
		detectFirewallData->Done();
	}
	else
	{
		detectFirewallData->stageCount++;
		detectFirewallData->listenSocket.Close(0);
	}
}


void DetectFirewallDoneRecv(const TMsgSocket::RecvBaseMsgResult& result, DetectFirewallData* detectFirewallData)
{
	AutoCrit crit(detectFirewallData->crit);

	if (!result.msg)
	{
		if (!result.closed)
			detectFirewallData->error = Error_ConnectionClosed;
	}
	else
	{
		try
		{
			SMsgFirewallStatusReply reply(*(SmallMessage*)(result.msg));

			reply.Unpack();
			
			if (reply.GetStatus()) // != success
				*(detectFirewallData->behindFirewall) = true;

			detectFirewallData->error = Error_Success;
		}
		catch (...)
		{
			detectFirewallData->error = Error_InvalidMessage;			
		}

		delete result.msg;
	}

	// Might have gotten this before recving a message on the accepted socket
	if (detectFirewallData->stageCount == 2)
	{
		crit.Leave();
		detectFirewallData->Done();
	}
	else
	{
		detectFirewallData->stageCount++;
		detectFirewallData->listenSocket.Close(0);
	}
}


void DetectFirewallDoneAccept(const WSSocket::AcceptResult& result, DetectFirewallData* detectFirewallData)
{
	AutoCrit crit(detectFirewallData->crit);
	
	if (detectFirewallData->acceptedSocket.IsOpen())
	{
		detectFirewallData->stageCount++;
		detectFirewallData->tMsgAcceptedSocket.RecvBaseMsgEx(NULL, detectFirewallData->timeout, true, DetectFirewallAcceptedDoneRecv, detectFirewallData);
	}
	else
	{
		*(detectFirewallData->behindFirewall) = true;

		if (detectFirewallData->stageCount == 1)
		{
			// Must have gotten a fail reply, which caused an Accept to complete when the socket closed
			crit.Leave();
			detectFirewallData->Done();
		}
		else
		{
			detectFirewallData->stageCount += 2;
			detectFirewallData->listenSocket.Close(0);
		}
	}
}


void DetectFirewallDoneOpen(const Socket::OpenResult& result, DetectFirewallData* detectFirewallData)
{
	if (result.error != Error_Success)
	{
		detectFirewallData->error = result.error;
		detectFirewallData->Done();
		return;
	}

	detectFirewallData->listenSocket.AcceptEx(&(detectFirewallData->acceptedSocket), detectFirewallData->timeout, true, DetectFirewallDoneAccept, detectFirewallData);

	unsigned short port = detectFirewallData->listenSocket.GetLocalAddress().GetPort();
	
	detectFirewallData->msg.SetListenPort(port);
	detectFirewallData->msg.SetWaitForConnect(true);
	detectFirewallData->msg.SetMaxConnectTime(0);

	detectFirewallData->tMsgSocket.SendBaseMsg(detectFirewallData->msg, detectFirewallData->timeout, true);
	detectFirewallData->tMsgSocket.RecvBaseMsgEx(NULL, detectFirewallData->timeout, true, DetectFirewallDoneRecv, detectFirewallData);
}


Error WONAPI::DetectFirewall(const IPSocket::Address& firewallDetectionServerAddr, bool* behindFirewall,
							 unsigned short useListenPort, long timeout, bool async,
							 const CompletionContainer<const DetectFirewallResult&>& completion)
{
	bool tmp = false;

	if (behindFirewall)
		*behindFirewall = false;

	DetectFirewallData* detectFirewallData = new DetectFirewallData;
	if (!detectFirewallData)
	{
		completion.Complete(DetectFirewallResult(Error_OutOfMemory, behindFirewall ? behindFirewall : &tmp));
		return Error_OutOfMemory;
	}

	Error listenErr = detectFirewallData->listenSocket.Listen(useListenPort);
	if (listenErr != Error_Success)
	{
		delete detectFirewallData;
		completion.Complete(DetectFirewallResult(listenErr, behindFirewall ? behindFirewall : &tmp));
		return listenErr;
	}

	detectFirewallData->autoDelete = async;
	detectFirewallData->behindFirewall = behindFirewall ? behindFirewall : &(detectFirewallData->tempIsBehindFirewall);
	*(detectFirewallData->behindFirewall) = false;
	detectFirewallData->completion = completion;
	detectFirewallData->timeout = timeout;
	detectFirewallData->error = Error_Timeout;

	detectFirewallData->tcpSocket.OpenEx(&firewallDetectionServerAddr, timeout, true, DetectFirewallDoneOpen, detectFirewallData);

	Error err = Error_Pending;
	
	if (!async)
	{
		WSSocket::PumpUntil(detectFirewallData->doneEvent, timeout);
		//detectFirewallData->doneEvent.WaitFor();
		err = detectFirewallData->error;
		delete detectFirewallData;
	}

	return err;
}


class DetectFirewallFailoverData
{
public:
	IPSocket::Address* addrs;
	unsigned short curAddr;
	unsigned short numAddrs;
	bool* behindFirewall;
	unsigned short useListenPort;
	long timeout;
	Event doneEvent;
	bool autoDel;
	Error err;
	CompletionContainer<const DetectFirewallResult&> completion;

	void Done(const DetectFirewallResult& result)
	{
		err = result.error;

		completion.Complete(result);

		if (autoDel)
			delete this;
		else
			doneEvent.Set();
	}

	~DetectFirewallFailoverData()
	{
		delete[] addrs;
	}
};


static void DoneDetectFirewallFailover(const DetectFirewallResult& result, DetectFirewallFailoverData* failoverData)
{
	if (result.error != Error_Success)
	{
		if (++(failoverData->curAddr) != failoverData->numAddrs)
		{
			DetectFirewallEx(failoverData->addrs[failoverData->curAddr], failoverData->behindFirewall, failoverData->useListenPort, failoverData->timeout, true, DoneDetectFirewallFailover, failoverData);
			return;
		}
	}
	failoverData->Done(result);
}


Error WONAPI::DetectFirewall(const IPSocket::Address* firewallDetectionServerAddrs, unsigned short numAddrs,
					 bool* behindFirewall, unsigned short useListenPort, long timeout,
					 bool async, const CompletionContainer<const DetectFirewallResult&>& completion)
{
	bool tmp = false;

	if (behindFirewall)
		*behindFirewall = false;

	if (!firewallDetectionServerAddrs || !numAddrs)
	{
		completion.Complete(DetectFirewallResult(Error_InvalidParams, behindFirewall ? behindFirewall : &tmp));
		return Error_InvalidParams;
	}

	IPSocket::Address* addrs = new IPSocket::Address[numAddrs];
	if (!addrs)
	{
		completion.Complete(DetectFirewallResult(Error_OutOfMemory, behindFirewall ? behindFirewall : &tmp));
		return Error_OutOfMemory;
	}

	DetectFirewallFailoverData* failoverData = new DetectFirewallFailoverData;
	if (!failoverData)
	{
		delete[] addrs;
		completion.Complete(DetectFirewallResult(Error_OutOfMemory, behindFirewall ? behindFirewall : &tmp));
		return Error_OutOfMemory;
	}

	failoverData->addrs = addrs;
	failoverData->numAddrs = numAddrs;
	failoverData->behindFirewall = behindFirewall;
	failoverData->useListenPort = useListenPort;
	failoverData->timeout = timeout;
	failoverData->autoDel = async;
	failoverData->curAddr = 0;
	failoverData->completion = completion;

	for (unsigned short s = 0; s < numAddrs; s++)
		addrs[s] = firewallDetectionServerAddrs[s];

	DetectFirewallEx(addrs[0], behindFirewall, useListenPort, timeout, true, DoneDetectFirewallFailover, failoverData);

	Error err = Error_Pending;
	if (!async)
	{
		WSSocket::PumpUntil(failoverData->doneEvent, timeout);
//		failoverData->doneEvent.WaitFor();
		err = failoverData->err;
		delete failoverData;
	}
	return err;
}


#include "wondll.h"


WONError WONFirewallDetect(const WONIPAddress* firewallDetectorServers, unsigned int numAddrs,
						   BOOL* behindFirewall, unsigned short listenPort, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && firewallDetectorServers && behindFirewall)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* srvAddrs = new IPSocket::Address[numAddrs];
		if (srvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoSrvAddrs(srvAddrs);

			for (int i = 0; i < numAddrs; i++)
				srvAddrs[i].Set(firewallDetectorServers[i]);

			bool b;
			err = DetectFirewall(srvAddrs, numAddrs, &b, listenPort, timeout);
			*behindFirewall = b ? TRUE : FALSE;
		}
	}
	return err;
}


static void TranslateFirewallDetectCompletion(const WONAPI::DetectFirewallResult& result, std::pair<HWONCOMPLETION, BOOL*> pr)
{
	BOOL* bPtr = pr.second;
	*bPtr = result.behindFirewall ? TRUE : FALSE;

	delete result.behindFirewall;
	WONComplete(pr.first, (void*)result.error);
};


void WONFirewallDetectAsync(WON_CONST WONIPAddress* firewallDetectorServers, unsigned int numAddrs,
							BOOL* behindFirewall, unsigned short listenPort, long timeout,
							HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && firewallDetectorServers && behindFirewall)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* srvAddrs = new IPSocket::Address[numAddrs];
		if (srvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoSrvAddrs(srvAddrs);
			bool* result = new bool;
			if (result)
			{
				for (int i = 0; i < numAddrs; i++)
					srvAddrs[i].Set(firewallDetectorServers[i]);

				err = DetectFirewallEx(srvAddrs, numAddrs, result, listenPort, timeout, true,
					TranslateFirewallDetectCompletion, std::pair<HWONCOMPLETION, BOOL*>(hCompletion, behindFirewall));
				return;
			}
		}
	}
	WONComplete(hCompletion, (void*)err);
}


