#include "WONWS.h"

#ifdef WIN32

using namespace WONAPI;


Winsock::WSAFDIsSetProc Winsock::__WSAFDIsSetPatch = NULL;


int PASCAL FAR __WSAFDIsSetPatch(SOCKET sock, fd_set FAR * set)
{
	return Winsock::__WSAFDIsSetPatch(sock, set);
}


Winsock::Winsock()
	:	winsockVersion(0)
{
	ClearProcs();
	Retry();
}


Winsock::~Winsock()
{
	UnloadWinsock();
}


void Winsock::LoadWinsock()
{

#ifndef WINSOCK_DYNAMIC

	__WSAFDIsSetPatch = (WSAFDIsSetProc)::__WSAFDIsSet;

	/* Socket functions */
	accept = (LPFN_ACCEPT)::accept;
	bind = (LPFN_BIND)::bind;
	closesocket = (LPFN_CLOSESOCKET)::closesocket;
	connect = (LPFN_CONNECT)::connect;
	ioctlsocket = (LPFN_IOCTLSOCKET)::ioctlsocket;
	getpeername = (LPFN_GETPEERNAME)::getpeername;
	getsockname = (LPFN_GETSOCKNAME)::getsockname;
	getsockopt = (LPFN_GETSOCKOPT)::getsockopt;
	htonl = (LPFN_HTONL)::htonl;
	htons = (LPFN_HTONS)::htons;
	inet_addr = (LPFN_INET_ADDR)::inet_addr;
	inet_ntoa = (LPFN_INET_NTOA)::inet_ntoa;
	listen = (LPFN_LISTEN)::listen;
	ntohl = (LPFN_NTOHL)::ntohl;
	ntohs = (LPFN_NTOHS)::ntohs;
	recv = (LPFN_RECV)::recv;
	recvfrom = (LPFN_RECVFROM)::recvfrom;
	select = (LPFN_SELECT)::select;
	send = (LPFN_SEND)::send;
	sendto = (LPFN_SENDTO)::sendto;
	setsockopt = (LPFN_SETSOCKOPT)::setsockopt;
	shutdown = (LPFN_SHUTDOWN)::shutdown;
	socket = (LPFN_SOCKET)::socket;

	/* Database functions */
	gethostbyaddr = (LPFN_GETHOSTBYADDR)::gethostbyaddr;
	gethostbyname = (LPFN_GETHOSTBYNAME)::gethostbyname;
	gethostname = (LPFN_GETHOSTNAME)::gethostname;
	getservbyport = (LPFN_GETSERVBYPORT)::getservbyport;
	getservbyname = (LPFN_GETSERVBYNAME)::getservbyname;
	getprotobynumber = (LPFN_GETPROTOBYNUMBER)::getprotobynumber;
	getprotobyname = (LPFN_GETPROTOBYNAME)::getprotobyname;

	/* Microsoft Windows Extension functions */
//	WSAStartup = (LPFN_WSASTARTUP)::WSAStartup; // already got it
	WSACleanup = (LPFN_WSACLEANUP)::WSACleanup;
	WSASetLastError = (LPFN_WSASETLASTERROR)::WSASetLastError;
	WSAGetLastError = (LPFN_WSAGETLASTERROR)::WSAGetLastError;
	WSAIsBlocking = (LPFN_WSAISBLOCKING)::WSAIsBlocking;
	WSAUnhookBlockingHook = (LPFN_WSAUNHOOKBLOCKINGHOOK)::WSAUnhookBlockingHook;
	WSASetBlockingHook = (LPFN_WSASETBLOCKINGHOOK)::WSASetBlockingHook;
	WSACancelBlockingCall = (LPFN_WSACANCELBLOCKINGCALL)::WSACancelBlockingCall;
	WSAAsyncGetServByName = (LPFN_WSAASYNCGETSERVBYNAME)::WSAAsyncGetServByName;
	WSAAsyncGetServByPort = (LPFN_WSAASYNCGETSERVBYPORT)::WSAAsyncGetServByPort;
	WSAAsyncGetProtoByName = (LPFN_WSAASYNCGETPROTOBYNAME)::WSAAsyncGetProtoByName;
	WSAAsyncGetProtoByNumber = (LPFN_WSAASYNCGETPROTOBYNUMBER)::WSAAsyncGetProtoByNumber;
	WSAAsyncGetHostByName = (LPFN_WSAASYNCGETHOSTBYNAME)::WSAAsyncGetHostByName;
	WSAAsyncGetHostByAddr = (LPFN_WSAASYNCGETHOSTBYADDR)::WSAAsyncGetHostByAddr;
	WSACancelAsyncRequest = (LPFN_WSACANCELASYNCREQUEST)::WSACancelAsyncRequest;
	WSAAsyncSelect = (LPFN_WSAASYNCSELECT)::WSAAsyncSelect;

#else
	__WSAFDIsSetPatch = (WSAFDIsSetProc)GetProcAddress(libHandle, "__WSAFDIsSet");

	/* Socket functions */
	accept = (LPFN_ACCEPT)GetProcAddress(libHandle, "accept");
	bind = (LPFN_BIND)GetProcAddress(libHandle, "bind");
	closesocket = (LPFN_CLOSESOCKET)GetProcAddress(libHandle, "closesocket");
	connect = (LPFN_CONNECT)GetProcAddress(libHandle, "connect");
	ioctlsocket = (LPFN_IOCTLSOCKET)GetProcAddress(libHandle, "ioctlsocket");
	getpeername = (LPFN_GETPEERNAME)GetProcAddress(libHandle, "getpeername");
	getsockname = (LPFN_GETSOCKNAME)GetProcAddress(libHandle, "getsockname");
	getsockopt = (LPFN_GETSOCKOPT)GetProcAddress(libHandle, "getsockopt");
	htonl = (LPFN_HTONL)GetProcAddress(libHandle, "htonl");
	htons = (LPFN_HTONS)GetProcAddress(libHandle, "htons");
	inet_addr = (LPFN_INET_ADDR)GetProcAddress(libHandle, "inet_addr");
	inet_ntoa = (LPFN_INET_NTOA)GetProcAddress(libHandle, "inet_ntoa");
	listen = (LPFN_LISTEN)GetProcAddress(libHandle, "listen");
	ntohl = (LPFN_NTOHL)GetProcAddress(libHandle, "ntohl");
	ntohs = (LPFN_NTOHS)GetProcAddress(libHandle, "ntohs");
	recv = (LPFN_RECV)GetProcAddress(libHandle, "recv");
	recvfrom = (LPFN_RECVFROM)GetProcAddress(libHandle, "recvfrom");
	select = (LPFN_SELECT)GetProcAddress(libHandle, "select");
	send = (LPFN_SEND)GetProcAddress(libHandle, "send");
	sendto = (LPFN_SENDTO)GetProcAddress(libHandle, "sendto");
	setsockopt = (LPFN_SETSOCKOPT)GetProcAddress(libHandle, "setsockopt");
	shutdown = (LPFN_SHUTDOWN)GetProcAddress(libHandle, "shutdown");
	socket = (LPFN_SOCKET)GetProcAddress(libHandle, "socket");

	/* Database functions */
	gethostbyaddr = (LPFN_GETHOSTBYADDR)GetProcAddress(libHandle, "gethostbyaddr");
	gethostbyname = (LPFN_GETHOSTBYNAME)GetProcAddress(libHandle, "gethostbyname");
	gethostname = (LPFN_GETHOSTNAME)GetProcAddress(libHandle, "gethostname");
	getservbyport = (LPFN_GETSERVBYPORT)GetProcAddress(libHandle, "getservbyport");
	getservbyname = (LPFN_GETSERVBYNAME)GetProcAddress(libHandle, "getservbyname");
	getprotobynumber = (LPFN_GETPROTOBYNUMBER)GetProcAddress(libHandle, "getprotobynumber");
	getprotobyname = (LPFN_GETPROTOBYNAME)GetProcAddress(libHandle, "getprotobyname");

	/* Microsoft Windows Extension functions */
//	WSAStartup = (LPFN_WSASTARTUP)GetProcAddress(libHandle, "WSAStartup"); // already got it
	WSACleanup = (LPFN_WSACLEANUP)GetProcAddress(libHandle, "WSACleanup");
	WSASetLastError = (LPFN_WSASETLASTERROR)GetProcAddress(libHandle, "WSASetLastError");
	WSAGetLastError = (LPFN_WSAGETLASTERROR)GetProcAddress(libHandle, "WSAGetLastError");
	WSAIsBlocking = (LPFN_WSAISBLOCKING)GetProcAddress(libHandle, "WSAIsBlocking");
	WSAUnhookBlockingHook = (LPFN_WSAUNHOOKBLOCKINGHOOK)GetProcAddress(libHandle, "WSAUnhookBlockingHook");
	WSASetBlockingHook = (LPFN_WSASETBLOCKINGHOOK)GetProcAddress(libHandle, "WSASetBlockingHook");
	WSACancelBlockingCall = (LPFN_WSACANCELBLOCKINGCALL)GetProcAddress(libHandle, "WSACancelBlockingCall");
	WSAAsyncGetServByName = (LPFN_WSAASYNCGETSERVBYNAME)GetProcAddress(libHandle, "WSAAsyncGetServByName");
	WSAAsyncGetServByPort = (LPFN_WSAASYNCGETSERVBYPORT)GetProcAddress(libHandle, "WSAAsyncGetServByPort");
	WSAAsyncGetProtoByName = (LPFN_WSAASYNCGETPROTOBYNAME)GetProcAddress(libHandle, "WSAAsyncGetProtoByName");
	WSAAsyncGetProtoByNumber = (LPFN_WSAASYNCGETPROTOBYNUMBER)GetProcAddress(libHandle, "WSAAsyncGetProtoByNumber");
	WSAAsyncGetHostByName = (LPFN_WSAASYNCGETHOSTBYNAME)GetProcAddress(libHandle, "WSAAsyncGetHostByName");
	WSAAsyncGetHostByAddr = (LPFN_WSAASYNCGETHOSTBYADDR)GetProcAddress(libHandle, "WSAAsyncGetHostByAddr");
	WSACancelAsyncRequest = (LPFN_WSACANCELASYNCREQUEST)GetProcAddress(libHandle, "WSACancelAsyncRequest");
	WSAAsyncSelect = (LPFN_WSAASYNCSELECT)GetProcAddress(libHandle, "WSAAsyncSelect");
#endif
}


void Winsock::LoadWinsock2()
{
#ifndef WINSOCK_DYNAMIC

	// Winsock 2 extensions 
/*	WSAAccept = (LPFN_WSAACCEPT)::WSAAccept;
	WSACloseEvent = (LPFN_WSACLOSEEVENT)::WSACloseEvent;
	WSAConnect = (LPFN_WSACONNECT)::WSAConnect;
	WSACreateEvent = (LPFN_WSACREATEEVENT)::WSACreateEvent;
	WSADuplicateSocketA = (LPFN_WSADUPLICATESOCKETA)::WSADuplicateSocketA;
	WSADuplicateSocketW = (LPFN_WSADUPLICATESOCKETW)::WSADuplicateSocketW;
	WSAEnumNetworkEvents = (LPFN_WSAENUMNETWORKEVENTS)::WSAEnumNetworkEvents;
	WSAEnumProtocolsA = (LPFN_WSAENUMPROTOCOLSA)::WSAEnumProtocolsA;
	WSAEnumProtocolsW = (LPFN_WSAENUMPROTOCOLSW)::WSAEnumProtocolsW;
	WSAEventSelect = (LPFN_WSAEVENTSELECT)::WSAEventSelect;
	WSAGetOverlappedResult = (LPFN_WSAGETOVERLAPPEDRESULT)::WSAEventSelect;
	WSAGetQOSByName = (LPFN_WSAGETQOSBYNAME)::WSAGetQOSByName;
	WSAHtonl = (LPFN_WSAHTONL)::WSAHtonl;
	WSAHtons = (LPFN_WSAHTONS)::WSAHtons;
	WSAIoctl = (LPFN_WSAIOCTL)::WSAIoctl;
	WSAJoinLeaf = (LPFN_WSAJOINLEAF)::WSAJoinLeaf;
	WSANtohl = (LPFN_WSANTOHL)::WSANtohl;
	WSANtohs = (LPFN_WSANTOHS)::WSANtohs;
	WSARecv = (LPFN_WSARECV)::WSARecv;
	WSARecvDisconnect = (LPFN_WSARECVDISCONNECT)::WSARecvDisconnect;
	WSARecvFrom = (LPFN_WSARECVFROM)::WSARecvFrom;
	WSAResetEvent = (LPFN_WSARESETEVENT)::WSAResetEvent;
	WSASend = (LPFN_WSASEND)::WSASend;
	WSASendDisconnect = (LPFN_WSASENDDISCONNECT)::WSASendDisconnect;
	WSASendTo = (LPFN_WSASENDTO)::WSASendTo;
	WSASetEvent = (LPFN_WSASETEVENT)::WSASetEvent;
	WSASocketA = (LPFN_WSASOCKETA)::WSASocketA;
	WSASocketW = (LPFN_WSASOCKETW)::WSASocketW;
	WSAWaitForMultipleEvents = (LPFN_WSAWAITFORMULTIPLEEVENTS)::WSAWaitForMultipleEvents;
	WSAAddressToStringA = (LPFN_WSAADDRESSTOSTRINGA)::WSAAddressToStringA;
	WSAAddressToStringW = (LPFN_WSAADDRESSTOSTRINGW)::WSAAddressToStringW;
	WSAStringToAddressA = (LPFN_WSASTRINGTOADDRESSA)::WSAStringToAddressA;
	WSAStringToAddressW = (LPFN_WSASTRINGTOADDRESSW)::WSAStringToAddressW;
	WSALookupServiceBeginA = (LPFN_WSALOOKUPSERVICEBEGINA)::WSALookupServiceBeginA;
	WSALookupServiceBeginW = (LPFN_WSALOOKUPSERVICEBEGINW)::WSALookupServiceBeginW;
	WSALookupServiceNextA = (LPFN_WSALOOKUPSERVICENEXTA)::WSALookupServiceNextA;
	WSALookupServiceNextW = (LPFN_WSALOOKUPSERVICENEXTW)::WSALookupServiceNextW;
	WSALookupServiceEnd = (LPFN_WSALOOKUPSERVICEEND)::WSALookupServiceEnd;
	WSAInstallServiceClassA = (LPFN_WSAINSTALLSERVICECLASSA)::WSAInstallServiceClassA;
	WSAInstallServiceClassW = (LPFN_WSAINSTALLSERVICECLASSW)::WSAInstallServiceClassW;
	WSARemoveServiceClass = (LPFN_WSAREMOVESERVICECLASS)::WSARemoveServiceClass;
	WSAGetServiceClassInfoA = (LPFN_WSAGETSERVICECLASSINFOA)::WSAGetServiceClassInfoA;
	WSAGetServiceClassInfoW = (LPFN_WSAGETSERVICECLASSINFOW)::WSAGetServiceClassInfoW;
	WSAEnumNameSpaceProvidersA = (LPFN_WSAENUMNAMESPACEPROVIDERSA)::WSAEnumNameSpaceProvidersA;
	WSAEnumNameSpaceProvidersW = (LPFN_WSAENUMNAMESPACEPROVIDERSW)::WSAEnumNameSpaceProvidersW;
	WSAGetServiceClassNameByClassIdA = (LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDA)::WSAGetServiceClassNameByClassIdA;
	WSAGetServiceClassNameByClassIdW = (LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDW)::WSAGetServiceClassNameByClassIdW;
	WSASetServiceA = (LPFN_WSASETSERVICEA)::WSASetServiceA;
	WSASetServiceW = (LPFN_WSASETSERVICEW)::WSASetServiceW;*/

#else

	/* Winsock 2 extensions */
	WSAAccept = (LPFN_WSAACCEPT)GetProcAddress(libHandle, "WSAAccept");
	WSACloseEvent = (LPFN_WSACLOSEEVENT)GetProcAddress(libHandle, "WSACloseEvent");
	WSAConnect = (LPFN_WSACONNECT)GetProcAddress(libHandle, "WSAConnect");
	WSACreateEvent = (LPFN_WSACREATEEVENT)GetProcAddress(libHandle, "WSACreateEvent");
	WSADuplicateSocketA = (LPFN_WSADUPLICATESOCKETA)GetProcAddress(libHandle, "WSADuplicateSocketA");
	WSADuplicateSocketW = (LPFN_WSADUPLICATESOCKETW)GetProcAddress(libHandle, "WSADuplicateSocketW");
	WSAEnumNetworkEvents = (LPFN_WSAENUMNETWORKEVENTS)GetProcAddress(libHandle, "WSAEnumNetworkEvents");
	WSAEnumProtocolsA = (LPFN_WSAENUMPROTOCOLSA)GetProcAddress(libHandle, "WSAEnumProtocolsA");
	WSAEnumProtocolsW = (LPFN_WSAENUMPROTOCOLSW)GetProcAddress(libHandle, "WSAEnumProtocolsW");
	WSAEventSelect = (LPFN_WSAEVENTSELECT)GetProcAddress(libHandle, "WSAEventSelect");
	WSAGetOverlappedResult = (LPFN_WSAGETOVERLAPPEDRESULT)GetProcAddress(libHandle, "WSAEventSelect");
	WSAGetQOSByName = (LPFN_WSAGETQOSBYNAME)GetProcAddress(libHandle, "WSAGetQOSByName");
	WSAHtonl = (LPFN_WSAHTONL)GetProcAddress(libHandle, "WSAHtonl");
	WSAHtons = (LPFN_WSAHTONS)GetProcAddress(libHandle, "WSAHtons");
	WSAIoctl = (LPFN_WSAIOCTL)GetProcAddress(libHandle, "WSAIoctl");
	WSAJoinLeaf = (LPFN_WSAJOINLEAF)GetProcAddress(libHandle, "WSAJoinLeaf");
	WSANtohl = (LPFN_WSANTOHL)GetProcAddress(libHandle, "WSANtohl");
	WSANtohs = (LPFN_WSANTOHS)GetProcAddress(libHandle, "WSANtohs");
	WSARecv = (LPFN_WSARECV)GetProcAddress(libHandle, "WSARecv");
	WSARecvDisconnect = (LPFN_WSARECVDISCONNECT)GetProcAddress(libHandle, "WSARecvDisconnect");
	WSARecvFrom = (LPFN_WSARECVFROM)GetProcAddress(libHandle, "WSARecvFrom");
	WSAResetEvent = (LPFN_WSARESETEVENT)GetProcAddress(libHandle, "WSAResetEvent");
	WSASend = (LPFN_WSASEND)GetProcAddress(libHandle, "WSASend");
	WSASendDisconnect = (LPFN_WSASENDDISCONNECT)GetProcAddress(libHandle, "WSASendDisconnect");
	WSASendTo = (LPFN_WSASENDTO)GetProcAddress(libHandle, "WSASendTo");
	WSASetEvent = (LPFN_WSASETEVENT)GetProcAddress(libHandle, "WSASetEvent");
	WSASocketA = (LPFN_WSASOCKETA)GetProcAddress(libHandle, "WSASocketA");
	WSASocketW = (LPFN_WSASOCKETW)GetProcAddress(libHandle, "WSASocketW");
	WSAWaitForMultipleEvents = (LPFN_WSAWAITFORMULTIPLEEVENTS)GetProcAddress(libHandle, "WSAWaitForMultipleEvents");
	WSAAddressToStringA = (LPFN_WSAADDRESSTOSTRINGA)GetProcAddress(libHandle, "WSAAddressToStringA");
	WSAAddressToStringW = (LPFN_WSAADDRESSTOSTRINGW)GetProcAddress(libHandle, "WSAAddressToStringW");
	WSAStringToAddressA = (LPFN_WSASTRINGTOADDRESSA)GetProcAddress(libHandle, "WSAStringToAddressA");
	WSAStringToAddressW = (LPFN_WSASTRINGTOADDRESSW)GetProcAddress(libHandle, "WSAStringToAddressW");
	WSALookupServiceBeginA = (LPFN_WSALOOKUPSERVICEBEGINA)GetProcAddress(libHandle, "WSALookupServiceBeginA");
	WSALookupServiceBeginW = (LPFN_WSALOOKUPSERVICEBEGINW)GetProcAddress(libHandle, "WSALookupServiceBeginW");
	WSALookupServiceNextA = (LPFN_WSALOOKUPSERVICENEXTA)GetProcAddress(libHandle, "WSALookupServiceNextA");
	WSALookupServiceNextW = (LPFN_WSALOOKUPSERVICENEXTW)GetProcAddress(libHandle, "WSALookupServiceNextW");
	WSALookupServiceEnd = (LPFN_WSALOOKUPSERVICEEND)GetProcAddress(libHandle, "WSALookupServiceEnd");
	WSAInstallServiceClassA = (LPFN_WSAINSTALLSERVICECLASSA)GetProcAddress(libHandle, "WSAInstallServiceClassA");
	WSAInstallServiceClassW = (LPFN_WSAINSTALLSERVICECLASSW)GetProcAddress(libHandle, "WSAInstallServiceClassW");
	WSARemoveServiceClass = (LPFN_WSAREMOVESERVICECLASS)GetProcAddress(libHandle, "WSARemoveServiceClass");
	WSAGetServiceClassInfoA = (LPFN_WSAGETSERVICECLASSINFOA)GetProcAddress(libHandle, "WSAGetServiceClassInfoA");
	WSAGetServiceClassInfoW = (LPFN_WSAGETSERVICECLASSINFOW)GetProcAddress(libHandle, "WSAGetServiceClassInfoW");
	WSAEnumNameSpaceProvidersA = (LPFN_WSAENUMNAMESPACEPROVIDERSA)GetProcAddress(libHandle, "WSAEnumNameSpaceProvidersA");
	WSAEnumNameSpaceProvidersW = (LPFN_WSAENUMNAMESPACEPROVIDERSW)GetProcAddress(libHandle, "WSAEnumNameSpaceProvidersW");
	WSAGetServiceClassNameByClassIdA = (LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDA)GetProcAddress(libHandle, "WSAGetServiceClassNameByClassIdA");
	WSAGetServiceClassNameByClassIdW = (LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDW)GetProcAddress(libHandle, "WSAGetServiceClassNameByClassIdW");
	WSASetServiceA = (LPFN_WSASETSERVICEA)GetProcAddress(libHandle, "WSASetServiceA");
	WSASetServiceW = (LPFN_WSASETSERVICEW)GetProcAddress(libHandle, "WSASetServiceW");
#endif
}


void Winsock::UnloadWinsock()
{
	if (winsockVersion != 0)
	{
		WSACleanup();

		ClearProcs();

#ifdef WINSOCK_DYNAMIC
		FreeLibrary(libHandle);
#endif

		winsockVersion = 0;
	}
}

void Winsock::ClearProcs()
{
	// NULLs should cause access faults (?) if called

	/* socket functions */
	accept		= NULL;
	bind		= NULL;
	closesocket	= NULL;
	connect		= NULL;
	ioctlsocket	= NULL;
	getpeername	= NULL;
	getsockname	= NULL;
	getsockopt	= NULL;
	htonl		= NULL;
	htons		= NULL;
	inet_addr	= NULL;
	inet_ntoa	= NULL;
	listen		= NULL;
	ntohl		= NULL;
	ntohs		= NULL;
	recv		= NULL;
	recvfrom	= NULL;
	select		= NULL;
	send		= NULL;
	sendto		= NULL;
	setsockopt	= NULL;
	shutdown	= NULL;
	socket		= NULL;

	/* Database functions */
	gethostbyaddr	= NULL;
	gethostbyname	= NULL;
	gethostname		= NULL;
	getservbyport	= NULL;
	getservbyname	= NULL;
	getprotobynumber= NULL;
	getprotobyname	= NULL;

	/* Microsoft Windows Extension functions */
	WSAStartup				= NULL;
	WSACleanup				= NULL;
	WSASetLastError			= NULL;
	WSAGetLastError			= NULL;
	WSAIsBlocking			= NULL;
	WSAUnhookBlockingHook	= NULL;
	WSASetBlockingHook		= NULL;
	WSACancelBlockingCall	= NULL;
	WSAAsyncGetServByName	= NULL;
	WSAAsyncGetServByPort	= NULL;
	WSAAsyncGetProtoByName	= NULL;
	WSAAsyncGetProtoByNumber= NULL;
	WSAAsyncGetHostByName	= NULL;
	WSAAsyncGetHostByAddr	= NULL;
	WSACancelAsyncRequest	= NULL;
	WSAAsyncSelect			= NULL;

	/* Winsock 2 extensions */
	WSAAccept				= NULL;
	WSACloseEvent			= NULL;
	WSAConnect				= NULL;
	WSACreateEvent			= NULL;
	WSADuplicateSocketA		= NULL;
	WSADuplicateSocketW		= NULL;
	WSAEnumNetworkEvents	= NULL;
	WSAEnumProtocolsA		= NULL;
	WSAEnumProtocolsW		= NULL;
	WSAEventSelect			= NULL;
	WSAGetOverlappedResult	= NULL;
	WSAGetQOSByName			= NULL;
	WSAHtonl				= NULL;
	WSAHtons				= NULL;
	WSAIoctl				= NULL;
	WSAJoinLeaf				= NULL;
	WSANtohl				= NULL;
	WSANtohs				= NULL;
	WSARecv					= NULL;
	WSARecvDisconnect		= NULL;
	WSARecvFrom				= NULL;
	WSAResetEvent			= NULL;
	WSASend					= NULL;
	WSASendDisconnect		= NULL;
	WSASendTo				= NULL;
	WSASetEvent				= NULL;
	WSASocketA				= NULL;
	WSASocketW				= NULL;
	WSAWaitForMultipleEvents	= NULL;
	WSAAddressToStringA			= NULL;
	WSAAddressToStringW			= NULL;
	WSAStringToAddressA			= NULL;
	WSAStringToAddressW			= NULL;
	WSALookupServiceBeginA		= NULL;
	WSALookupServiceBeginW		= NULL;
	WSALookupServiceNextA		= NULL;
	WSALookupServiceNextW		= NULL;
	WSALookupServiceEnd			= NULL;
	WSAInstallServiceClassA		= NULL;
	WSAInstallServiceClassW		= NULL;
	WSARemoveServiceClass		= NULL;
	WSAGetServiceClassInfoA		= NULL;
	WSAGetServiceClassInfoW		= NULL;
	WSAEnumNameSpaceProvidersA	= NULL;
	WSAEnumNameSpaceProvidersW	= NULL;
	WSAGetServiceClassNameByClassIdA= NULL;
	WSAGetServiceClassNameByClassIdW= NULL;
	WSASetServiceA					= NULL;
	WSASetServiceW					= NULL;
}


DWORD Winsock::Retry()
{
	UnloadWinsock();

#ifdef WINSOCK_DYNAMIC

	libHandle = LoadLibrary("ws2_32.dll");
	if (!libHandle)
		libHandle = LoadLibrary("wsock32.dll");

	if (libHandle)
	{
		WSAStartup = (LPFN_WSASTARTUP)GetProcAddress(libHandle, "WSAStartup");
#else
		WSAStartup = (LPFN_WSASTARTUP)::WSAStartup;
#endif
		
		if (WSAStartup != 0)
		{
			WSADATA wsaData;
			
			// For now; request winsock 2, if that doesn't work, request 1.1, or fail
			
			if ((WSAStartup(0x0002, &wsaData) == 0) && (wsaData.wVersion == 0x0002))
			{
				LoadWinsock();
				LoadWinsock2();
			}
			else if ((WSAStartup(0x0101, &wsaData) != 0) || (wsaData.wVersion == 0x0101))
			{
				LoadWinsock();
			}
			else
			{
				WSAStartup = 0;
				wsaData.wVersion = 0;
			}
			winsockVersion = wsaData.wVersion;
		}
#ifdef WINSOCK_DYNAMIC
	}
#endif
	return winsockVersion;
}


#endif
