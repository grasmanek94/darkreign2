#include "common/won.h"
#include "FactoryAPI.h"
#include "Authentication/AuthSocket.h"
#include "msg/Fact/SMsgFactStartProcess.h"
#include "msg/Fact/SMsgFactStatusReply.h"
#include "msg/Fact/SMsgFactStartProcessUnicode.h"
#include "msg/Fact/SMsgFactStopProcess.h"
#include "msg/Comm/SMsgCommStatusReply.h"
#include "msg/ServerStatus.h"
#include <list>
#include <string>
#include <set>


using std::string;
using std::list;
using std::set;


using namespace WONAPI;
using namespace WONMsg;
using namespace WONCommon;


typedef list<IPSocket::Address>	AddressList;
typedef set<IPSocket::Address>	AddressSet;
typedef list<unsigned short>	PortList;
typedef set<unsigned short>		PortSet;


class StartTitanServerData
{
public:
	Error					error;
	TCPSocket				tcpSocket;
	AuthSocket				authSocket;
	AddressList				factories;
	AddressList::iterator	curFactory;
	unsigned short*			startedOnPorts;
	unsigned short*			numStartedPorts;
	string					configurationName;
	IPSocket::Address		directoryServer;
	wstring					dirPath;
	wstring					displayName;
	bool					abortOnDirRegisterFailure;
	string					commandLineFragment;
	wstring					wideCommandLineFragment;
	bool					useWide;
	bool					replaceCommandLine;
	unsigned char			numPortsRequested;
	PortList				specificPorts;
	AddressList				authorizedIPs;
	long					timeout;
	bool					autoDelete;
	CompletionContainer<const StartTitanServerResult&> completion;
	Event					doneEvent;
	IPSocket::Address*		startedOnFactory;

	StartTitanServerData(Identity* ident)
		:	authSocket(ident, tcpSocket)
	{ }

	void Done(Error err, const IPSocket::Address& factory, const PortSet* ports)
	{
		error = err;
		if (startedOnFactory)
			*startedOnFactory = factory;
		StartTitanServerResult result(error, factory);
		if (ports)
		{			
			unsigned short maxPorts = 0;
			if (numStartedPorts)
			{
				maxPorts = *numStartedPorts;
				*numStartedPorts = 0;
			}
			PortSet::const_iterator itor = ports->begin();
			if (itor != ports->end())
			{
				do
				{
					if (startedOnPorts)
					{
						if (numStartedPorts)
						{
							if (*numStartedPorts < maxPorts)
								startedOnPorts[(*numStartedPorts)++] = *itor;
						}
						else
						{
							if (itor == ports->begin())
								*startedOnPorts = *itor;
						}
					}
					result.processPorts.push_back(*itor);
					itor++;
				} while (itor != ports->end());
			}
		}
		else if (numStartedPorts)
			*numStartedPorts = 0;
		completion.Complete(result);
		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
	}

	void TryNextFactoryServer(Error err);
};


static void StartTitanServerDoneRecvReply(const AuthSocket::AuthRecvBaseMsgResult& result, StartTitanServerData* startData)
{
	if (!result.msg)
		startData->TryNextFactoryServer(result.closed ? Error_ConnectionClosed : Error_Timeout);
	else
	{
		try
		{
			SMsgFactStatusReply msg(*(SmallMessage*)(result.msg));

			msg.Unpack();

			short status = msg.GetProcessStatus();
			if (status != StatusCommon_StartSuccess)
				startData->TryNextFactoryServer(status);
			else
				startData->Done(Error_Success, *startData->curFactory, &msg.GetProcessPortIDSet());
		}
		catch (...)
		{
			startData->TryNextFactoryServer(Error_InvalidMessage);
		}
		delete result.msg;
	}
}


static void StartTitanServerOpenDone(const Socket::OpenResult& result, StartTitanServerData* startData)
{
	if (result.error != Error_Success)
		startData->TryNextFactoryServer(result.error);
	else
	{
		SMsgFactStartProcess		msgAscii;
		SMsgFactStartProcessUnicode	msgUnicode;

		SMsgFactStartProcessBase* msg;
		
		if (startData->useWide)
		{
			msgUnicode.SetCmdLine(startData->wideCommandLineFragment);
			msg = &msgUnicode;
		}
		else
		{
			msgAscii.SetCmdLine(startData->commandLineFragment);
			msg = &msgAscii;
		}

		msg->SetProcessName(startData->configurationName);
		msg->SetAddCmdLine(!startData->replaceCommandLine);
		if (startData->timeout < 0)
			msg->SetWaitTime(0);
		else if (startData->timeout == 0)
			msg->SetWaitTime(1);
		else
			msg->SetWaitTime(startData->timeout);
		if (startData->directoryServer.IsValid())
			msg->SetDirServerAddress(startData->directoryServer.GetAddressString());
		msg->SetDisplayName(startData->displayName);
		msg->SetRegisterDir(startData->dirPath);
		msg->SetAbortRegFail(startData->abortOnDirRegisterFailure);
		msg->SetTotalPorts(startData->numPortsRequested);

		PortList::iterator itor = startData->specificPorts.begin();
		PortSet portSet;
		while (itor != startData->specificPorts.end())
		{
			portSet.insert(PortSet::value_type(*itor));
			itor++;
		}

		msg->SetPortSet(portSet);

		AddressList::iterator itor2 = startData->authorizedIPs.begin();
		FACT_SERV_TRUSTED_ADDRESS_SET addressSet;
		while (itor2 != startData->authorizedIPs.end())
		{
			addressSet.insert(FACT_SERV_TRUSTED_ADDRESS_SET::value_type((*itor2).GetAddressString()));
			itor2++;
		}

		msg->SetAuthorizedIPSet(addressSet);

		startData->authSocket.SendBaseMsg(*msg, true, startData->timeout, true, true);
		startData->authSocket.RecvBaseMsgEx(0, false, startData->timeout, true, StartTitanServerDoneRecvReply, startData);
	}
}


void StartTitanServerData::TryNextFactoryServer(Error err)
{
	if (++curFactory == factories.end())
		Done(err, IPSocket::Address(), 0);
	else
	{
		authSocket.Close(0);
		authSocket.ReleaseSession();
		tcpSocket.SetRemoteAddress(*curFactory);
		authSocket.OpenEx(true, timeout, true, StartTitanServerOpenDone, this);
	}
}


static Error StartTitanServerX(Identity* identity, const IPSocket::Address* factories,
							   unsigned int numFactories, IPSocket::Address* startedOnFactory,
							   const string& configurationName, const IPSocket::Address& directoryServer,
							   const wstring& dirPath, const wstring& displayName, bool abortOnDirRegisterFailure,
							   unsigned short* startedOnPorts, unsigned short* numStartedPorts,
							   const string& commandLineFragment, const wstring& wideCommandLineFragment, bool useWide,
							   bool replaceCommandLine, unsigned char numPortsRequested, unsigned char numSpecificPorts,
							   unsigned short* specificPortsArray, const IPSocket::Address* authorizedIPs,
							   unsigned short numAuthorizedIPs, long timeout, bool async,
							   const CompletionContainer<const StartTitanServerResult&>& completion)
{
	if (startedOnFactory)
		*startedOnFactory = IPSocket::Address();

	if (!numFactories)
	{
		completion.Complete(StartTitanServerResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	StartTitanServerData* startData = new StartTitanServerData(identity);
	if (!startData)
	{
		completion.Complete(StartTitanServerResult(Error_OutOfMemory, IPSocket::Address()));
		return Error_OutOfMemory;
	}

	startData->configurationName = configurationName;
	startData->directoryServer = directoryServer;
	startData->dirPath = dirPath;
	startData->displayName = displayName;
	startData->abortOnDirRegisterFailure = abortOnDirRegisterFailure;
	startData->commandLineFragment = commandLineFragment;
	startData->wideCommandLineFragment = wideCommandLineFragment;
	startData->useWide = useWide;
	startData->replaceCommandLine = replaceCommandLine;
	startData->numPortsRequested = numPortsRequested;
	startData->timeout = timeout;
	startData->autoDelete = async;
	startData->completion = completion;
	startData->startedOnPorts = startedOnPorts;
	startData->numStartedPorts = numStartedPorts;
	startData->startedOnFactory = startedOnFactory;


	int i;
	for (i = 0; i < numFactories; i++)
		startData->factories.push_back(factories[i]);

	for (i = 0; i < numSpecificPorts; i++)
		startData->specificPorts.push_back(specificPortsArray[i]);

	for (i = 0; i < numAuthorizedIPs; i++)
		startData->authorizedIPs.push_back(authorizedIPs[i]);

	startData->curFactory = startData->factories.begin();

	startData->tcpSocket.SetRemoteAddress(*startData->curFactory);
	startData->authSocket.OpenEx(true, timeout, true, StartTitanServerOpenDone, startData);

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(startData->doneEvent, timeout);
		//startData->doneEvent.WaitFor();
		err = startData->error;
		delete startData;
	}

	return err;
}


Error WONAPI::StartTitanServer(Identity* identity, const IPSocket::Address* factories,
							   unsigned int numFactories, IPSocket::Address* startedOnFactory,
							   const string& configurationName, const IPSocket::Address& directoryServer,
							   const WONString& dirPath, const WONString& displayName, bool abortOnDirRegisterFailure,
							   unsigned short* startedOnPorts, unsigned short* numStartedPorts,
							   const WONString& commandLineFragment, bool replaceCommandLine,
							   unsigned char numPortsRequested, unsigned char numSpecificPorts,
							   unsigned short* specificPortsArray, const IPSocket::Address* authorizedIPs,
							   unsigned short numAuthorizedIPs, long timeout, bool async,
							   const CompletionContainer<const StartTitanServerResult&>& completion)
{
	return StartTitanServerX(identity, factories, numFactories, startedOnFactory, configurationName,
		directoryServer, dirPath, displayName, abortOnDirRegisterFailure, startedOnPorts,
		numStartedPorts, string(), commandLineFragment, true, replaceCommandLine, numPortsRequested,
		numSpecificPorts, specificPortsArray, authorizedIPs, numAuthorizedIPs, timeout, async, completion);
}


class StopTitanServerData
{
public:
	TCPSocket tcpSocket;
	AuthSocket authSocket;
	Error error;
	IPSocket::Address factory;
	string configurationName;
	unsigned short processPort;
	unsigned long timeout;
	bool autoDelete;
	Event doneEvent;
	CompletionContainer<const StopTitanServerResult&> completion;

	StopTitanServerData(Identity* ident)
		:	authSocket(ident, tcpSocket)
	{ }


	void Done(Error err)
	{
		error = err;
		completion.Complete(StopTitanServerResult(err, factory, processPort));
		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
	}
};


static void StopTitanServerDoneRecvReply(const AuthSocket::AuthRecvBaseMsgResult& result, StopTitanServerData* stopData)
{
	if (!result.msg)
		stopData->Done(result.closed ? Error_ConnectionClosed : Error_Timeout);
	else
	{
		try
		{
			SMsgFactStatusReply msg(*(SmallMessage*)(result.msg));

			msg.Unpack();

			stopData->Done(msg.GetProcessStatus());
		}
		catch (...)
		{
			stopData->Done(Error_InvalidMessage);
		}
		delete result.msg;
	}
}


static void StopTitanServerDoneOpen(const Socket::OpenResult& result, StopTitanServerData* stopData)
{
	if (result.error != Error_Success)
		stopData->Done(result.error);
	else
	{
		SMsgFactStopProcess msg;

		msg.SetProcessName(stopData->configurationName);
		msg.SetProcessPortID(stopData->processPort);

		stopData->authSocket.SendBaseMsg(msg, true, stopData->timeout, true, true);
		stopData->authSocket.RecvBaseMsgEx(0, false, stopData->timeout, true, StopTitanServerDoneRecvReply, stopData);
	}
}



Error WONAPI::StopTitanServer(Identity* identity, const IPSocket::Address& factory, const string& configurationName,
							  unsigned short processPort, long timeout, bool async,
							  const CompletionContainer<const StopTitanServerResult&>& completion)
{
	StopTitanServerData* stopData = new StopTitanServerData(identity);
	if (!stopData)
	{
		completion.Complete(StopTitanServerResult(Error_OutOfMemory, factory, processPort));
		return Error_OutOfMemory;
	}

	stopData->configurationName = configurationName;
	stopData->processPort = processPort;
	stopData->timeout = timeout;
	stopData->autoDelete = async;
	stopData->factory = factory;
	stopData->completion = completion;

	stopData->tcpSocket.SetRemoteAddress(factory);
	stopData->authSocket.OpenEx(true, timeout, true, StopTitanServerDoneOpen, stopData);

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(stopData->doneEvent, timeout);
		//stopData->doneEvent.WaitFor();
		err = stopData->error;
		delete stopData;
	}

	return err;
}


#include "wondll.h"

static void internalSetWONIPAddr(WONIPAddress* addr, const IPSocket::Address& src)
{
	addr->portNum = src.GetPort();
	in_addr inaddr = src.GetAddress();
	addr->ipAddress = *(long*)(&inaddr);
}


WONError WONFactoryStartTitanServerW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* factories, unsigned int numFactories,
						   WONIPAddress* startedOnFactory, WON_CONST char* configurationName,
						   unsigned short* startedOnPorts, unsigned short* numStartedPorts,
						   WON_CONST wchar_t* commandLineFragment, BOOL replaceCommandLine,
						   unsigned char numPortsRequested, unsigned char numSpecificPorts,
						   unsigned short* specificPortsArray, WON_CONST WONIPAddress* authorizedIPs,
						   unsigned short numAuthorizedIPs, long timeout)
{
	Error err = Error_InvalidParams;
	if (numFactories && factories && (!numAuthorizedIPs || authorizedIPs))
	{
		err = Error_OutOfMemory;
		IPSocket::Address* factorySrvAddrs = new IPSocket::Address[numFactories];
		if (factorySrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoFactorySrvAddrs(factorySrvAddrs);

			IPSocket::Address* authoritzedAddrs = new IPSocket::Address[numAuthorizedIPs ? numAuthorizedIPs : 1];
			if (authoritzedAddrs)	// allocates at least one, to make the logic easier
			{
				array_auto_ptr<IPSocket::Address> autoAuthoritzedAddrs(authoritzedAddrs);

				int i;
				for (i = 0; i < numFactories; i++)
					factorySrvAddrs[i].Set(factories[i]);
				if (numAuthorizedIPs)
					for (i = 0; i < numAuthorizedIPs; i++)
						authoritzedAddrs[i].Set(authorizedIPs[i]);

				IPSocket::Address startedOn;

				err = StartTitanServer((Identity*)hWONAuth, factorySrvAddrs, numFactories,
					&startedOn, configurationName, startedOnPorts, numStartedPorts, commandLineFragment,
					replaceCommandLine ? true : false,
					numPortsRequested, numSpecificPorts, specificPortsArray, authoritzedAddrs,
					numAuthorizedIPs, timeout);
				if (startedOnFactory)
					internalSetWONIPAddr(startedOnFactory, startedOn);
			}
		}
	}
	return err;
}


WONError WONFactoryStartTitanServerA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* factories, unsigned int numFactories,
						   WONIPAddress* startedOnFactory, WON_CONST char* configurationName,
						   unsigned short* startedOnPorts, unsigned short* numStartedPorts,
						   WON_CONST char* commandLineFragment, BOOL replaceCommandLine,
						   unsigned char numPortsRequested, unsigned char numSpecificPorts,
						   unsigned short* specificPortsArray, WON_CONST WONIPAddress* authorizedIPs,
						   unsigned short numAuthorizedIPs, long timeout)
{
	return WONFactoryStartTitanServerW(hWONAuth, factories, numFactories, startedOnFactory, configurationName,
		startedOnPorts, numStartedPorts,
		WONString(commandLineFragment).GetUnicodeCString(), replaceCommandLine, numPortsRequested, numSpecificPorts, specificPortsArray,
		authorizedIPs, numAuthorizedIPs, timeout);
}


class WONFactoryStartData
{
public:
	WONIPAddress* startedOnFactory;
	HWONCOMPLETION hCompletion;
};


static void WONFactoryStartCompletionTranslator(const StartTitanServerResult& result, WONFactoryStartData* startData)
{
	if (startData->startedOnFactory)
		internalSetWONIPAddr(startData->startedOnFactory, result.factory);
	WONComplete(startData->hCompletion, (void*)result.error);
	delete startData;
}


void WONFactoryStartTitanServerAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* factories, unsigned int numFactories,
							WONIPAddress* startedOnFactory, WON_CONST char* configurationName,
							unsigned short* startedOnPorts, unsigned short* numStartedPorts,
							WON_CONST wchar_t* commandLineFragment, BOOL replaceCommandLine,
							unsigned char numPortsRequested, unsigned char numSpecificPorts,
							unsigned short* specificPortsArray, WON_CONST WONIPAddress* authorizedIPs,
							unsigned short numAuthorizedIPs, long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numFactories && factories && (!numAuthorizedIPs || authorizedIPs))
	{
		err = Error_OutOfMemory;
		IPSocket::Address* factorySrvAddrs = new IPSocket::Address[numFactories];
		if (factorySrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoFactorySrvAddrs(factorySrvAddrs);

			IPSocket::Address* authoritzedAddrs = new IPSocket::Address[numAuthorizedIPs ? numAuthorizedIPs : 1];
			if (authoritzedAddrs)	// allocates at least one, to make the logic easier
			{
				array_auto_ptr<IPSocket::Address> autoAuthoritzedAddrs(authoritzedAddrs);
				
				WONFactoryStartData* startData = new WONFactoryStartData;
				if (startData)
				{
					startData->hCompletion = hCompletion;
					startData->startedOnFactory = startedOnFactory;

					int i;
					for (i = 0; i < numFactories; i++)
						factorySrvAddrs[i].Set(factories[i]);
					if (numAuthorizedIPs)
						for (i = 0; i < numAuthorizedIPs; i++)
							authoritzedAddrs[i].Set(authorizedIPs[i]);

					StartTitanServerEx((Identity*)hWONAuth, factorySrvAddrs, numFactories,
						0, configurationName, startedOnPorts, numStartedPorts, commandLineFragment,
						replaceCommandLine ? true : false, numPortsRequested, numSpecificPorts,
						specificPortsArray, authoritzedAddrs, numAuthorizedIPs, timeout, true,
						WONFactoryStartCompletionTranslator, startData);
					return;
				}
			}
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONFactoryStartTitanServerAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* factories, unsigned int numFactories,
							WONIPAddress* startedOnFactory, WON_CONST char* configurationName,
							unsigned short* startedOnPorts, unsigned short* numStartedPorts,
							WON_CONST char* commandLineFragment, BOOL replaceCommandLine,
							unsigned char numPortsRequested, unsigned char numSpecificPorts,
							unsigned short* specificPortsArray, WON_CONST WONIPAddress* authorizedIPs,
							unsigned short numAuthorizedIPs, long timeout, HWONCOMPLETION hCompletion)
{
	WONFactoryStartTitanServerAsyncW(hWONAuth, factories, numFactories, startedOnFactory, configurationName,
							startedOnPorts, numStartedPorts,
							WONString(commandLineFragment).GetUnicodeCString(), replaceCommandLine, numPortsRequested,
							numSpecificPorts, specificPortsArray, authorizedIPs, numAuthorizedIPs, timeout,
							hCompletion);
}


WONError WONFactoryStopTitanServer(HWONAUTH hWONAuth, WON_CONST WONIPAddress* factory, WON_CONST char* configurationName,
						 unsigned short processPort, long timeout)
{
	Error err = Error_InvalidParams;
	if (factory)
	{
		IPSocket::Address addr(*factory);
		err = StopTitanServer((Identity*)hWONAuth, addr, configurationName, processPort, timeout);
	}
	return err;
}


static void WONFactoryStopTranslateCompletion(const StopTitanServerResult& result, HWONCOMPLETION hCompletion)
{
	WONComplete(hCompletion, (void*)(result.error));
}


void WONFactoryStopTitanServerAsync(HWONAUTH hWONAuth, WON_CONST WONIPAddress* factory, WON_CONST char* configurationName,
						  unsigned short processPort, long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (factory)
	{
		IPSocket::Address addr(*factory);
		StopTitanServerEx((Identity*)hWONAuth, addr, configurationName, processPort, timeout,
			true, WONFactoryStopTranslateCompletion, hCompletion);
		return;
	}
	WONComplete(hCompletion, (void*)err);
}


