/********************************************************************************

  WON::Winsock

	This Winsock class dynamically loads the winsock dll.  Without this object,
	programs that wish to take advantage of Winsock 2 would have to be linked against
	the Winsock 2 library, and would would fail to launch on machines without it.

	12/30/98	-	Colen Garoutte-Carson	- Initial functionality

********************************************************************************/

#ifndef __WON_WINSOCK_H__
#define __WON_WINSOCK_H__

#ifdef WIN32

#ifdef _WINSOCKAPI_
#error WONWS.h (or header including WONWS.h) must be included before winsock2.h or winsock.h !
#endif

#define __WSAFDIsSet __WSAFDIsSetPatch
#define INCL_WINSOCK_API_TYPEDEFS 1
#include <winsock2.h>


namespace WONAPI {


class Winsock
{
private:
	DWORD winsockVersion;
	HINSTANCE libHandle;

	void LoadWinsock();
	void LoadWinsock2();
	void UnloadWinsock();
	void ClearProcs();
public:
	// Winsock API routines :

	typedef int (PASCAL FAR* WSAFDIsSetProc)(SOCKET, fd_set FAR *);
	static WSAFDIsSetProc __WSAFDIsSetPatch;

	
	/* Socket functions */
	LPFN_ACCEPT								accept;
	LPFN_BIND								bind;
	LPFN_CLOSESOCKET						closesocket;
	LPFN_CONNECT							connect;
	LPFN_IOCTLSOCKET						ioctlsocket;
	LPFN_GETPEERNAME						getpeername;
	LPFN_GETSOCKNAME						getsockname;
	LPFN_GETSOCKOPT							getsockopt;
	LPFN_HTONL								htonl;
	LPFN_HTONS								htons;
	LPFN_INET_ADDR							inet_addr;
	LPFN_INET_NTOA							inet_ntoa;
	LPFN_LISTEN								listen;
	LPFN_NTOHL								ntohl;
	LPFN_NTOHS								ntohs;
	LPFN_RECV								recv;
	LPFN_RECVFROM							recvfrom;
	LPFN_SELECT								select;
	LPFN_SEND								send;
	LPFN_SENDTO								sendto;
	LPFN_SETSOCKOPT							setsockopt;
	LPFN_SHUTDOWN							shutdown;
	LPFN_SOCKET								socket;

	/* Database functions */
	LPFN_GETHOSTBYADDR						gethostbyaddr;
	LPFN_GETHOSTBYNAME						gethostbyname;
	LPFN_GETHOSTNAME						gethostname;
	LPFN_GETSERVBYPORT						getservbyport;
	LPFN_GETSERVBYNAME						getservbyname;
	LPFN_GETPROTOBYNUMBER					getprotobynumber;
	LPFN_GETPROTOBYNAME						getprotobyname;

	/* Microsoft Windows Extension functions */
	LPFN_WSASTARTUP							WSAStartup;
	LPFN_WSACLEANUP							WSACleanup;
	LPFN_WSASETLASTERROR					WSASetLastError;
	LPFN_WSAGETLASTERROR					WSAGetLastError;
	LPFN_WSAISBLOCKING						WSAIsBlocking;
	LPFN_WSAUNHOOKBLOCKINGHOOK				WSAUnhookBlockingHook;
	LPFN_WSASETBLOCKINGHOOK					WSASetBlockingHook;
	LPFN_WSACANCELBLOCKINGCALL				WSACancelBlockingCall;
	LPFN_WSAASYNCGETSERVBYNAME				WSAAsyncGetServByName;
	LPFN_WSAASYNCGETSERVBYPORT				WSAAsyncGetServByPort;
	LPFN_WSAASYNCGETPROTOBYNAME				WSAAsyncGetProtoByName;
	LPFN_WSAASYNCGETPROTOBYNUMBER			WSAAsyncGetProtoByNumber;
	LPFN_WSAASYNCGETHOSTBYNAME				WSAAsyncGetHostByName;
	LPFN_WSAASYNCGETHOSTBYADDR				WSAAsyncGetHostByAddr;
	LPFN_WSACANCELASYNCREQUEST				WSACancelAsyncRequest;
	LPFN_WSAASYNCSELECT						WSAAsyncSelect;

	/* Winsock 2 extensions */
	LPFN_WSAACCEPT							WSAAccept;
	LPFN_WSACLOSEEVENT						WSACloseEvent;
	LPFN_WSACONNECT							WSAConnect;
	LPFN_WSACREATEEVENT						WSACreateEvent;
	LPFN_WSADUPLICATESOCKETA				WSADuplicateSocketA;
	LPFN_WSADUPLICATESOCKETW				WSADuplicateSocketW;
	LPFN_WSAENUMNETWORKEVENTS				WSAEnumNetworkEvents;
	LPFN_WSAENUMPROTOCOLSA					WSAEnumProtocolsA;
	LPFN_WSAENUMPROTOCOLSW					WSAEnumProtocolsW;
	LPFN_WSAEVENTSELECT						WSAEventSelect;
	LPFN_WSAGETOVERLAPPEDRESULT				WSAGetOverlappedResult;
	LPFN_WSAGETQOSBYNAME					WSAGetQOSByName;
	LPFN_WSAHTONL							WSAHtonl;
	LPFN_WSAHTONS							WSAHtons;
	LPFN_WSAIOCTL							WSAIoctl;
	LPFN_WSAJOINLEAF						WSAJoinLeaf;
	LPFN_WSANTOHL							WSANtohl;
	LPFN_WSANTOHS							WSANtohs;
	LPFN_WSARECV							WSARecv;
	LPFN_WSARECVDISCONNECT					WSARecvDisconnect;
	LPFN_WSARECVFROM						WSARecvFrom;
	LPFN_WSARESETEVENT						WSAResetEvent;
	LPFN_WSASEND							WSASend;
	LPFN_WSASENDDISCONNECT					WSASendDisconnect;
	LPFN_WSASENDTO							WSASendTo;
	LPFN_WSASETEVENT						WSASetEvent;
	LPFN_WSASOCKETA							WSASocketA;
	LPFN_WSASOCKETW							WSASocketW;
	LPFN_WSAWAITFORMULTIPLEEVENTS			WSAWaitForMultipleEvents;
	LPFN_WSAADDRESSTOSTRINGA				WSAAddressToStringA;
	LPFN_WSAADDRESSTOSTRINGW				WSAAddressToStringW;
	LPFN_WSASTRINGTOADDRESSA				WSAStringToAddressA;
	LPFN_WSASTRINGTOADDRESSW				WSAStringToAddressW;
	LPFN_WSALOOKUPSERVICEBEGINA				WSALookupServiceBeginA;
	LPFN_WSALOOKUPSERVICEBEGINW				WSALookupServiceBeginW;
	LPFN_WSALOOKUPSERVICENEXTA				WSALookupServiceNextA;
	LPFN_WSALOOKUPSERVICENEXTW				WSALookupServiceNextW;
	LPFN_WSALOOKUPSERVICEEND				WSALookupServiceEnd;
	LPFN_WSAINSTALLSERVICECLASSA			WSAInstallServiceClassA;
	LPFN_WSAINSTALLSERVICECLASSW			WSAInstallServiceClassW;
	LPFN_WSAREMOVESERVICECLASS				WSARemoveServiceClass;
	LPFN_WSAGETSERVICECLASSINFOA			WSAGetServiceClassInfoA;
	LPFN_WSAGETSERVICECLASSINFOW			WSAGetServiceClassInfoW;
	LPFN_WSAENUMNAMESPACEPROVIDERSA			WSAEnumNameSpaceProvidersA;
	LPFN_WSAENUMNAMESPACEPROVIDERSW			WSAEnumNameSpaceProvidersW;
	LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDA	WSAGetServiceClassNameByClassIdA;
	LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDW	WSAGetServiceClassNameByClassIdW;
	LPFN_WSASETSERVICEA						WSASetServiceA;
	LPFN_WSASETSERVICEW						WSASetServiceW;

	Winsock();
	~Winsock();

	DWORD GetVersion()	{ return winsockVersion; }	// Returns 0 if not able to load
	DWORD Retry();
};

};

#endif

#endif
