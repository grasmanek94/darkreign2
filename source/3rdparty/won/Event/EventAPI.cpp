#pragma warning(disable : 4786)
#include "EventAPI.h"
#include "msg/Event/SMsgEventRecordEvent.h"
#include "msg/Event/SMsgEventStatusReply.h"
#include "Socket/WSSocket.h"
#include "Authentication/AuthSocket.h"
#include "common/Event.h"
#include "wondll.h"

using namespace WONAPI;
using namespace WONCommon;
using namespace std;


class ReportEventsData
{
public:
	CompletionContainer<Error> completion;
	bool autoDel;
	Error err;
	list<WONEvent> evts;
	unsigned int numEvts;
	unsigned int evtsLeft;
	long timeout;
	long perUDPtimeout;
	Event doneEvent;
	list<IPSocket::Address> addrs;
	list<IPSocket::Address>::iterator addrItor;
	AuthSocket* authSocket;
	bool useUDP;
	bool reliable;
	CriticalSection crit;

	void Done()
	{
		completion.Complete(err);
		if (autoDel)
			delete this;
		else
			doneEvent.Set();
	}

	void TryNextEventServer();

	~ReportEventsData()
	{
		delete authSocket;
	}
};


static void DoneSendEvent(const AuthSocket::AuthRecvBaseMsgResult& result, ReportEventsData* reportEventsData)
{
	AutoCrit autoCrit(reportEventsData->crit);
	if (!result.msg)
	{
		reportEventsData->err = result.closed ? Error_ConnectionClosed : Error_Timeout;
		if (reportEventsData->evtsLeft == reportEventsData->numEvts)
		{
			// Only bother trying the next server if none of the events were accepted
			// This could be improved to work on a per-event granularity.  But, for now,
			// this is easier, because we're not matching out of order UDP replies
			reportEventsData->TryNextEventServer();
		}
		else
			reportEventsData->Done();
		return;
	}
	delete result.msg;
	if (!--reportEventsData->evtsLeft)
	{
		reportEventsData->err = Error_Success;
		reportEventsData->Done();
	}
}


static void ReportEventOpenDone(const AuthSocket::OpenResult& result, ReportEventsData* reportEventsData)
{
	if (result.error != Error_Success)
	{
		reportEventsData->err = result.error;
		reportEventsData->TryNextEventServer();
		return;
	}

	if (reportEventsData->useUDP)
	{
		if (reportEventsData->reliable)
		{	// use reliable UDP
			list<WONEvent>::iterator itor = reportEventsData->evts.begin();
			for (; itor != reportEventsData->evts.end(); ++itor)
			{
				void (*DoneSendEventFunc)(const AuthSocket::AuthRecvBaseMsgResult&, ReportEventsData*) = DoneSendEvent;
				reportEventsData->authSocket->SendBaseMsgRequestEx((*itor).msg, 0, true, false, reportEventsData->perUDPtimeout, reportEventsData->timeout, true, true, DoneSendEventFunc, reportEventsData);
			}
		}
		else
		{	// Just send 'em, and forget 'em
			list<WONEvent>::iterator itor = reportEventsData->evts.begin();
			for (; itor != reportEventsData->evts.end(); ++itor)
				reportEventsData->authSocket->SendBaseMsg((*itor).msg, true, reportEventsData->timeout, true, true);
			reportEventsData->err = Error_Success;
			reportEventsData->Done();
		}
	}
	else
	{	// Send them all, wait for all replies
		list<WONEvent>::iterator itor = reportEventsData->evts.begin();
		for (; itor != reportEventsData->evts.end(); ++itor)
		{
			void (*DoneSendEventFunc)(const AuthSocket::AuthRecvBaseMsgResult&, ReportEventsData*) = DoneSendEvent;
			reportEventsData->authSocket->SendBaseMsg((*itor).msg, true, -1, true, true);
			reportEventsData->authSocket->RecvBaseMsgEx(0, false, reportEventsData->timeout, true, DoneSendEventFunc, reportEventsData);		
		}
	}
}


void ReportEventsData::TryNextEventServer()
{
	if (++addrItor == addrs.end())
		Done();
	else
	{
		authSocket->Close(0);
		authSocket->ReleaseSession();
		authSocket->GetSocket()->SetRemoteAddress(*addrItor);
		authSocket->SetRefreshAddress(*addrItor);
		authSocket->OpenEx(true, timeout, true, ReportEventOpenDone, this);
	}
}


Error WONAPI::ReportEvents(Identity* ident, const IPSocket::Address* eventSrvrs, unsigned int numSrvrs,
						   const WONEvent* evts, unsigned int numEvts, bool reliable, bool useUDP,
						   long timeout, long perUDPtimeout, bool async,
						   const CompletionContainer<Error>& completion)
{
	if (!eventSrvrs || !numSrvrs || !evts || !numEvts)
	{
		completion.Complete(Error_InvalidParams);
		return Error_InvalidParams;
	}
	ReportEventsData* reportEventsData = new ReportEventsData;
	if (!reportEventsData)
	{
		completion.Complete(Error_OutOfMemory);
		return Error_OutOfMemory;
	}

	reportEventsData->authSocket = 0;
	reportEventsData->completion = completion;
	reportEventsData->autoDel = async;
	reportEventsData->err = Error_Success;
	reportEventsData->useUDP = useUDP;
	reportEventsData->reliable = reliable;
	reportEventsData->numEvts = numEvts;
	reportEventsData->evtsLeft = numEvts;
	reportEventsData->timeout = timeout;
	reportEventsData->perUDPtimeout = perUDPtimeout;

	for (int curEvt = 0; curEvt < numEvts; curEvt++)
		reportEventsData->evts.push_back(evts[curEvt]);

	for (int i = 0; i < numSrvrs; i++)
		reportEventsData->addrs.push_back(eventSrvrs[i]);
	reportEventsData->addrItor = reportEventsData->addrs.begin();
	
	AuthSocket* authSocket;
	if (useUDP)
	{
		UDPSocket* udpSocket = new UDPSocket(*eventSrvrs);
		if (!udpSocket)
		{
			delete reportEventsData;
			completion.Complete(Error_OutOfMemory);
			return Error_OutOfMemory;
		}
		authSocket = new AuthSocket(ident, udpSocket, true, *eventSrvrs, 4, true, false, false);
		if (!authSocket)
		{
			delete udpSocket;
			delete reportEventsData;
			completion.Complete(Error_OutOfMemory);
			return Error_OutOfMemory;
		}
		reportEventsData->authSocket = authSocket;
	}
	else
	{
		TCPSocket* tcpSocket = new TCPSocket(*eventSrvrs);
		if (!tcpSocket)
		{
			delete reportEventsData;
			completion.Complete(Error_OutOfMemory);
			return Error_OutOfMemory;
		}
		authSocket = new AuthSocket(ident, tcpSocket, true, 4, true, false, false);
		if (!authSocket)
		{
			delete tcpSocket;
			delete reportEventsData;
			completion.Complete(Error_OutOfMemory);
			return Error_OutOfMemory;
		}
		reportEventsData->authSocket = authSocket;
	}
	authSocket->OpenEx(true, timeout, true, ReportEventOpenDone, reportEventsData);

	if (async)
		return Error_Pending;

	WSSocket::PumpUntil(reportEventsData->doneEvent, timeout);
	Error err = reportEventsData->err;
	delete reportEventsData;
	return err;
}


HWONEVENT WONEventCreate(unsigned short activityType)
{
	return (HWONEVENT)new WONEvent;
}


void WONEventSetActivityType(HWONEVENT hWONEvent, unsigned short activityType)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->SetActivityType(activityType);
}


void WONEventSetActivityDataTime(HWONEVENT hWONEvent, time_t activityDateTime)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->SetActivityDateTime(activityDateTime);
}


void WONEventSetUserA(HWONEVENT hWONEvent, unsigned short authMethod, unsigned long userId, const char* userName)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->SetUser(authMethod, userId, userName);
}


void WONEventSetUserW(HWONEVENT hWONEvent, unsigned short authMethod, unsigned long userId, const wchar_t* userName)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->SetUser(authMethod, userId, userName);
}


void WONEventSetClientA(HWONEVENT hWONEvent, const char* clientName, const char* clientAddr)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->SetClient(clientName, clientAddr);
}


void WONEventSetClientW(HWONEVENT hWONEvent, const wchar_t* clientName, const char* clientAddr)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->SetClient(clientName, clientAddr);
}


void WONEventSetServerA(HWONEVENT hWONEvent, unsigned short serverType, const char* logicalName, const char* serverAddr)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->SetServer(serverType, logicalName, serverAddr);
}


void WONEventSetServerW(HWONEVENT hWONEvent, unsigned short serverType, const wchar_t* logicalName, const char* serverAddr)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->SetServer(serverType, logicalName, serverAddr);
}


void WONEventAddDetailStringA(HWONEVENT hWONEvent, unsigned short detailType, const char* str)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->AddDetail(detailType, str);
}


void WONEventAddDetailStringW(HWONEVENT hWONEvent, unsigned short detailType, const wchar_t* str)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->AddDetail(detailType, str);
}


void WONEventAddDetailNumber(HWONEVENT hWONEvent, unsigned short detailType, double d)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->AddDetail(detailType, d);
}


void WONEventAddAttachmentA(HWONEVENT hWONEvent, const char* desc, BYTE contentType, void* body, unsigned short size)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->AddAttachment(desc, contentType, body, size);
}


void WONEventAddAttachmentW(HWONEVENT hWONEvent, const wchar_t* desc, BYTE contentType, void* body, unsigned short size)
{
	WONEvent* evt = (WONEvent*)hWONEvent;
	evt->AddAttachment(desc, contentType, body, size);
}


void WONEventCloseHandle(HWONEVENT hWONEvent)
{
	delete (WONEvent*)hWONEvent;
}


WONError WONEventReport(HWONAUTH hWONAuth, WON_CONST WONIPAddress* evtServers, unsigned int numAddrs,
						HWONEVENT* events, unsigned int numEvts, BOOL reliable, BOOL useUDP,
						long timeout, long perUDPtimeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && evtServers && events && numEvts)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* evtSrvAddrs = new IPSocket::Address[numAddrs];
		if (evtSrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoEvtSrvAddrs(evtSrvAddrs);

			WONEvent* eventArray = new WONEvent[numEvts];
			if (eventArray)
			{
				array_auto_ptr<WONEvent> autoEventArray(eventArray);

				for (int i = 0; i < numAddrs; i++)
					evtSrvAddrs[i].Set(evtServers[i].ipAddress, evtServers[i].portNum);

				err = ReportEvents((Identity*)hWONAuth, evtSrvAddrs, numAddrs, eventArray,
					numEvts, reliable ? true : false, useUDP ? true : false, timeout, perUDPtimeout);
			}
		}
	}
	return err;
}


static void TranslateWONCompletionWithError(Error err, HWONCOMPLETION hCompletion)
{
	WONComplete(hCompletion, (void*)err);
}


void WONEventReportAsync(HWONAUTH hWONAuth, WON_CONST WONIPAddress* evtServers, unsigned int numAddrs,
						 HWONEVENT* events, unsigned int numEvts, BOOL reliable, BOOL useUDP,
						 long timeout, long perUDPtimeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && evtServers && events && numEvts)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* evtSrvAddrs = new IPSocket::Address[numAddrs];
		if (evtSrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoEvtSrvAddrs(evtSrvAddrs);

			WONEvent* eventArray = new WONEvent[numEvts];
			if (eventArray)
			{
				array_auto_ptr<WONEvent> autoEventArray(eventArray);

				for (int i = 0; i < numAddrs; i++)
					evtSrvAddrs[i].Set(evtServers[i].ipAddress, evtServers[i].portNum);

				ReportEventsEx((Identity*)hWONAuth, evtSrvAddrs, numAddrs, eventArray,
					numEvts, reliable ? true : false, useUDP ? true : false, timeout, perUDPtimeout,
					true, TranslateWONCompletionWithError, hCompletion);
				return;
			}
		}
	}
	WONComplete(hCompletion, (void*)err);
}




