// Disable debug STL warnings
#pragma warning(disable : 4786)
#include <set>
#include <assert.h>
#include "Ping.h"
#include "common/CriticalSection.h"
#include "common/Event.h"
#include "Socket/RawIPSocket.h"
#include "Socket/TCPSocket.h"
#include "Socket/UDPSocket.h"
#include "Socket/TMsgSocket.h"
#include "msg/Comm/MMsgCommPing.h"
#include "msg/TServiceTypes.h"
#include "msg/Comm/MMsgTypesComm.h"
#include "common/WONEndian.h"

#ifdef _LINUX
#include "common/linuxGlue.h"
#elif defined(WIN32)
#include <process.h>
#else
//#error unknown platform
#endif

using namespace WONAPI;
using namespace WONMsg;
using namespace WONCommon;


// The RawSocket ping implementation is derived from VC sample code

#define ICMP_ECHO		8
#define ICMP_ECHOREPLY	0

// The IP header
struct IpHeader {
	unsigned int h_len:4;          // length of the header
	unsigned int version:4;        // Version of IP
	unsigned char tos;             // Type of service
	unsigned short total_len;      // total length of the packet
	unsigned short ident;          // unique identifier
	unsigned short frag_and_flags; // flags
	unsigned char  ttl; 
	unsigned char proto;           // protocol (TCP, UDP etc)
	unsigned short checksum;       // IP checksum

	unsigned int sourceIP;
	unsigned int destIP;
};


// ICMP header
struct IcmpHeader {
    unsigned char  type;
    unsigned char  code;
    unsigned short checksum;
    unsigned long  id; // combined id and seq
};


struct IcmpData
{
	unsigned long time;
};

struct IcmpPacket
{
	IcmpHeader header;
	IcmpData data;
};


static unsigned short checksum(unsigned short* buffer, int size)
{
	register unsigned long sum = 0;

	assert(size % sizeof(unsigned short) == 0);	// ICMP header much be an even number of bytes

	size /= sizeof(unsigned short);
	while (size--)
		sum += *buffer++;

	sum += (sum >> 16);	// why?  Didn't see this documented anywhere

	return ~(sum);
}


class IcmpPingData
{
public:
	IPSocket::Address addr;
	RawIPSocket theSocket;
	IcmpPacket packet;
	Error error;
	unsigned int* pingTimeArg;
	Event doneEvent;
	bool autoDelete;
	CompletionContainer<const PingOnceResult&> completion;
	unsigned long timeout;

	IcmpPingData(IPSocket::Address address, unsigned int* timeArg, bool autoDel)
		:	addr(address),
			pingTimeArg(timeArg),
			autoDelete(autoDel),
			theSocket(IPPROTO_ICMP)
	{
		timeBeginPeriod(1);
	}

	void Done(Error err, unsigned int pingTime)
	{
		error = err;
		theSocket.Close();
		if (pingTimeArg)
			*pingTimeArg = pingTime;
		completion.Complete(PingOnceResult(addr, err, pingTime, pingTimeArg));
		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
		timeEndPeriod(1);
	}
};


static void RecvPingComplete(const RawIPSocket::TransmitResult& result, IcmpPingData* icmpPingData)
{
	unsigned long recvTime = timeGetTime();

	if (!result.bytesUsed)
	{
		icmpPingData->Done(Error_Timeout, 0);
		return;
	}
	
	// make sure it's our packet

	IpHeader* iphdr = (IpHeader*)(result.buffer);
	unsigned short iphdrlen = iphdr->h_len * 4;  // number of 32-bit words *4 = bytes

	Error err = Error_InvalidMessage;

	if (result.bytesUsed >= iphdrlen + sizeof(IcmpPacket))
	{
		IcmpHeader* icmphdr = (IcmpHeader*)((unsigned char*)iphdr + iphdrlen);

		if (icmphdr->type == ICMP_ECHOREPLY && icmphdr->id == (unsigned long)icmpPingData)
			err = Error_Success;
	}

	icmpPingData->Done(err, recvTime - icmpPingData->packet.data.time);
}


static void SendPingComplete(const RawIPSocket::TransmitResult& result, IcmpPingData* icmpPingData)
{
	if (result.bytesRequested != result.bytesUsed)
	{
		icmpPingData->Done(Error_HostUnreachable, 0);
		return;
	}

	// 2K should be enough
	icmpPingData->theSocket.RecvEx(2048, 0, (long)(icmpPingData->timeout), true, RecvPingComplete, icmpPingData);
}


class PingThreadData
{
public:
	IPSocket::Address addr;
	Error error;
	unsigned int* pingTimeArg;
	bool autoDelete;
	CompletionContainer<const PingOnceResult&> completion;
	unsigned long timeout;
	unsigned long startTime;

	PingThreadData(IPSocket::Address address, unsigned int* timeArg, bool autoDel)
		:	addr(address),
			pingTimeArg(timeArg),
			autoDelete(autoDel)
	{ }

	void Done(Error err, unsigned int pingTime)
	{
		error = err;
		if (pingTimeArg)
			*pingTimeArg = pingTime;
		completion.Complete(PingOnceResult(addr, err, pingTime, pingTimeArg));
		if (autoDelete)
			delete this;
	}

};


struct ip_option_information {
    unsigned char	Ttl;             // Time To Live
    unsigned char	Tos;             // Type Of Service
    unsigned char	Flags;           // IP header flags
    unsigned char	OptionsSize;     // Size in bytes of options data
    unsigned char*	OptionsData;     // Pointer to options data
};


struct icmp_echo_reply
{
    unsigned long					Address;         // Replying address
    unsigned long					Status;          // Reply IP_STATUS
    unsigned long					RoundTripTime;   // RTT in milliseconds
    unsigned short					DataSize;        // Reply data size in bytes
    unsigned short					Reserved;        // Reserved for system use
    void*							Data;            // Pointer to the reply data
    struct ip_option_information	Options;         // Reply options
}; 


#ifdef WIN32

extern "C" {

typedef HANDLE (WINAPI * IcmpCreateFileFunc)(VOID); 
typedef BOOL (WINAPI * IcmpCloseHandleFunc)(HANDLE); 
typedef DWORD (WINAPI * IcmpSendEchoFunc)(HANDLE, unsigned long, LPVOID, WORD, ip_option_information*, LPVOID, DWORD, DWORD);

IcmpCreateFileFunc	IcmpCreateFile;
IcmpCloseHandleFunc	IcmpCloseHandle;
IcmpSendEchoFunc	IcmpSendEcho;

};


static HINSTANCE icmpLib = NULL;
static bool loadedDll = false;


class AutoFreeLib
{
public:
	~AutoFreeLib()
	{
		if (loadedDll)
			FreeLibrary(icmpLib);
	}
} autoFreeLib;


static bool LoadIcmpDLL()
{
	if (loadedDll)
		return true;

	icmpLib = LoadLibrary("ICMP.dll");

	if (!icmpLib)
		return false;

	IcmpCreateFile = (IcmpCreateFileFunc)GetProcAddress(icmpLib, "IcmpCreateFile");
	IcmpCloseHandle = (IcmpCloseHandleFunc)GetProcAddress(icmpLib, "IcmpCloseHandle");
	IcmpSendEcho = (IcmpSendEchoFunc)GetProcAddress(icmpLib, "IcmpSendEcho");

	loadedDll = true;

	return true;
}


static unsigned int __stdcall PingThread(void* arg)
{
	PingThreadData* pingThreadData = (PingThreadData*)arg;
	
	HANDLE icmpHandle = IcmpCreateFile();

	timeBeginPeriod(1);

	pingThreadData->startTime = timeGetTime();

	struct ReplyBufferRec
	{
		icmp_echo_reply        echoReply;
		unsigned char          data[128];
	};

	ReplyBufferRec		   replyBuffer;

	DWORD result = IcmpSendEcho (icmpHandle, *(unsigned long*)&(pingThreadData->addr.GetAddress()), 
		NULL, 0, NULL, &replyBuffer, sizeof(ReplyBufferRec), pingThreadData->timeout);

	if (!result)
		pingThreadData->Done(Error_HostUnreachable, 0);
	else
		pingThreadData->Done(Error_Success, timeGetTime() - pingThreadData->startTime);

	timeEndPeriod(1);

	IcmpCloseHandle(icmpHandle);

	return 0;
}
#endif


Error WONAPI::IcmpPingOnce(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout, bool async, const CompletionContainer<const PingOnceResult&>& completion)
{
	Error err = Error_Pending;

	if (pingTime)
		*pingTime = 0;

	IPSocket::Address icmpAddr(addr.GetAddress(), 0);

#ifdef WIN32
	bool useRawSocket = (WSSocket::GetWinsock()->GetVersion() & 0x00FF) >= 2;

	if (useRawSocket)
	{
#endif
		IcmpPingData* icmpPingData = new IcmpPingData(icmpAddr, pingTime, async);
		if (!icmpPingData)
		{
			completion.Complete(PingOnceResult(icmpAddr, Error_OutOfMemory, 0, pingTime));
			return Error_OutOfMemory;
		}


		err = icmpPingData->theSocket.Open(&icmpAddr);	// Opening a datagram socket should never block
		if (err != Error_Success)
		{
			completion.Complete(PingOnceResult(icmpAddr, err, 0, pingTime));
			return err;
		}

		icmpPingData->completion = completion;
		icmpPingData->timeout = timeout;

		// Fill in ICMP header
		icmpPingData->packet.header.type = ICMP_ECHO;
		icmpPingData->packet.header.code = 0;
		icmpPingData->packet.header.checksum = 0;
		icmpPingData->packet.header.id = (unsigned long)icmpPingData;	// convert address to a ulong
		icmpPingData->packet.data.time = timeGetTime();
		icmpPingData->packet.header.checksum = getLittleEndian(checksum((unsigned short*)&(icmpPingData->packet), sizeof(IcmpPacket)));

		icmpPingData->theSocket.SendEx(sizeof(IcmpPacket), &(icmpPingData->packet), timeout, true, false, SendPingComplete, icmpPingData);

		err = Error_Pending;
		if (!async)
		{
			WSSocket::PumpUntil(icmpPingData->doneEvent, timeout);
			//icmpPingData->doneEvent.WaitFor();
			err = icmpPingData->error;
			delete icmpPingData;
		}
#ifdef WIN32
	}
	else // if (!useRawSocket)
	{
		if (!LoadIcmpDLL())
		{
			completion.Complete(PingOnceResult(icmpAddr, Error_OutOfMemory, 0, pingTime));
			return Error_OutOfMemory;
		}

		PingThreadData* pingThreadData = new PingThreadData(icmpAddr, pingTime, async);
		if (!pingThreadData)
		{
			completion.Complete(PingOnceResult(icmpAddr, Error_OutOfMemory, 0, pingTime));
			return Error_OutOfMemory;
		}

		pingThreadData->completion = completion;
		pingThreadData->timeout = timeout;

#ifdef _MTAPI
		DWORD threadId;
		HANDLE pingThreadHandle = (HANDLE)_beginthreadex(0, 0, PingThread, pingThreadData, 0, (unsigned int*)&threadId);
		if (pingThreadHandle == NULL)
		{
			completion.Complete(PingOnceResult(icmpAddr, Error_OutOfMemory, 0, pingTime));
			return Error_OutOfMemory;
		}

		if (!async)
		{
			WaitForSingleObject(pingThreadHandle, INFINITE);
			err = pingThreadData->error;
			delete pingThreadData;
			CloseHandle(pingThreadHandle);
		}
#else
		PingThread(pingThreadData);
		if (!async)
		{
			err = pingThreadData->error;
			delete pingThreadData;
		}
#endif
	}
#endif
	return err;
}


class MultiplePingData
{
public:
	Error error;
	IPSocket::Address addr;
	unsigned int lowestPing;
	unsigned int numPings;
	unsigned int numPingsDone;
	unsigned int numPingsDoneSuccessful;
	unsigned int pingSoFar;
	unsigned int* pingTimeArg;
	bool useAverage;
	CompletionContainer<const PingResult&> completion;
	Event doneEvent;
	bool autoDelete;
	unsigned long timeout;
	unsigned long beginTime;
	unsigned long timeoutPerPing;

	MultiplePingData(const IPSocket::Address& address, unsigned int numPing, unsigned int* timeArg, bool autoDel)
		:	pingTimeArg(timeArg),
			addr(address),
			autoDelete(autoDel),
			pingSoFar(0),
			lowestPing(0),
			numPingsDone(0),
			numPingsDoneSuccessful(0),
			numPings(numPing)
	{
		timeBeginPeriod(1);
	}

	void Done(Error err)
	{
		error = err;
		unsigned int average = numPingsDoneSuccessful ? pingSoFar / numPingsDoneSuccessful : 0;
		unsigned int finalPing = useAverage ? average : lowestPing;
		if (pingTimeArg)
			*pingTimeArg = finalPing;
		completion.Complete(PingResult(addr, err, lowestPing, average, pingTimeArg));
		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
		timeEndPeriod(1);
	}
};


static void DonePingOnce(const PingOnceResult& result, MultiplePingData* pingData)
{
	if (result.error != Error_Success)
	{
		pingData->error = result.error;
	}
	else
	{
		pingData->numPingsDoneSuccessful++;
		pingData->pingSoFar += result.pingTime;
		if (!pingData->lowestPing || result.pingTime < pingData->lowestPing)
			pingData->lowestPing = result.pingTime;
	}
	pingData->numPingsDone++;
	if (pingData->numPingsDone == pingData->numPings || pingData->beginTime + pingData->timeout < timeGetTime())
		pingData->Done(pingData->lowestPing ? Error_Success : pingData->error);
	else
		IcmpPingOnceEx(pingData->addr, 0, pingData->timeoutPerPing, true, DonePingOnce, pingData);
}


Error WONAPI::IcmpPing(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout,
			   unsigned int numPings, unsigned long timeoutPerPing, bool setPingTimeArgToAverage, bool async,
			   const CompletionContainer<const PingResult&>& completion)
{
	if (pingTime)
		*pingTime = 0;

	if (numPings == 0)
	{
		completion.Complete(PingResult(addr, Error_InvalidParams, 0, 0, pingTime));
		return Error_InvalidParams;
	}

	MultiplePingData* pingData = new MultiplePingData(addr, numPings, pingTime, async);
	if (!pingData)
	{
		completion.Complete(PingResult(addr, Error_OutOfMemory, 0, 0, pingTime));
		return Error_OutOfMemory;
	}

	pingData->error = Error_Timeout;
	pingData->useAverage = setPingTimeArgToAverage;
	pingData->timeout = timeout;
	pingData->completion = completion;
	pingData->timeoutPerPing = timeoutPerPing;
	pingData->beginTime = timeGetTime();

	IcmpPingOnceEx(addr, 0, timeoutPerPing, true, DonePingOnce, pingData);

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(pingData->doneEvent, timeout);
		//pingData->doneEvent.WaitFor();
		err = pingData->error;
		delete pingData;
	}

	return err;
}


class TitanTCPPingOnceData
{
public:
	Error error;
	IPSocket::Address addr;
	bool includeConnectTime;
	bool autoDelete;
	unsigned long timeout;
	unsigned int* pingTimeArg;
	CompletionContainer<const PingOnceResult&> completion;
	Event doneEvent;
	unsigned long startTime;
	IPSocket* theSocket;
	TMsgSocket titanSocket;

	TitanTCPPingOnceData(TCPSocket* thisSocket, unsigned char lengthFieldSize)
		:	theSocket(thisSocket),
			titanSocket(thisSocket, true, lengthFieldSize)
	{
		timeBeginPeriod(1);
	}

	void Done(Error err, unsigned int time)
	{
		error = err;
		if (pingTimeArg)
			*pingTimeArg = time;
		completion.Complete(PingOnceResult(addr, error, time, pingTimeArg));
		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
		timeEndPeriod(1);
	}
};


static void TitanTCPPingOnceDoneRecv(const TMsgSocket::RecvBaseMsgResult& result, TitanTCPPingOnceData* pingData)
{
	unsigned long pingTime = 0;
	Error err;
	if (!result.msg)
		err = result.closed ? Error_ConnectionClosed : Error_Timeout;
	else
	{
		if (result.msg->GetServiceType() != CommonService || result.msg->GetMessageType() != MiniCommPingReply)
			err = Error_InvalidMessage;
		else
		{
			pingTime = timeGetTime() - pingData->startTime;
			//MMsgCommPingReply reply(*(MiniMessage*)(result.msg));
			err = Error_Success;
		}
		delete result.msg;
	}
	pingData->Done(err, 0);
}


static void TitanTCPPingOnceDoneOpen(const Socket::OpenResult& result, TitanTCPPingOnceData* pingData)
{
	if (result.error != Error_Success)
	{
		pingData->Done(result.error, 0);
		return;
	}
	
	MMsgCommPing pingMsg;

	if (!pingData->includeConnectTime)
		pingData->startTime = timeGetTime();

	pingData->titanSocket.SendBaseMsg(pingMsg, -1, true, true);

	pingData->titanSocket.RecvBaseMsgEx(NULL, pingData->timeout, true, TitanTCPPingOnceDoneRecv, pingData);
}


Error WONAPI::TitanTCPPingOnce(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout,
							   unsigned char lengthFieldSize, bool includeConnectTime, bool async,
							   const CompletionContainer<const PingOnceResult&>& completion)
{
	if (pingTime)
		*pingTime = 0;

	TCPSocket* theSocket = new TCPSocket;
	if (!theSocket)
	{
		completion.Complete(PingOnceResult(addr, Error_OutOfMemory, 0, pingTime));
		return Error_OutOfMemory;
	}

	TitanTCPPingOnceData* pingData = new TitanTCPPingOnceData(theSocket, lengthFieldSize);
	if (!pingData)
	{
		delete theSocket;
		completion.Complete(PingOnceResult(addr, Error_OutOfMemory, 0, pingTime));
		return Error_OutOfMemory;
	}

	pingData->addr = addr;
	pingData->includeConnectTime = includeConnectTime;
	pingData->autoDelete = async;
	pingData->timeout = timeout;
	pingData->pingTimeArg = pingTime;
	pingData->completion = completion;
	pingData->startTime = timeGetTime();

	theSocket->OpenEx(&addr, timeout, true, TitanTCPPingOnceDoneOpen, pingData);

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(pingData->doneEvent, timeout);
		//pingData->doneEvent.WaitFor();
		err = pingData->error;
		delete pingData;
	}

	return err;
}


class TitanTCPPingData
{
public:
	Error error;
	IPSocket::Address addr;
	bool includeConnectTime;
	bool autoDelete;
	unsigned long timeout;
	unsigned int* pingTimeArg;
	CompletionContainer<const PingResult&> completion;
	Event doneEvent;
	unsigned long startTime;
	unsigned long beginTime;
	unsigned long timeoutPerPing;
	IPSocket* theSocket;
	TMsgSocket titanSocket;

	unsigned char lengthFieldSize;
	unsigned int lowestPing;
	unsigned int numPings;
	unsigned int numPingsDone;
	unsigned int numPingsDoneSuccessful;
	unsigned int pingSoFar;
	bool useAverage;


	TitanTCPPingData(TCPSocket* thisSocket, unsigned char lengthFieldSize)
		:	theSocket(thisSocket),
			titanSocket(thisSocket, true, lengthFieldSize),
			numPingsDone(0),
			numPingsDoneSuccessful(0),
			pingSoFar(0),
			lowestPing(0)
	{
		timeBeginPeriod(1);
	}

	void Done()
	{
		if (lowestPing != 0)
			error = Error_Success;

		unsigned int average = numPingsDoneSuccessful ? pingSoFar / numPingsDoneSuccessful : 0;
		unsigned int finalPing = useAverage ? average : lowestPing;
		if (pingTimeArg)
			*pingTimeArg = finalPing;
		completion.Complete(PingResult(addr, error, lowestPing, average, pingTimeArg));

		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
		timeEndPeriod(1);
	}
};


static void TitanTCPPingSendPing(TitanTCPPingData* pingData);


static void TitanTCPPingDoneRecv(const TMsgSocket::RecvBaseMsgResult& result, TitanTCPPingData* pingData)
{
	pingData->numPingsDone++;
	if (!result.msg)
		pingData->error = result.closed ? Error_ConnectionClosed : Error_Timeout;
	else
	{
		if (result.msg->GetServiceType() != CommonService || result.msg->GetMessageType() != MiniCommPingReply)
			pingData->error = Error_InvalidMessage;
		else
		{
			unsigned long pingTime = timeGetTime() - pingData->startTime;
			
			pingData->numPingsDoneSuccessful++;
			pingData->pingSoFar += pingTime;
			if (!pingData->lowestPing || pingTime < pingData->lowestPing)
				pingData->lowestPing = pingTime;
		}
		delete result.msg;
	}
	if (pingData->numPings == pingData->numPingsDone || pingData->beginTime + pingData->timeout < timeGetTime())
		pingData->Done();
	else
		TitanTCPPingSendPing(pingData);
}


static void TitanTCPPingSendPing(TitanTCPPingData* pingData)
{
	MMsgCommPing pingMsg;

	pingData->startTime = timeGetTime();

	pingData->titanSocket.SendBaseMsg(pingMsg, -1, true, true);
	pingData->titanSocket.RecvBaseMsgEx(NULL, pingData->timeoutPerPing, true, TitanTCPPingDoneRecv, pingData);
}


static void TitanTCPPingDoneOpen(const Socket::OpenResult& result, TitanTCPPingData* pingData)
{
	if (result.error != Error_Success)
	{
		pingData->error = result.error;
		pingData->Done();
		return;
	}
	TitanTCPPingSendPing(pingData);
}


static void TitanTCPPingDoneOne(const PingOnceResult& result, TitanTCPPingData* pingData)
{
	pingData->numPingsDone++;
	if (result.error != Error_Success)
		pingData->error = result.error;
	else
	{		
		pingData->numPingsDoneSuccessful++;
		pingData->pingSoFar += result.pingTime;
		if (!pingData->lowestPing || result.pingTime < pingData->lowestPing)
			pingData->lowestPing = result.pingTime;
	}
	if (pingData->numPings == pingData->numPingsDone || pingData->beginTime + pingData->timeout < timeGetTime())
		pingData->Done();
	else
		TitanTCPPingOnceEx(pingData->addr, 0, pingData->timeoutPerPing, pingData->lengthFieldSize, pingData->includeConnectTime, true, TitanTCPPingDoneOne, pingData);
}


Error WONAPI::TitanTCPPing(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout,
						   unsigned char lengthFieldSize, bool includeConnectTime, unsigned int numPings,
						   unsigned long timeoutPerPing, bool setPingTimeArgToAverage, bool async,
						   const CompletionContainer<const PingResult&>& completion)
{
	if (pingTime)
		*pingTime = 0;

	if (numPings == 0)
	{
		completion.Complete(PingResult(addr, Error_InvalidParams, 0, 0, pingTime));
		return Error_InvalidParams;
	}

	TCPSocket* theSocket = new TCPSocket;
	if (!theSocket)
	{
		completion.Complete(PingResult(addr, Error_OutOfMemory, 0, 0, pingTime));
		return Error_OutOfMemory;
	}

	TitanTCPPingData* pingData = new TitanTCPPingData(theSocket, lengthFieldSize);
	if (!pingData)
	{
		delete theSocket;
		completion.Complete(PingResult(addr, Error_OutOfMemory, 0, 0, pingTime));
		return Error_OutOfMemory;
	}

	pingData->error = Error_Timeout;
	pingData->addr = addr;
	pingData->includeConnectTime = includeConnectTime;
	pingData->autoDelete = async;
	pingData->timeout = timeout;
	pingData->pingTimeArg = pingTime;
	pingData->completion = completion;
	pingData->beginTime = timeGetTime();
	pingData->numPings = numPings;
	pingData->useAverage = setPingTimeArgToAverage;
	pingData->timeoutPerPing = timeoutPerPing;
	pingData->lengthFieldSize = lengthFieldSize;

	if (includeConnectTime || numPings == 1)
		TitanTCPPingOnceEx(addr, 0, timeoutPerPing, lengthFieldSize, includeConnectTime, true, TitanTCPPingDoneOne, pingData);
	else // i (!includeConnectTime && numPings != 1)
		theSocket->OpenEx(&addr, timeoutPerPing, true, TitanTCPPingDoneOpen, pingData);

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(pingData->doneEvent, timeout);
		//pingData->doneEvent.WaitFor();
		err = pingData->error;
		delete pingData;
	}

	return err;
}


class TitanUDPPingOnceData
{
public:
	Error error;
	IPSocket::Address addr;
	bool autoDelete;
	unsigned long timeout;
	unsigned int* pingTimeArg;
	CompletionContainer<const PingOnceResult&> completion;
	Event doneEvent;
	unsigned long startTime;
	UDPSocket* theSocket;
	TMsgSocket titanSocket;

	TitanUDPPingOnceData(UDPSocket* thisSocket)
		:	theSocket(thisSocket),
			titanSocket(thisSocket, true)
	{
		timeBeginPeriod(1);
	}

	void Done(Error err, unsigned int time)
	{
		error = err;
		if (pingTimeArg)
			*pingTimeArg = time;
		completion.Complete(PingOnceResult(addr, error, time, pingTimeArg));
		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
		timeEndPeriod(1);
	}
};


static void TitanUDPPingOnceDoneRecv(const TMsgSocket::RecvBaseMsgResult& result, TitanUDPPingOnceData* pingData)
{
	unsigned long pingTime = 0;
	Error err;

	if (!result.msg)
		err = Error_Timeout;
	else
	{
		if (result.msg->GetServiceType() != CommonService || result.msg->GetMessageType() != MiniCommPingReply)
			err = Error_InvalidMessage;
		else
		{
			unsigned long pingTime = timeGetTime() - pingData->startTime;
			//MMsgCommPingReply reply(*(MiniMessage*)(result.msg));
			err = Error_Success;
		}
		delete result.msg;
	}
	pingData->Done(err, pingTime);
}


Error WONAPI::TitanUDPPingOnce(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout,
							   bool async, const CompletionContainer<const PingOnceResult&>& completion)
{
	if (pingTime)
		*pingTime = 0;

	UDPSocket* theSocket = new UDPSocket;
	if (!theSocket)
	{
		completion.Complete(PingOnceResult(addr, Error_OutOfMemory, 0, pingTime));
		return Error_OutOfMemory;
	}

	TitanUDPPingOnceData* pingData = new TitanUDPPingOnceData(theSocket);
	if (!pingData)
	{
		delete theSocket;
		completion.Complete(PingOnceResult(addr, Error_OutOfMemory, 0, pingTime));
		return Error_OutOfMemory;
	}

	pingData->addr = addr;
	pingData->autoDelete = async;
	pingData->timeout = timeout;
	pingData->pingTimeArg = pingTime;
	pingData->completion = completion;

	theSocket->Open(&addr);	// will complete immediately

	MMsgCommPing pingMsg;

	pingData->startTime = timeGetTime();

	pingData->titanSocket.SendBaseMsg(pingMsg, -1, true, true);
	pingData->titanSocket.RecvBaseMsgEx(NULL, timeout, true, TitanUDPPingOnceDoneRecv, pingData);

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(pingData->doneEvent, timeout);
		//pingData->doneEvent.WaitFor();
		err = pingData->error;
		delete pingData;
	}

	return err;
}


class TitanUDPPingData
{
public:
	Error error;
	IPSocket::Address addr;
	bool autoDelete;
	unsigned int* pingTimeArg;
	CompletionContainer<const PingResult&> completion;
	Event doneEvent;
	unsigned long timeoutPerPing;
	unsigned long timeout;
	unsigned long beginTime;
	unsigned int lowestPing;
	unsigned int numPings;
	unsigned int numPingsDone;
	unsigned int numPingsDoneSuccessful;
	unsigned int pingSoFar;
	bool useAverage;

	TitanUDPPingData()
		:	lowestPing(0),
			numPingsDone(0),
			numPingsDoneSuccessful(0),
			pingSoFar(0)
	{
		timeBeginPeriod(1);
	}

	void Done()
	{
		if (lowestPing != 0)
			error = Error_Success;

		unsigned int average = numPingsDoneSuccessful ? pingSoFar / numPingsDoneSuccessful : 0;
		unsigned int finalPing = useAverage ? average : lowestPing;
		if (pingTimeArg)
			*pingTimeArg = finalPing;
		completion.Complete(PingResult(addr, error, lowestPing, average, pingTimeArg));

		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
		timeEndPeriod(1);
	}

};


static void TitanUDPPingDoneOne(const PingOnceResult& result, TitanUDPPingData* pingData)
{
	pingData->numPingsDone++;
	if (result.error != Error_Success)
		pingData->error = result.error;
	else
	{		
		pingData->numPingsDoneSuccessful++;
		pingData->pingSoFar += result.pingTime;
		if (!pingData->lowestPing || result.pingTime < pingData->lowestPing)
			pingData->lowestPing = result.pingTime;
	}
	if (pingData->numPings == pingData->numPingsDone || pingData->beginTime + pingData->timeout < timeGetTime())
		pingData->Done();
	else
		TitanUDPPingOnceEx(pingData->addr, 0, pingData->timeoutPerPing, true, TitanUDPPingDoneOne, pingData);
}


Error WONAPI::TitanUDPPing(const IPSocket::Address& addr, unsigned int* pingTime, unsigned long timeout,
						   unsigned int numPings, unsigned long timeoutPerPing, bool setPingTimeArgToAverage, 
						   bool async, const CompletionContainer<const PingResult&>& completion)
{
	if (pingTime)
		*pingTime = 0;

	TitanUDPPingData* pingData = new TitanUDPPingData;
	if (!pingData)
	{
		completion.Complete(PingResult(addr, Error_OutOfMemory, 0, 0, pingTime));
		return Error_OutOfMemory;
	}

	pingData->error = Error_Timeout;
	pingData->addr = addr;
	pingData->autoDelete = async;
	pingData->pingTimeArg = pingTime;
	pingData->completion = completion;
	pingData->timeoutPerPing = timeoutPerPing;
	pingData->timeout = timeout;
	pingData->beginTime = timeGetTime();
	pingData->useAverage = setPingTimeArgToAverage;
	pingData->numPings = numPings;

	TitanUDPPingOnceEx(addr, 0, timeoutPerPing, true, TitanUDPPingDoneOne, pingData);

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(pingData->doneEvent, timeout);
		//pingData->doneEvent.WaitFor();
		err = pingData->error;
		delete pingData;
	}

	return err;
}


struct PingStruct
{
	IPSocket::Address addr;
	unsigned int ping;
};


struct ltPingStruct
{
	bool operator()(const PingStruct& ps1, const PingStruct& ps2) const
	{
		if (!ps1.ping)
			return false;
		if (!ps2.ping)
			return true;
		return ps1.ping < ps2.ping;
	}
};




class OrderServersData
{
public:
	std::multiset<PingStruct, ltPingStruct> sortedSet;

	IPSocket::Address* addrs;
	unsigned int numAddrs;
	unsigned int* numGoodAddrsArg;
	unsigned int numGoodAddrs;
	unsigned int numPingsPerServer;
	unsigned long timeoutPerPing;
	unsigned long timeoutPerServer;
	unsigned int* pingArrayArg;
	unsigned int serverIndex;
	unsigned int numAddrsDone;
	unsigned char lengthFieldSize;
	bool includeConnectTime;
	CompletionContainer<const OrderServersResult&> completion;
	bool useAveragePing;
	bool autoDelete;
	Error error;
	Event doneEvent;
	CriticalSection crit;

	void Done()
	{
		if (numGoodAddrs != 0)
			error = Error_Success;

		if (numGoodAddrsArg)
			*numGoodAddrsArg = numGoodAddrs;

		IPSocket::Address* destAddrs = addrs;
		unsigned int* pingArrayDest = pingArrayArg;

		std::multiset<PingStruct, ltPingStruct>::iterator itor = sortedSet.begin();
		while (itor != sortedSet.end())
		{
			*destAddrs = (*itor).addr;
			destAddrs++;
			if (pingArrayDest)
			{
				*pingArrayDest = (*itor).ping;
				pingArrayDest++;
			}
			itor++;
		}
		
		completion.Complete(OrderServersResult(error, addrs, numAddrs, numGoodAddrs, numGoodAddrsArg));
		
		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
	}
};


static void OrderServersByIcmpPingDoneOne(const PingResult& result, OrderServersData* orderData)
{
	AutoCrit crit(orderData->crit);

	orderData->numAddrsDone++;
	
	if (result.error != Error_Success)
		orderData->error = result.error;
	else
		orderData->numGoodAddrs++;

	PingStruct ps;
	ps.addr = result.addr;
	ps.ping = result.pingTime;

	orderData->sortedSet.insert(std::multiset<PingStruct, ltPingStruct>::value_type(ps));

	if (orderData->serverIndex != orderData->numAddrs)
		IcmpPingEx(orderData->addrs[orderData->serverIndex++], 0, orderData->timeoutPerServer, orderData->numPingsPerServer, orderData->timeoutPerPing, orderData->useAveragePing, true, OrderServersByIcmpPingDoneOne, orderData);
	else if (orderData->numAddrsDone == orderData->numAddrs)
	{
		crit.Leave();
		orderData->Done();
	}
}


Error WONAPI::OrderServersByIcmpPing(IPSocket::Address* addresses, unsigned int numAddresses,
							 unsigned int* pingArray, unsigned int* numGoodAddrs,
							 unsigned int maxSimultaniousPings, unsigned int numPingsPerServer,
							 unsigned long timeoutPerServer, unsigned long timeoutPerPing,
							 bool useAveragePing, bool async, const CompletionContainer<const OrderServersResult&>& completion)
{
	if (numGoodAddrs)
		*numGoodAddrs = 0;

	if (!numAddresses || !addresses)
	{
		completion.Complete(OrderServersResult(Error_InvalidParams, addresses, numAddresses, 0, numGoodAddrs));
		return Error_InvalidParams;
	}
	
	OrderServersData* orderData = new OrderServersData;
	if (!orderData)
	{
		completion.Complete(OrderServersResult(Error_OutOfMemory, addresses, numAddresses, 0, numGoodAddrs));
		return Error_OutOfMemory;
	}

	orderData->addrs = addresses;
	orderData->numAddrs = numAddresses;
	orderData->numGoodAddrs = 0;
	orderData->numGoodAddrsArg = numGoodAddrs;
	orderData->error = Error_Success;
	orderData->numPingsPerServer = numPingsPerServer;
	orderData->timeoutPerServer = timeoutPerServer;
	orderData->timeoutPerPing = timeoutPerPing;
	orderData->useAveragePing = useAveragePing;
	orderData->pingArrayArg = pingArray;
	orderData->autoDelete = async;
	orderData->serverIndex = 0;
	orderData->numAddrsDone = 0;

	unsigned int numSimultanious = maxSimultaniousPings > numAddresses ? numAddresses : (maxSimultaniousPings > 0 ? maxSimultaniousPings : 1);

	AutoCrit crit(orderData->crit);
	
	while (orderData->serverIndex < numSimultanious)
		IcmpPingEx(addresses[orderData->serverIndex++], 0, timeoutPerServer, numPingsPerServer, timeoutPerPing, useAveragePing, true, OrderServersByIcmpPingDoneOne, orderData);

	crit.Leave();

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(orderData->doneEvent);
		//orderData->doneEvent.WaitFor();
		err = orderData->error;
		delete orderData;
	}

	return err;
}


static void OrderServersByTitanTCPPingDoneOne(const PingResult& result, OrderServersData* orderData)
{
	AutoCrit crit(orderData->crit);

	orderData->numAddrsDone++;
	
	if (result.error != Error_Success)
		orderData->error = result.error;
	else
		orderData->numGoodAddrs++;

	PingStruct ps;
	ps.addr = result.addr;
	ps.ping = result.pingTime;

	orderData->sortedSet.insert(std::multiset<PingStruct, ltPingStruct>::value_type(ps));

	if (orderData->serverIndex != orderData->numAddrs)
		TitanTCPPingEx(orderData->addrs[orderData->serverIndex++], 0, orderData->timeoutPerServer, orderData->lengthFieldSize, orderData->includeConnectTime, orderData->numPingsPerServer, orderData->timeoutPerPing, orderData->useAveragePing, true, OrderServersByTitanTCPPingDoneOne, orderData);
	else if (orderData->numAddrsDone == orderData->numAddrs)
	{
		crit.Leave();
		orderData->Done();
	}
}


Error WONAPI::OrderServersByTitanTCPPing(IPSocket::Address* addresses, unsigned int numAddresses,
								 unsigned int* pingArray, unsigned char lengthFieldSize,
								 bool includeConnectTime,  unsigned int* numGoodAddrs,
								 unsigned int maxSimultaniousPings, unsigned int numPingsPerServer,
								 unsigned long timeoutPerServer, unsigned long timeoutPerPing,
								 bool useAveragePing, bool async, const CompletionContainer<const OrderServersResult&>& completion)
{
	if (numGoodAddrs)
		*numGoodAddrs = 0;

	if (!numAddresses || !addresses)
	{
		completion.Complete(OrderServersResult(Error_InvalidParams, addresses, numAddresses, 0, numGoodAddrs));
		return Error_InvalidParams;
	}
	
	OrderServersData* orderData = new OrderServersData;
	if (!orderData)
	{
		completion.Complete(OrderServersResult(Error_OutOfMemory, addresses, numAddresses, 0, numGoodAddrs));
		return Error_OutOfMemory;
	}

	orderData->addrs = addresses;
	orderData->numAddrs = numAddresses;
	orderData->numGoodAddrs = 0;
	orderData->numGoodAddrsArg = numGoodAddrs;
	orderData->error = Error_Success;
	orderData->numPingsPerServer = numPingsPerServer;
	orderData->timeoutPerServer = timeoutPerServer;
	orderData->timeoutPerPing = timeoutPerPing;
	orderData->useAveragePing = useAveragePing;
	orderData->pingArrayArg = pingArray;
	orderData->autoDelete = async;
	orderData->serverIndex = 0;
	orderData->numAddrsDone = 0;
	orderData->includeConnectTime = includeConnectTime;
	orderData->lengthFieldSize = lengthFieldSize;

	unsigned int numSimultanious = maxSimultaniousPings > numAddresses ? numAddresses : (maxSimultaniousPings > 0 ? maxSimultaniousPings : 1);

	AutoCrit crit(orderData->crit);
	
	while (orderData->serverIndex < numSimultanious)
		TitanTCPPingEx(addresses[orderData->serverIndex++], 0, timeoutPerServer, lengthFieldSize, includeConnectTime, numPingsPerServer, timeoutPerPing, useAveragePing, true, OrderServersByTitanTCPPingDoneOne, orderData);

	crit.Leave();

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(orderData->doneEvent);
		//orderData->doneEvent.WaitFor();
		err = orderData->error;
		delete orderData;
	}

	return err;
}


static void OrderServersByTitanUDPPingDoneOne(const PingResult& result, OrderServersData* orderData)
{
	AutoCrit crit(orderData->crit);

	orderData->numAddrsDone++;
	
	if (result.error != Error_Success)
		orderData->error = result.error;
	else
		orderData->numGoodAddrs++;

	PingStruct ps;
	ps.addr = result.addr;
	ps.ping = result.pingTime;

	orderData->sortedSet.insert(std::multiset<PingStruct, ltPingStruct>::value_type(ps));

	if (orderData->serverIndex != orderData->numAddrs)
		TitanUDPPingEx(orderData->addrs[orderData->serverIndex++], 0, orderData->timeoutPerServer, orderData->numPingsPerServer, orderData->timeoutPerPing, orderData->useAveragePing, true, OrderServersByTitanUDPPingDoneOne, orderData);
	else if (orderData->numAddrsDone == orderData->numAddrs)
	{
		crit.Leave();
		orderData->Done();
	}
}


Error WONAPI::OrderServersByTitanUDPPing(IPSocket::Address* addresses, unsigned int numAddresses,
								 unsigned int* pingArray, unsigned int* numGoodAddrs,
								 unsigned int maxSimultaniousPings, unsigned int numPingsPerServer,
								 unsigned long timeoutPerServer, unsigned long timeoutPerPing,
								 bool useAveragePing, bool async, const CompletionContainer<const OrderServersResult&>& completion)
{
	if (numGoodAddrs)
		*numGoodAddrs = 0;

	if (!numAddresses || !addresses)
	{
		completion.Complete(OrderServersResult(Error_InvalidParams, addresses, numAddresses, 0, numGoodAddrs));
		return Error_InvalidParams;
	}
	
	OrderServersData* orderData = new OrderServersData;
	if (!orderData)
	{
		completion.Complete(OrderServersResult(Error_OutOfMemory, addresses, numAddresses, 0, numGoodAddrs));
		return Error_OutOfMemory;
	}

	orderData->addrs = addresses;
	orderData->numAddrs = numAddresses;
	orderData->numGoodAddrs = 0;
	orderData->numGoodAddrsArg = numGoodAddrs;
	orderData->error = Error_Success;
	orderData->numPingsPerServer = numPingsPerServer;
	orderData->timeoutPerServer = timeoutPerServer;
	orderData->timeoutPerPing = timeoutPerPing;
	orderData->useAveragePing = useAveragePing;
	orderData->pingArrayArg = pingArray;
	orderData->autoDelete = async;
	orderData->serverIndex = 0;
	orderData->numAddrsDone = 0;

	unsigned int numSimultanious = maxSimultaniousPings > numAddresses ? numAddresses : (maxSimultaniousPings > 0 ? maxSimultaniousPings : 1);

	AutoCrit crit(orderData->crit);
	
	while (orderData->serverIndex < numSimultanious)
		TitanUDPPingEx(addresses[orderData->serverIndex++], 0, timeoutPerServer, numPingsPerServer, timeoutPerPing, useAveragePing, true, OrderServersByTitanUDPPingDoneOne, orderData);

	crit.Leave();

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(orderData->doneEvent);
		//orderData->doneEvent.WaitFor();
		err = orderData->error;
		delete orderData;
	}

	return err;
}


#include "wondll.h"


WONError WONPingIcmpOnce(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout)
{
	if (!addr || !pingTime)
		return Error_InvalidParams;
	IPSocket::Address tmpAddr(*addr);
	return IcmpPingOnce(tmpAddr, pingTime, timeout, false);
}


static void TranslatePingOnceCompletion(const PingOnceResult& result, HWONCOMPLETION hCompletion)
{
	WONComplete(hCompletion, (void*)result.error);
}


void WONPingIcmpOnceAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
					   HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (addr && pingTime)
	{
		IPSocket::Address tmpAddr(*addr);

		IcmpPingOnceEx(tmpAddr, pingTime, timeout, true, TranslatePingOnceCompletion, hCompletion);
		return;
	}
	WONComplete(hCompletion, (void*)err);
}



WONError WONPingIcmp(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
				  unsigned int numPings, unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage)
{
	if (!addr || !pingTime)
		return Error_InvalidParams;
	IPSocket::Address tmpAddr(*addr);
	return IcmpPing(tmpAddr, pingTime, timeout, numPings, timeoutPerPing, setPingTimeArgToAverage ? true : false);
}


static void TranslatePingCompletion(const PingResult& result, HWONCOMPLETION hCompletion)
{
	WONComplete(hCompletion, (void*)result.error);
}


void WONPingIcmpAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
				   unsigned int numPings, unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage,
				   HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (addr && pingTime)
	{
		IPSocket::Address tmpAddr(*addr);

		IcmpPingEx(tmpAddr, pingTime, timeout, numPings, timeoutPerPing, setPingTimeArgToAverage ? true : false,
			true, TranslatePingCompletion, hCompletion);
		return;
	}
	WONComplete(hCompletion, (void*)err);
}


WONError WONPingTitanTCPOnce(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
						  unsigned char lengthFieldSize, BOOL includeConnectTime)
{
	if (!addr || !pingTime)
		return Error_InvalidParams;
	IPSocket::Address tmpAddr(*addr);
	return TitanTCPPingOnce(tmpAddr, pingTime, timeout, lengthFieldSize, includeConnectTime ? true : false);
}


void WONPingTitanTCPOnceAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
						   unsigned char lengthFieldSize, BOOL includeConnectTime, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (addr && pingTime)
	{
		IPSocket::Address tmpAddr(*addr);

		TitanTCPPingOnceEx(tmpAddr, pingTime, timeout, lengthFieldSize, includeConnectTime ? true : false,
			true, TranslatePingOnceCompletion, hCompletion);
		return;
	}
	WONComplete(hCompletion, (void*)err);
}


WONError WONPingTitanTCP(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
					  unsigned char lengthFieldSize, BOOL includeConnectTime, unsigned int numPings,
					  unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage)
{
	if (!addr)
		return Error_InvalidParams;
	IPSocket::Address tmpAddr(*addr);
	return TitanTCPPing(tmpAddr, pingTime, timeout, lengthFieldSize, includeConnectTime ? true : false,
		numPings, timeoutPerPing, setPingTimeArgToAverage ? true : false);
}


void WONPingTitanTCPAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
					   unsigned char lengthFieldSize, BOOL includeConnectTime, unsigned int numPings,
					   unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (addr && pingTime)
	{
		IPSocket::Address tmpAddr(*addr);

		TitanTCPPingEx(tmpAddr, pingTime, timeout, lengthFieldSize, includeConnectTime ? true : false,
			numPings, timeoutPerPing, setPingTimeArgToAverage ? true : false,
			true, TranslatePingCompletion, hCompletion);
		return;
	}
	WONComplete(hCompletion, (void*)err);
}


WONError WONPingTitanUDPOnce(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout)
{
	if (!addr)
		return Error_InvalidParams;
	IPSocket::Address tmpAddr(*addr);
	return TitanUDPPingOnce(tmpAddr, pingTime, timeout);
}


void WONPingTitanUDPOnceAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
						   HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (addr && pingTime)
	{
		IPSocket::Address tmpAddr(*addr);

		TitanUDPPingOnceEx(tmpAddr, pingTime, timeout, true, TranslatePingOnceCompletion, hCompletion);
		return;
	}
	WONComplete(hCompletion, (void*)err);
}


WONError WONPingTitanUDP(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
					  unsigned int numPings, unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage)
{
	if (!addr)
		return Error_InvalidParams;
	IPSocket::Address tmpAddr(*addr);
	return TitanUDPPing(tmpAddr, pingTime, timeout, numPings, timeoutPerPing, setPingTimeArgToAverage ? true : false);
}


void WONPingTitanUDPAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
					   unsigned int numPings, unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage,
					   HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (addr && pingTime)
	{
		IPSocket::Address tmpAddr(*addr);

		TitanUDPPingEx(tmpAddr, pingTime, timeout, numPings, timeoutPerPing, setPingTimeArgToAverage ? true : false,
			true, TranslatePingCompletion, hCompletion);
		return;
	}
	WONComplete(hCompletion, (void*)err);
}


WONError WONPingOrderServersByIcmp(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
								unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
								unsigned int numPingsPerServer, unsigned long timeoutPerServer,
								unsigned long timeoutPerPing, BOOL useAveragePing)
{
	if (!addresses || !numAddresses || !numGoodAddrs)	// ok to ignore pingArray
		return Error_InvalidParams;

	IPSocket::Address* newAddrs = new IPSocket::Address[numAddresses];
	if (!newAddrs)
		return Error_OutOfMemory;
	array_auto_ptr<IPSocket::Address> autoAddrs(newAddrs);

	unsigned int i;
	for (i = 0; i < numAddresses; i++)
		newAddrs[i].Set(addresses[i]);
	
	Error err = OrderServersByIcmpPing(newAddrs, numAddresses, pingArray, numGoodAddrs, maxSimultaniousPings,
		numPingsPerServer, timeoutPerServer, timeoutPerPing, useAveragePing ? true : false, false);
	if (*numGoodAddrs)
	{
		for (i = 0; i < *numGoodAddrs; i++)
		{
			addresses[i].portNum = newAddrs[i].GetPort();
			in_addr inaddr = newAddrs[i].GetAddress();
			addresses[i].ipAddress = *(long*)(&inaddr);
		}
	}
	return err;
}


class WONPingOrderServersData
{
public:
	WONIPAddress* addresses;
	IPSocket::Address* newAddrs;
	HWONCOMPLETION hCompletion;

	~WONPingOrderServersData()
	{
		delete[] addresses;
	}
};


static void TranslateOrderServersCompletion(const OrderServersResult& result, WONPingOrderServersData* orderData)
{
	for (int i = 0; i < result.numGoodAddrs; i++)
	{
		orderData->addresses[i].portNum = orderData->newAddrs[i].GetPort();
		in_addr inaddr = orderData->newAddrs[i].GetAddress();
		orderData->addresses[i].ipAddress = *(long*)(&inaddr);
	}
	WONComplete(orderData->hCompletion, (void*)result.error);
	delete orderData;
}


void WONPingOrderServersByIcmpAsync(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
								 unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
								 unsigned int numPingsPerServer, unsigned long timeoutPerServer,
								 unsigned long timeoutPerPing, BOOL useAveragePing, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (addresses && numAddresses && numGoodAddrs)	// ok to ignore pingArray
	{
		err = Error_OutOfMemory;
		WONPingOrderServersData* orderData = new WONPingOrderServersData;
		if (orderData)
		{
			orderData->newAddrs = new IPSocket::Address[numAddresses];
			if (orderData->newAddrs)
			{
				orderData->addresses = addresses;
				orderData->hCompletion = hCompletion;

				for (int i = 0; i < numAddresses; i++)
					orderData->newAddrs[i].Set(addresses[i]);
	
				OrderServersByIcmpPingEx(orderData->newAddrs, numAddresses, pingArray, numGoodAddrs,
					maxSimultaniousPings, numPingsPerServer, timeoutPerServer, timeoutPerPing,
					useAveragePing ? true : false, true, TranslateOrderServersCompletion, orderData);
				return;
			}
			delete orderData;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


WONError WONPingOrderServersByTitanTCP(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
									unsigned char lengthFieldSize, BOOL includeConnectTime, 
									unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
									unsigned int numPingsPerServer, unsigned long timeoutPerServer,
									unsigned long timeoutPerPing, BOOL useAveragePing)
{
	if (!addresses || !numAddresses || !numGoodAddrs)	// ok to ignore pingArray
		return Error_InvalidParams;

	IPSocket::Address* newAddrs = new IPSocket::Address[numAddresses];
	if (!newAddrs)
		return Error_OutOfMemory;
	array_auto_ptr<IPSocket::Address> autoAddrs(newAddrs);

	unsigned int i;
	for (i = 0; i < numAddresses; i++)
		newAddrs[i].Set(addresses[i]);
	
	Error err = OrderServersByTitanTCPPing(newAddrs, numAddresses, pingArray, lengthFieldSize,
		includeConnectTime ? true : false, numGoodAddrs, maxSimultaniousPings, numPingsPerServer,
		timeoutPerServer, timeoutPerPing, useAveragePing ? true : false, false);
	if (*numGoodAddrs)
	{
		for (i = 0; i < *numGoodAddrs; i++)
		{
			addresses[i].portNum = newAddrs[i].GetPort();
			in_addr inaddr = newAddrs[i].GetAddress();
			addresses[i].ipAddress = *(long*)(&inaddr);
		}
	}
	return err;
}


void WONPingOrderServersByTitanTCPAsync(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
									 unsigned char lengthFieldSize, BOOL includeConnectTime, 
									 unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
									 unsigned int numPingsPerServer, unsigned long timeoutPerServer,
									 unsigned long timeoutPerPing, BOOL useAveragePing, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (addresses && numAddresses && numGoodAddrs)	// ok to ignore pingArray
	{
		err = Error_OutOfMemory;
		WONPingOrderServersData* orderData = new WONPingOrderServersData;
		if (orderData)
		{
			orderData->newAddrs = new IPSocket::Address[numAddresses];
			if (orderData->newAddrs)
			{
				orderData->addresses = addresses;
				orderData->hCompletion = hCompletion;

				for (int i = 0; i < numAddresses; i++)
					orderData->newAddrs[i].Set(addresses[i]);
	
				OrderServersByTitanTCPPingEx(orderData->newAddrs, numAddresses, pingArray, lengthFieldSize,
					includeConnectTime ? true : false, numGoodAddrs, maxSimultaniousPings,
					numPingsPerServer, timeoutPerServer, timeoutPerPing, useAveragePing ? true : false,
					true, TranslateOrderServersCompletion, orderData);
				return;
			}
			delete orderData;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


WONError WONPingOrderServersByTitanUDP(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
									unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
									unsigned int numPingsPerServer, unsigned long timeoutPerServer,
									unsigned long timeoutPerPing, BOOL useAveragePing)
{
	if (!addresses || !numAddresses || !numGoodAddrs)	// ok to ignore pingArray
		return Error_InvalidParams;

	IPSocket::Address* newAddrs = new IPSocket::Address[numAddresses];
	if (!newAddrs)
		return Error_OutOfMemory;
	array_auto_ptr<IPSocket::Address> autoAddrs(newAddrs);

	unsigned int i;
	for (i = 0; i < numAddresses; i++)
		newAddrs[i].Set(addresses[i]);
	
	Error err = OrderServersByTitanUDPPing(newAddrs, numAddresses, pingArray,
		numGoodAddrs, maxSimultaniousPings, numPingsPerServer,
		timeoutPerServer, timeoutPerPing, useAveragePing ? true : false, false);
	if (*numGoodAddrs)
	{
		for (i = 0; i < *numGoodAddrs; i++)
		{
			addresses[i].portNum = newAddrs[i].GetPort();
			in_addr inaddr = newAddrs[i].GetAddress();
			addresses[i].ipAddress = *(long*)(&inaddr);
		}
	}
	return err;
}


void WONPingOrderServersByTitanUDPAsync(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
									 unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
									 unsigned int numPingsPerServer, unsigned long timeoutPerServer,
									 unsigned long timeoutPerPing, BOOL useAveragePing, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (addresses && numAddresses && numGoodAddrs)	// ok to ignore pingArray
	{
		err = Error_OutOfMemory;
		WONPingOrderServersData* orderData = new WONPingOrderServersData;
		if (orderData)
		{
			orderData->newAddrs = new IPSocket::Address[numAddresses];
			if (orderData->newAddrs)
			{
				orderData->addresses = addresses;
				orderData->hCompletion = hCompletion;

				for (int i = 0; i < numAddresses; i++)
					orderData->newAddrs[i].Set(addresses[i]);
	
				OrderServersByTitanUDPPingEx(orderData->newAddrs, numAddresses, pingArray,
					numGoodAddrs, maxSimultaniousPings,
					numPingsPerServer, timeoutPerServer, timeoutPerPing, useAveragePing ? true : false,
					true, TranslateOrderServersCompletion, orderData);
				return;
			}
			delete orderData;
		}
	}
	WONComplete(hCompletion, (void*)err);
}

