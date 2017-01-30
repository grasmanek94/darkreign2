////////////////////////////////////////////////////////////////////////////////
//
// Socket
//
// Copyright 1999-2000
// Matthew Versluys
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32_socket.h"
#include "win32_guid.h"
#include "win32_dns.h"
#include "logging.h"


////////////////////////////////////////////////////////////////////////////////
//
// Libraries
//

//#pragma comment(lib, "ws2_32.lib")




////////////////////////////////////////////////////////////////////////////////
//
// NameSpace Win32
//
namespace Win32
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Winsock2 functions
  //
  // To avoid staticly binding to ws2_32.lib
  //

  typedef int (STDCALL *WSAConnectFunc)(SOCKET s, const struct sockaddr FAR *name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS);
  typedef int (STDCALL *WSAEnumNetworkEventsFunc)(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents);
  typedef int (STDCALL *WSAEnumProtocolsFunc)(LPINT lpiProtocols, LPWSAPROTOCOL_INFO lpProtocolBuffer, LPDWORD lpdwBufferLength);
  typedef int (STDCALL *WSAEventSelectFunc)(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents);
  typedef int (STDCALL *WSARecvFunc)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE);
  typedef int (STDCALL *WSARecvFromFunc)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, struct sockaddr FAR *lpFrom, LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE);
  typedef int (STDCALL *WSASendFunc)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
  typedef int (STDCALL *WSASendToFunc)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const struct sockaddr FAR *lpTo, int iToLen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
  typedef SOCKET (STDCALL *WSASocketFunc)(int af, int type, int protocol, LPWSAPROTOCOL_INFO lpProtocolInfo, GROUP g, DWORD dwFlags);

  static WSAConnectFunc WSAConnect;
  static WSAEnumNetworkEventsFunc WSAEnumNetworkEvents;
  static WSAEnumProtocolsFunc WSAEnumProtocols;
  static WSAEventSelectFunc WSAEventSelect;
  static WSARecvFunc WSARecv;
  static WSARecvFromFunc WSARecvFrom;
  static WSASendFunc WSASend;
  static WSASendToFunc WSASendTo;
  static WSASocketFunc WSASocket;


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  static HINSTANCE dll;
  static Bool initialized = FALSE;
  const U32 MinRateTime = 1000;
  const U32 MinRateSize = 1000;


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Socket::Statistics::Flow
  //

  //
  // Constructor
  //
  Socket::Statistics::Flow::Flow()
  {
    Reset();
  }


  //
  // Socket was opened
  //
  void Socket::Statistics::Flow::Reset()
  {
    packets = 0;
    bytes = 0;
    rate = 0.0f;
    smoothRate = 0.0f;
    maxRate = F32_MIN;
    minRate = F32_MAX;
    markSize = 0;
    markTime = Clock::Time::Ms();
  }


  //
  // Add a packet
  //
  Socket::Statistics::Flow::AddPacket(U32 size)
  {
    // Increment packets
    packets++;

    // Add bytes
    bytes += size;

    // Since we last marked it, has there been a minimum amount 
    // of time and size elapsed since the last rate calculation
    U32 time = Clock::Time::Ms();
    S32 diff = time - markTime;

    if 
    (
      diff > MinRateTime ||
      markSize > MinRateSize
    )  
    {
      // Recompute the rate
      rate = F32(markSize) / F32(diff);
      smoothRate = smoothRate * 0.9f + rate * 0.1f;
      maxRate = Max(maxRate, rate);
      minRate = Min(minRate, rate);

      // Update markTime/markSize
      markTime = time;
      markSize = 0;
    }

    // Add the size of this packet to the marked size
    markSize += size;
  }


  //
  // Write a report to the log file for this flow
  //
  void Socket::Statistics::Flow::Report(U32 time) const
  {
    LDIAG("Time            : " << time)
    LDIAG("Packets         : " << packets)
    LDIAG("Avg Packets/Sec : " << (time ? F32(packets) * 1000.0f / F32(time) : 0.0f))
    LDIAG("Bytes           : " << bytes)
    LDIAG("Avg Bytes/Sec   : " << (time ? F32(bytes)  * 1000.0f / F32(time) : 0.0f))
    LDIAG("Bytes/Packet    : " << (packets ? F32(bytes) / F32(packets) : 0.0f))
    LDIAG("Rate            : " << rate * 1000.0f)
    LDIAG("Smooth Rate     : " << smoothRate * 1000.0f)
    LDIAG("Max Rate        : " << maxRate * 1000.0f)
    LDIAG("Min Rate        : " << minRate * 1000.0f)
  }


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Socket::Statistics
  //


  //
  // Socket was opened
  //
  void Socket::Statistics::Opened()
  {
    // Save the time the socket was opened
    timeOpened = Clock::Time::Ms();

    // Tell flows that the socket is open
    recv.Reset();
    sent.Reset();
  }


  //
  // Packet was received
  //
  void Socket::Statistics::Recv(U32 size)
  {
    recv.AddPacket(size);
  }


  //
  // Packet was sent
  //
  void Socket::Statistics::Sent(U32 size)
  {
    sent.AddPacket(size);
  }


  //
  // Write a report to the log file for this socket
  //
  void Socket::Statistics::Report() const
  {
    LDIAG("Socket Statistics")
    U32 time = Clock::Time::Ms() - timeOpened;
    LDIAG("Received")
    recv.Report(time);
    LDIAG("Sent")
    sent.Report(time);
  }


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Socket::Address
  //


  //
  // Socket::Address::Address
  //
  // Construct an empty address
  //
  Socket::Address::Address()
  {
    sin_family = AF_INET;
  }


  //
  // Socket::Address::Address
  //
  // Construct from a text addres port combo
  //
  Socket::Address::Address(const char *ip, U16 port)
  {
    sin_family = AF_INET;
    SetIP(ip);
    SetPort(port);
  }


  //
  // Socket::Address::Address
  //
  // Construct from an IP port combo
  //
  Socket::Address::Address(U32 ip, U16 port)
  {
    sin_family = AF_INET;
    SetIP(ip);
    SetPort(port);
  }


  //
  // Socket::Address::Address
  //
  // Copy from another address
  //
  Socket::Address::Address(const Address &address)
  {
    sin_family = address.sin_family;
    sin_addr = address.sin_addr;
    sin_port = address.sin_port;
  }


  //
  // Socket::Address::Address
  //
  // Copy from a sockaddr_in structure
  //
  Socket::Address::Address(const sockaddr_in &address)
  {
    sin_family = address.sin_family;
    sin_addr = address.sin_addr;
    sin_port = address.sin_port;
  }


  //
  // Socket::Address::SetIP
  //
  // Set IP
  //
  void Socket::Address::SetIP(const char *ip)
  {
    sin_addr.s_addr = inet_addr(ip);
  }


  //
  // Socket::Address::SetIP
  //
  // Set IP
  //
  void Socket::Address::SetIP(U32 ip)
  {
    sin_addr.s_addr = htonl(ip);
  }


  //
  // Socket::Address::SetIP
  //
  // Set port
  //
  void Socket::Address::SetPort(U16 port)
  {
    sin_port = htons(port);
  }


  //
  // Socket::Address::GetText
  //
  // Get the text representation of the address
  //
  const char * Socket::Address::GetText() const
  {
    return (inet_ntoa(sin_addr));
  }


  //
  // Socket::Address::GetDisplayText
  //
  // Get the text representation of the address
  //
  const char * Socket::Address::GetDisplayText() const
  {
    // Lookup the host for this address
    DNS::Host *host;
    
    if (DNS::GetByAddress(*this, host) && host)
    {
      // We have a host name, return it
      return (host->GetName());
    }
    else
    {
      return (inet_ntoa(sin_addr));
    }
  }



  //
  // Socket::Address::GetIP
  //
  // Get the IP of the address
  //
  U32 Socket::Address::GetIP() const
  {
    return (ntohl(sin_addr.s_addr));
  }


  //
  // Socket::Address::GetPort
  //
  // Get the port of the address
  //
  U16 Socket::Address::GetPort() const
  {
    return (ntohs(sin_port));
  }


  //
  // Is this a local address
  //
  void Socket::Address::SetLocal()
  {
    SetIP("127.0.0.1");
  }


  //
  // Is this a local address
  //
  Bool Socket::Address::IsLocal() const
  {
    return (sin_addr.s_addr == inet_addr("127.0.0.1"));
  }


  //
  // Is the given string a valid address ?
  //
  Bool Socket::Address::IsAddress(const char *address)
  {
    return (inet_addr(address) == INADDR_NONE ? FALSE : TRUE);
  }


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Socket
  //


  //
  // Default Constructor
  //
  Socket::Socket(Type type, Bool stats)
  : type(type),
    stats(NULL)
  {
    ASSERT(initialized)

    if (stats)
    {
      UseStats();
    }

    Open();
  }


  //
  // Copy constructor
  //
  Socket::Socket(const Socket &socket)
  : socket(socket.socket),
    stats(NULL)
  {
    ASSERT(initialized)
  }


  //
  // Initializing constructor
  //
  Socket::Socket(const SOCKET &socket)
  : socket(socket),
    stats(NULL)
  {
    ASSERT(initialized)
  }


  //
  // Destructor
  //
  Socket::~Socket()
  {
    ASSERT(initialized)
  }


  //
  // Use Statistics
  //
  void Socket::UseStats()
  {
    if (!stats)
    {
      stats = new Statistics;
    }
  }


  //
  // Get Statistics
  //
  const Socket::Statistics & Socket::GetStats()
  {
    if (stats)
    {
      return (*stats);
    }
    else
    {
      LERR("Socket doesn't contain statistics")
      return (*stats);
    }
  }

  
  //
  // Set a socket option
  //
  Bool Socket::SetSockOpt(U32 proto, U32 option, const char *value, U32 length)
  {
    if (setsockopt(socket, proto, option, value, length) == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("setsockopt: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("setsockopt: The network subsystem has failed.")

        case WSAEFAULT:
          LERR("setsockopt: optval is not in a valid part of the process address space or optlen parameter is too small.")

        case WSAEINPROGRESS:
          LERR("setsockopt: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAEINVAL:
          LERR("setsockopt: level is not valid, or the information in optval is not valid.")

        case WSAENETRESET:
          LERR("setsockopt: Connection has timed out when SO_KEEPALIVE is set.")

        case WSAENOPROTOOPT:
          LERR("setsockopt: The option is unknown or unsupported for the specified provider or socket")

        case WSAENOTCONN:
          LERR("setsockopt: Connection has been reset when SO_KEEPALIVE is set.")

        case WSAENOTSOCK:
          LERR("setsockopt: The descriptor is not a socket.")

        default:
          LERR("setsockopt: Unknown error!")
      }
    }
    return (TRUE);
  }


  //
  // Get a socket option
  //
  Bool Socket::GetSockOpt(U32 proto, U32 option, char *value, U32 &length)
  {
    if (getsockopt(socket, proto, option, value, (SI *) &length) == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("getsockopt: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("getsockopt: The network subsystem has failed.")

        case WSAEFAULT:
          LERR("getsockopt: One of the optval or the optlen parameters is not a valid part of the user address space, or the optlen parameter is too small.")

        case WSAEINPROGRESS:
          LERR("getsockopt: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAEINVAL:
          LERR("getsockopt: The level parameter is unknown or invalid.")

        case WSAENOPROTOOPT:
          LERR("getsockopt: The option is unknown or unsupported by the indicated protocol family. ")

        case WSAENOTSOCK:
          LERR("getsockopt: The descriptor is not a socket.")

        default:
          LERR("getsockopt: Unknown error!")
      }
    }
    return (TRUE);
  }


  //
  // Connect this socket to the given address
  //
  Bool Socket::Connect(const Address &address)
  {
    ASSERT(initialized)

    if (WSAConnect(socket, (const sockaddr *) &address, sizeof (Address), NULL, NULL, NULL, NULL) == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("WSAConnect: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("WSAConnect: The network subsystem has failed.")
    
        case WSAEADDRINUSE:
          LERR("WSAConnect: The local address of the socket is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs during the execution of bind, but could be delayed until this function if the bind function operates on a partially wild-card address (involving ADDR_ANY) and if a specific address needs to be \"committed\" at the time of this function.")
    
        case WSAEINTR:
          LERR("WSAConnect: The (blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall.")
    
        case WSAEINPROGRESS:
          LERR("WSAConnect: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")
    
        case WSAEALREADY:
          LERR("WSAConnect: A nonblocking connect/WSAConnect call is in progress on the specified socket.")
    
        case WSAEADDRNOTAVAIL:
          LDIAG("WSAConnect: The remote address is not a valid address (such as ADDR_ANY).")
          return (FALSE);
    
        case WSAEAFNOSUPPORT:
          LDIAG("WSAConnect: Addresses in the specified family cannot be used with this socket.")
          return (FALSE);
    
        case WSAECONNREFUSED:
          LDIAG("WSAConnect: The attempt to connect was rejected.")
          return (FALSE);
    
        case WSAEFAULT:
          LDIAG("WSAConnect: The name or the namelen parameter is not a valid part of the user address space, the namelen parameter is too small, the buffer length for lpCalleeData, lpSQOS, and lpGQOS are too small, or the buffer length for lpCallerData is too large.")
          return (FALSE);
    
        case WSAEINVAL:
          LERR("WSAConnect: The parameter s is a listening socket, or the destination address specified is not consistent with that of the constrained group to which the socket belongs.")
    
        case WSAEISCONN:
          LERR("WSAConnect: The socket is already connected (connection-oriented sockets only).")
    
        case WSAENETUNREACH:
          LDIAG("WSAConnect: The network cannot be reached from this host at this time.")
          return (FALSE);
    
        case WSAENOBUFS:
          LERR("WSAConnect: No buffer space is available. The socket cannot be connected.")
    
        case WSAENOTSOCK:
          LDIAG("WSAConnect: The descriptor is not a socket.")
          return (FALSE);
    
        case WSAEOPNOTSUPP:
          LERR("WSAConnect: The FLOWSPEC structures specified in lpSQOS and lpGQOS cannot be satisfied.")
    
        case WSAEPROTONOSUPPORT:
          LERR("WSAConnect: The lpCallerData argument is not supported by the service provider.")
    
        case WSAETIMEDOUT:
          LDIAG("WSAConnect: Attempt to connect timed out without establishing a connection.")
          return (FALSE);
    
        case WSAEWOULDBLOCK:
          //LDIAG("WSAConnect: The socket is marked as nonblocking and the connection cannot be completed immediately.")
          return (TRUE);
    
        case WSAEACCES:
          LERR("WSAConnect: Attempt to connect datagram socket to broadcast address failed because setsockopt SO_BROADCAST is not enabled.")

        default:
          LERR("WSAConnect: Unknown Error")
      }
    }

    return (TRUE);
  }


  //
  // Bind this socket to the given local address
  //
  Bool Socket::Bind(const Address &address)
  {
    ASSERT(initialized)

    if (bind(socket, (const sockaddr *) &address, sizeof (Address)) == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("bind: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("bind: The network subsystem has failed.")

        case WSAEADDRINUSE:
          LERR("bind: A process on the machine is already bound to the same fully-qualified address and the socket has not been marked to allow address re-use with SO_REUSEADDR. For example, the IP address and port are bound in the af_inet case). (See the SO_REUSEADDR socket option under setsockopt.)")

        case WSAEADDRNOTAVAIL:
          LERR("bind: The specified address is not a valid address for this machine.")

        case WSAEFAULT:
          LERR("bind: The name or namelen parameter is not a valid part of the user address space, the namelen parameter is too small, the name parameter contains an incorrect address format for the associated address family, or the first two bytes of the memory block specified by name does not match the address family associated with the socket descriptor s.")

        case WSAEINPROGRESS:
          LERR("bind: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAEINVAL:
          LERR("bind: The socket is already bound to an address.")

        case WSAENOBUFS:
          LERR("bind: Not enough buffers available, too many connections.")

        case WSAENOTSOCK:
          LERR("bind: The descriptor is not a socket.")

        default:
          LERR("bind: Unknown Error")
      }
    }
    return (TRUE);
  }


  //
  // Listen
  //
  Bool Socket::Listen(U32 backlog)
  {
    ASSERT(initialized)

    if (listen(socket, backlog) == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("listen: A successful WSAStartup call must occur before using this function.");

        case WSAENETDOWN:
          LERR("listen: The network subsystem has failed.");

        case WSAEADDRINUSE:
          LERR("listen: The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs during execution of the bind function, but could be delayed until this function if the bind was to a partially wild-card address (involving ADDR_ANY) and if a specific address needs to be committed at the time of this function.");

        case WSAEINPROGRESS:
          LERR("listen: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");

        case WSAEINVAL:
          LERR("listen: The socket has not been bound with bind.");

        case WSAEISCONN:
          LERR("listen: The socket is already connected.");

        case WSAEMFILE:
          LERR("listen: No more socket descriptors are available.");

        case WSAENOBUFS:
          LERR("listen: No buffer space is available.");

        case WSAENOTSOCK:
          LERR("listen: The descriptor is not a socket.");

        case WSAEOPNOTSUPP:
          LERR("listen: The referenced socket is not of a type that supports the listen operation.");

        default:
          LERR("listen: Unknown Error")
      }
    }
    return (TRUE);
  }


  //
  // Accept a connection
  //
  Socket Socket::Accept(Address &address)
  {
    ASSERT(initialized)

    SOCKET s;

    SI size = sizeof (SOCKADDR_IN);
    s = accept(socket, reinterpret_cast<SOCKADDR *>(&address), &size);

    if (s == INVALID_SOCKET)
    {
      int error = WSAGetLastError();
      switch (error)
      {
        case WSANOTINITIALISED:
          LERR("accept: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("accept: The network subsystem has failed.")

        case WSAEFAULT:
          LERR("accept: The addrlen parameter is too small or addr is not a valid part of the user address space.")

        case WSAEINTR:
          LERR("accept: A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.")

        case WSAEINPROGRESS:
          LERR("accept: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAEINVAL:
          LERR("accept: The listen function was not invoked prior to accept.")

        case WSAEMFILE:
          LERR("accept: The queue is nonempty upon entry to accept and there are no descriptors available.")

        case WSAENOBUFS:
          LERR("accept: No buffer space is available.")

        case WSAENOTSOCK:
          LERR("accept: The descriptor is not a socket.")

        case WSAEOPNOTSUPP:
          LERR("accept: The referenced socket is not a type that supports connection-oriented service.")

        case WSAEWOULDBLOCK:
          LERR("accept: The socket is marked as nonblocking and no connections are present to be accepted.")

        default:
          LERR("accept: Unknown Error " << error)
      }
    }

    return (Socket(s));
  }


  //
  // Open a new socket
  //
  void Socket::Open()
  {
    if (stats)
    {
      stats->Opened();
    }

    switch (type)
    {
      case UDP:
        socket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        break;

      case TCP:
        socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        break;

      case RAW: // We only endorse ICMP proto RAW sockets
        socket = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);
        break;

      default:
        LERR("Unknown socket type! " << type)
        break;
    }

    if (socket == INVALID_SOCKET)
    {
      int error = WSAGetLastError();
      switch (error)
      {
        case WSANOTINITIALISED:
          LERR("WSASocket: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("WSASocket: The network subsystem has failed.")

        case WSAEAFNOSUPPORT:
          LERR("WSASocket: The specified address family is not supported.")

        case WSAEINPROGRESS:
          LERR("WSASocket: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAEMFILE:
          LERR("WSASocket: No more socket descriptors are available.")

        case WSAENOBUFS:
          LERR("WSASocket: No buffer space is available. The socket cannot be created.")

        case WSAEPROTONOSUPPORT:
          LERR("WSASocket: The specified protocol is not supported.")

        case WSAEPROTOTYPE:
          LERR("WSASocket: The specified protocol is the wrong type for this socket.")

        case WSAESOCKTNOSUPPORT:
          LERR("WSASocket: The specified socket type is not supported in this address family.")

        case WSAEINVAL:
          LERR("WSASocket: This value is true for any of the following conditions. The parameter g specified is not valid. The WSAPROTOCOL_INFO structure that lpProtocolInfo points to is incomplete, the contents are invalid or the WSAPROTOCOL_INFO structure has already been used in an earlier duplicate socket operation. The values specified for members of the socket triple <af, type, and protocol> are individually supported, but the given combination is not.")
       
        case WSAEFAULT:
          LERR("WSASocket: lpProtocolInfo argument is not in a valid part of the process address space.")

  //      case WSAINVALIDPROVIDER:
  //        LERR("WSASocket: The service provider returned a version other than 2.2.")
      
  //      case WSAINVALIDPROCTABLE:
  //        LERR("WSASocket: The service provider returned an invalid or incomplete procedure table to the WSPStartup.")

        case WSAEACCES:
          // Only occurs with RAW sockets, must be handled by maintainer of RAW sockets
          LDIAG("WSASocket: Not enough priveleges to perform this operation")
          break;

        default:
          LERR("WSASocket: Unknown Error " << dec << error << " [" << HEX(8, error) << "]")
      }
    }
  }


  //
  // Close a connection
  //
  void Socket::Close()
  {
    if (stats)
    {
      stats->Report();
      delete stats;
      stats = NULL;
    }

    if (closesocket(socket) == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("closesocket: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("closesocket: The network subsystem has failed.")

        case WSAENOTSOCK:
          // This happens when closing an already closed socket, we don't care about it
          //LERR("closesocket: The descriptor is not a socket.")
          break;

        case WSAEINPROGRESS:
          LERR("closesocket: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAEINTR:
          LERR("closesocket: The (blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall.")

        case WSAEWOULDBLOCK:
          LERR("closesocket: The socket is marked as nonblocking and SO_LINGER is set to a nonzero time-out value.")

        default:
          LERR("closesocket: Unknown Error")
      }
    }

    // Invalidate the socket
    socket = INVALID_SOCKET;
  }


  //
  // Get the local address of a socket
  //
  Bool Socket::GetLocalAddress(Address &address)
  {
    SI addressLength = sizeof (Address);
    if (getsockname(socket, reinterpret_cast<sockaddr *>(&address), &addressLength))
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("getsockname: A successful WSAStartup call must occur before using this API.")

        case WSAENETDOWN:
          LDIAG("getsockname: The network subsystem has failed.")
          return (FALSE);

        case WSAEFAULT:
          LERR("getsockname: The name or the namelen parameter is not a valid part of the user address space, or the namelen parameter is too small.")

        case WSAEINPROGRESS:
          LERR("getsockname: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAENOTSOCK:
          LDIAG("getsockname: The descriptor is not a socket.")
          return (FALSE);

        case WSAEINVAL:
          LDIAG("getsockname: The socket has not been bound to an address with bind, or ADDR_ANY is specified in bind but connection has not yet occurs.")
          return (FALSE);

        default:
          LERR("getsockname: Unknown Error!")
      }
    }
    return (TRUE);
  }


  //
  // Setup an event for this socket
  //
  void Socket::EventSelect(Win32::Event &event, U32 events)
  {
    ASSERT(initialized)

    if (WSAEventSelect(socket, event.GetHandle(), events) == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("WSAEventSelect: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("WSAEventSelect: The network subsystem has failed.")

        case WSAEINVAL:
          LERR("WSAEventSelect: Indicates that one of the specified parameters was invalid, or the specified socket is in an invalid state.")

        case WSAEINPROGRESS:
          LERR("WSAEventSelect: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAENOTSOCK:
          LERR("WSAEventSelect: The descriptor is not a socket.")

        default:
          LERR("WSAEventSelect: Unknown Error")
      }
    }
  }


  //
  // Enum events
  //
  Bool Socket::EnumEvents(const Win32::Event &event, NetworkEvents &networkEvents)
  {
    ASSERT(initialized)

    if (WSAEnumNetworkEvents(socket, event.GetHandle(), &networkEvents) == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("WSAEnumNetworkEvents: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("WSAEnumNetworkEvents: The network subsystem has failed.")

        case WSAEINVAL:
          LERR("WSAEnumNetworkEvents: Indicates that one of the specified parameters was invalid.")

        case WSAEINPROGRESS:
          LERR("WSAEnumNetworkEvents: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAENOTSOCK:
          LDIAG("WSAEnumNetworkEvents: The descriptor is not a socket.")
          return (FALSE);

        case WSAEFAULT:
          LERR("WSAEnumNetworkEvents: The lpNetworkEvents argument is not a valid part of the user address space.")

        default:
          LERR("WSAEnumNetworkEvents: Unknown error")
      }
    }
    return (TRUE);
  }


  //
  // Send
  //
  U32 Socket::Send(const U8 *data, U32 length)
  {
    ASSERT(initialized)

    int sent = send(socket, (const char *) data, length, 0);

    if (sent == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("send: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("send: The network subsystem has failed.")

        case WSAEACCES:
          LERR("send: The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address.")

        case WSAEINTR:
          LERR("send: A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.")

        case WSAEINPROGRESS:
          LERR("send: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAEFAULT:
          LERR("send: The buf parameter is not completely contained in a valid part of the user address space.")

        case WSAENETRESET:
          LERR("send: The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.")

        case WSAENOBUFS:
          LERR("send: No buffer space is available.")

        case WSAENOTCONN:
          LDIAG("send: The socket is not connected.")
          return (0);

        case WSAENOTSOCK:
          LDIAG("send: The descriptor is not a socket.")
          return (0);

        case WSAEOPNOTSUPP:
          LERR("send: MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations.")

        case WSAESHUTDOWN:
          LERR("send: The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.")

        case WSAEWOULDBLOCK:
          //LDIAG("send: The socket is marked as nonblocking and the requested operation would block.")
          return (0);

        case WSAEMSGSIZE:
          LERR("send: The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.")

        case WSAEHOSTUNREACH:
          LERR("send: The remote host cannot be reached from this host at this time.")

        case WSAEINVAL:
          LERR("send: The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled.")

        case WSAECONNABORTED:
          //LDIAG("send: The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.")
          return (0);

        case WSAECONNRESET:
          //LDIAG("send: The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a \"Port Unreachable\" ICMP packet. The application should close the socket as it is no longer usable.")
          return (0);

        case WSAETIMEDOUT:
          //LERR("send: The connection has been dropped, because of a network failure or because the system on the other end went down without notice.")
          return (0);

        default:
          LERR("send: Unknown Error")
      }
    }

    return (sent);
  }


  //
  // Send
  //
  U32 Socket::Send(const Address &address, const U8 *data, U32 length)
  {
    ASSERT(initialized)

    int sent = sendto(socket, (const char *) data, length, 0, reinterpret_cast<const sockaddr *>(&address), sizeof (Address));

    if (sent == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("sendto: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          //LDIAG("sendto: The network subsystem has failed.")
          return (0);

        case WSAEACCES:
          LDIAG("sendto: The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address.")
          return (0);

        case WSAEINVAL:
          LERR("sendto: An unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled.")
 
        case WSAEINTR:
          LERR("sendto: A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.")
 
        case WSAEINPROGRESS:
          LERR("sendto: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")
 
        case WSAEFAULT:
          LDIAG("sendto: The buf or to parameters are not part of the user address space, or the tolen parameter is too small.")
          return (0);
 
        case WSAENETRESET:
          //LDIAG("sendto: The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.")
          return (0);
 
        case WSAENOBUFS:
          LERR("sendto: No buffer space is available.")
 
        case WSAENOTCONN:
          LERR("sendto: The socket is not connected (connection-oriented sockets only).")
 
        case WSAENOTSOCK:
          //LDIAG("sendto: The descriptor is not a socket.")
          return (0);
 
        case WSAEOPNOTSUPP:
          LERR("sendto: MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations.")
 
        case WSAESHUTDOWN:
          //LDIAG("sendto: The socket has been shut down; it is not possible to sendto on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.")
          return (0);
 
        case WSAEWOULDBLOCK:
          LERR("sendto: The socket is marked as nonblocking and the requested operation would block.")
 
        case WSAEMSGSIZE:
          LERR("sendto: The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.")
 
        case WSAEHOSTUNREACH:
          //LDIAG("sendto: The remote host cannot be reached from this host at this time.")
          return (0);
 
        case WSAECONNABORTED:
          //LDIAG("sendto: The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.")
          return (0);
 
        case WSAECONNRESET:
          //LDIAG("sendto: The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a \"Port Unreachable\" ICMP packet. The application should close the socket as it is no longer usable.")
          return (0);

        case WSAEADDRNOTAVAIL:
          LERR("sendto: The remote address is not a valid address, for example, ADDR_ANY.")
 
        case WSAEAFNOSUPPORT:
          LERR("sendto: Addresses in the specified family cannot be used with this socket.")
 
        case WSAEDESTADDRREQ:
          LERR("sendto: A destination address is required.")
 
        case WSAENETUNREACH:
          //LDIAG("sendto: The network cannot be reached from this host at this time.")
          return (0);
 
        case WSAETIMEDOUT:
          //LDIAG("sendto: The connection has been dropped, because of a network failure or because the system on the other end went down without notice.")
          return (0);

        default:
          LERR("sendto: Unknown Error")
      }
    }

    return (sent);
  }


  //
  // Send (Overlapped)
  //
  U32 Socket::Send(Buffer *buffers, U32 numBuffers, Overlapped &overlapped, CompletionRoutine completionRoutine)
  {
    ASSERT(initialized)

    U32 sent = 0;
    if 
    (
      WSASend
      (
        socket, 
        buffers, 
        numBuffers, 
        &sent, 
        0, 
        &overlapped, 
        reinterpret_cast<LPWSAOVERLAPPED_COMPLETION_ROUTINE>(completionRoutine)
      )
    )
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("WSASend: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("WSASend: The network subsystem has failed.")

        case WSAEACCES:
          LERR("WSASend: The requested address is a broadcast address, but the appropriate flag was not set.")

        case WSAEINTR:
          LERR("WSASend: A blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.")

        case WSAEINPROGRESS:
          LERR("WSASend: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAEFAULT:
          LERR("WSASend: The lpBuffers, lpNumberOfBytesSent, lpOverlapped, lpCompletionRoutine argument is not totally contained in a valid part of the user address space.")

        case WSAENETRESET:
          LERR("WSASend: The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.")

        case WSAENOBUFS:
          LERR("WSASend: The Windows Sockets provider reports a buffer deadlock.")

        case WSAENOTCONN:
          LERR("WSASend: The socket is not connected.")

        case WSAENOTSOCK:
          LERR("WSASend: The descriptor is not a socket.")

        case WSAEOPNOTSUPP:
          LERR("WSASend: MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, MSG_PARTIAL is not supported, or the socket is unidirectional and supports only receive operations.")

        case WSAESHUTDOWN:
          LERR("WSASend: The socket has been shut down; it is not possible to WSASend on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.")

        case WSAEWOULDBLOCK:
          LERR("WSASend: Overlapped sockets: There are too many outstanding overlapped I/O requests. Nonoverlapped sockets: The socket is marked as nonblocking and the send operation cannot be completed immediately.")

        case WSAEMSGSIZE:
          LERR("WSASend: The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.")

        case WSAEINVAL:
          LERR("WSASend: The socket has not been bound with bind or the socket is not created with the overlapped flag.")

        case WSAECONNABORTED:
          LERR("WSASend: The virtual circuit was terminated due to a time-out or other failure.")

        case WSAECONNRESET:
          LERR("WSASend: The virtual circuit was reset by the remote side.")

        case WSA_IO_PENDING:
          LERR("WSASend: An overlapped operation was successfully initiated and completion will be indicated at a later time.")

        case WSA_OPERATION_ABORTED:
          LERR("WSASend: The overlapped operation has been canceled due to the closure of the socket, or the execution of the SIO_FLUSH command in WSAIoctl.")

        default:
          LERR("WSASend: Unknown Error!")
      }
    }

    return (sent);
  }


  //
  // Send (to an address overlapped)
  //
  U32 Socket::Send(const Address &address, Buffer *buffers, U32 numBuffers, Overlapped &overlapped, CompletionRoutine completionRoutine)
  {
    ASSERT(initialized)

    U32 sent = 0;
    if 
    (
      WSASendTo
      (
        socket, 
        buffers, 
        numBuffers, 
        &sent, 
        0, 
        reinterpret_cast<const sockaddr *>(&address), 
        sizeof (Address), 
        &overlapped, 
        reinterpret_cast<LPWSAOVERLAPPED_COMPLETION_ROUTINE>(completionRoutine)
      )
    )
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("WSASendTo: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("WSASendTo: The network subsystem has failed.")

        case WSAEACCES:
          LERR("WSASendTo: The requested address is a broadcast address, but the appropriate flag was not set.")

        case WSAEINTR:
          LERR("WSASendTo: A blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.")

        case WSAEINPROGRESS:
          LERR("WSASendTo: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAEFAULT:
          LERR("WSASendTo: The lpBuffers, lpTo, lpOverlapped, lpNumberOfBytesSent, or lpCompletionRoutine parameters are not part of the user address space, or the lpTo argument is too small.")

        case WSAENETRESET:
          LERR("WSASendTo: The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.")

        case WSAENOBUFS:
          LERR("WSASendTo: The Windows Sockets provider reports a buffer deadlock.")

        case WSAENOTCONN:
          LERR("WSASendTo: The socket is not connected (connection-oriented sockets only).")

        case WSAENOTSOCK:
          LERR("WSASendTo: The descriptor is not a socket.")

        case WSAEOPNOTSUPP:
          LERR("WSASendTo: MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, MSG_PARTIAL is not supported, or the socket is unidirectional and supports only receive operations.")

        case WSAESHUTDOWN:
          LERR("WSASendTo: The socket has been shut down; it is not possible to WSASendTo on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.")

        case WSAEWOULDBLOCK:
          LERR("WSASendTo: Overlapped sockets: there are too many outstanding overlapped I/O requests. Nonoverlapped sockets: The socket is marked as nonblocking and the send operation cannot be completed immediately.")

        case WSAEMSGSIZE:
          LERR("WSASendTo: The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.")

        case WSAEINVAL:
          LERR("WSASendTo: The socket has not been bound with bind, or the socket is not created with the overlapped flag.")

        case WSAECONNABORTED:
          LERR("WSASendTo: The virtual circuit was terminated due to a time-out or other failure.")

        case WSAECONNRESET:
          LERR("WSASendTo: The virtual circuit was reset by the remote side.")

        case WSAEADDRNOTAVAIL:
          LERR("WSASendTo: The remote address is not a valid address (such as ADDR_ANY).")

        case WSAEAFNOSUPPORT:
          LERR("WSASendTo: Addresses in the specified family cannot be used with this socket.")

        case WSAEDESTADDRREQ:
          LERR("WSASendTo: A destination address is required.")

        case WSAENETUNREACH:
          LERR("WSASendTo: The network cannot be reached from this host at this time.")

        case WSA_IO_PENDING:
          LERR("WSASendTo: An overlapped operation was successfully initiated and completion will be indicated at a later time.")

        case WSA_OPERATION_ABORTED:
          LERR("WSASendTo: The overlapped operation has been canceled due to the closure of the socket, or the execution of the SIO_FLUSH command in WSAIoctl.")

        default:
          LERR("WSASendTo: Unknown Error!")
      }
    }

    return (sent);
  }


  //
  // Recv
  //
  U32 Socket::Recv(U8 *data, U32 length)
  {
    ASSERT(initialized)

    int received = recv(socket, (char *) data, length, 0);

    if (received == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("recv: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LDIAG("recv: The network subsystem has failed.")
          return (0);

        case WSAEFAULT:
          LERR("recv: The buf parameter is not completely contained in a valid part of the user address space.")

        case WSAENOTCONN:
          LDIAG("recv: The socket is not connected.")
          return (0);

        case WSAEINTR:
          LERR("recv: The (blocking) call was canceled through WSACancelBlockingCall.")

        case WSAEINPROGRESS:
          LERR("recv: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAENETRESET:
          LDIAG("recv: The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.") 
          return (0);

        case WSAENOTSOCK:
          LDIAG("recv: The descriptor is not a socket.")
          return (0);

        case WSAEOPNOTSUPP:
          LERR("recv: MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.")

        case WSAESHUTDOWN:
          LDIAG("recv: The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.")
          return (0);

        case WSAEWOULDBLOCK:
//        LDIAG("recv: The socket is marked as nonblocking and the receive operation would block.")
          // The operation would block (indicating that there's not enough data so we received zero)
          return (0);

        case WSAEMSGSIZE:
          LERR("recv: The message was too large to fit into the specified buffer and was truncated.")

        case WSAEINVAL:
          LERR("recv: The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative.")

        case WSAECONNABORTED:
          LDIAG("recv: The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.")
          return (0);

        case WSAETIMEDOUT:
          LDIAG("recv: The connection has been dropped because of a network failure or because the peer system failed to respond.")
          return (0);

        case WSAECONNRESET:
          LDIAG("recv: The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer usable. On a UPD-datagram socket this error would indicate that a previous send operation resulted in an ICMP \"Port Unreachable\" message.")
          return (0);

        default:
          LERR("recv: Unknown Error")
      }
    }

    return (received);
  }


  //
  // Recv
  //
  U32 Socket::Recv(Address &address, U8 *data, U32 length)
  {
    ASSERT(initialized)

    SI addressLength = sizeof (Address);
    int received = recvfrom(socket, (char *) data, length, 0, (sockaddr *) &address, &addressLength);

    if (received == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("recvfrom: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LDIAG("recvfrom: The network subsystem has failed.")
          return (0);

        case WSAEFAULT:
          LERR("recvfrom: The buf or from parameters are not part of the user address space, or the fromlen parameter is too small to accommodate the peer address.")

        case WSAEINTR:
          LERR("recvfrom: The (blocking) call was canceled through WSACancelBlockingCall.")

        case WSAEINPROGRESS:
          LERR("recvfrom: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAEINVAL:
          LDIAG("recvfrom: The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled, or (for byte stream-style sockets only) len was zero or negative.")
          return (0);

        case WSAEISCONN:
          LERR("recvfrom: The socket is connected. This function is not permitted with a connected socket, whether the socket is connection-oriented or connectionless.")

        case WSAENETRESET:
          LDIAG("recvfrom: The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.")
          return (0);

        case WSAENOTSOCK:
          LDIAG("recvfrom: The descriptor is not a socket.")
          return (0);

        case WSAEOPNOTSUPP:
          LERR("recvfrom: MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.")

        case WSAESHUTDOWN:
          LDIAG("recvfrom: The socket has been shut down; it is not possible to recvfrom on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.")
          return (0);

        case WSAEWOULDBLOCK:
          LDIAG("recvfrom: The socket is marked as nonblocking and the recvfrom operation would block.")
          return (0);

        case WSAEMSGSIZE:
          //LERR("recvfrom: The message was too large to fit into the specified buffer and was truncated.")
          return (0);

        case WSAETIMEDOUT:
          LDIAG("recvfrom: The connection has been dropped, because of a network failure or because the system on the other end went down without notice.")
          return (0);

        case WSAECONNRESET:
          //LERR("recvfrom: The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer usable. On a UPD-datagram socket this error would indicate that a previous send operation resulted in an ICMP \"Port Unreachable\" message.")
          return (0);

        default:
          LERR("recvfrom: Unknown Error")
      }
    }

    return (received);
  }


  //
  // Recv (Overlapped)
  //
  U32 Socket::Recv(Buffer *buffers, U32 numBuffers, Overlapped &overlapped, CompletionRoutine completionRoutine)
  {
    U32 received = 0;
    if 
    (
      WSARecv
      (
        socket, 
        buffers, 
        numBuffers, 
        &received, 
        0, 
        &overlapped, 
        reinterpret_cast<LPWSAOVERLAPPED_COMPLETION_ROUTINE>(completionRoutine)
      )
    )
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("WSARecv: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("WSARecv: The network subsystem has failed.")

        case WSAENOTCONN:
          LERR("WSARecv: The socket is not connected.")

        case WSAEINTR:
          LERR("WSARecv: The (blocking) call was canceled through WSACancelBlockingCall.")

        case WSAEINPROGRESS:
          LERR("WSARecv: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAENETRESET:
          LERR("WSARecv: The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.")

        case WSAENOTSOCK:
          LERR("WSARecv: The descriptor is not a socket.")

        case WSAEFAULT:
          LERR("WSARecv: The lpBuffers parameter is not completely contained in a valid part of the user address space.")

        case WSAEOPNOTSUPP:
          LERR("WSARecv: MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.")

        case WSAESHUTDOWN:
          LERR("WSARecv: The socket has been shut down; it is not possible to call WSARecv on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.")

        case WSAEWOULDBLOCK:
          LERR("WSARecv: Overlapped sockets: there are too many outstanding overlapped I/O requests. Nonoverlapped sockets: The socket is marked as nonblocking and the receive operation cannot be completed immediately.")

        case WSAEMSGSIZE:
          LERR("WSARecv: The message was too large to fit into the specified buffer and (for unreliable protocols only) any trailing portion of the message that did not fit into the buffer has been discarded.")

        case WSAEINVAL:
          LERR("WSARecv: The socket has not been bound (for example, with bind).")

        case WSAECONNABORTED:
          LERR("WSARecv: The virtual circuit was terminated due to a time-out or other failure.")

        case WSAECONNRESET:
          LERR("WSARecv: The virtual circuit was reset by the remote side.")

        case WSAEDISCON:
          LERR("WSARecv: Socket s is message oriented and the virtual circuit was gracefully closed by the remote side.")

        case WSA_IO_PENDING:
          LERR("WSARecv: An overlapped operation was successfully initiated and completion will be indicated at a later time.")

        case WSA_OPERATION_ABORTED:
          LERR("WSARecv: The overlapped operation has been canceled due to the closure of the socket.")

        default:
          LERR("WSARecv: Unknown Error!")
      }
    }

    return (received);
  }


  //
  // Recv (from an address overlapped)
  //
  U32 Socket::Recv(Address &address, Buffer *buffers, U32 numBuffers, Overlapped &overlapped, CompletionRoutine completionRoutine)
  {
    U32 received = 0;
    SI addressLength = sizeof (Address);
    if 
    (
      WSARecvFrom
      (
        socket, 
        buffers, 
        numBuffers, 
        &received, 
        0, 
        reinterpret_cast<sockaddr *>(&address), 
        &addressLength, 
        &overlapped, 
        reinterpret_cast<LPWSAOVERLAPPED_COMPLETION_ROUTINE>(completionRoutine)
      )
    )
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("WSARecvFrom: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("WSARecvFrom: The network subsystem has failed.")

        case WSAEFAULT:
          LERR("WSARecvFrom: The lpBuffers, lpFlags, lpFrom, lpNumberOfBytesRecvd, lpFromlen, lpOverlapped, or lpCompletionRoutine argument is not totally contained in a valid part of the user address space: the lpFrom buffer was too small to accommodate the peer address.")

        case WSAEINTR:
          LERR("WSARecvFrom: A blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.")

        case WSAEINPROGRESS:
          LERR("WSARecvFrom: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")

        case WSAEINVAL:
          LERR("WSARecvFrom: The socket has not been bound (with bind, for example).")

        case WSAEISCONN:
          LERR("WSARecvFrom: The socket is connected. This function is not permitted with a connected socket, whether the socket is connection-oriented or connectionless.")

        case WSAENETRESET:
          LERR("WSARecvFrom: The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.")

        case WSAENOTCONN:
          LERR("WSARecvFrom: The socket is not connected (connection-oriented sockets only).")

        case WSAEOPNOTSUPP:
          LERR("WSARecvFrom: MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.")

        case WSAESHUTDOWN:
          LERR("WSARecvFrom: The socket has been shut down; it is not possible to WSARecvFrom on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.")

        case WSAEWOULDBLOCK:
          LERR("WSARecvFrom: Overlapped sockets: There are too many outstanding overlapped I/O requests. Nonoverlapped sockets: The socket is marked as nonblocking and the receive operation cannot be completed immediately.")

        case WSAEMSGSIZE:
          LERR("WSARecvFrom: The message was too large to fit into the specified buffer and (for unreliable protocols only) any trailing portion of the message that did not fit into the buffer has been discarded.")

        case WSAECONNRESET:
          LERR("WSARecvFrom: The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer useable. On a UPD-datagram socket this error would indicate that a previous send operation resulted in an ICMP \"Port Unreachable\" message.")

        case WSAEDISCON:
          LERR("WSARecvFrom: Socket s is message oriented and the virtual circuit was gracefully closed by the remote side.")

        case WSA_IO_PENDING:
          LERR("WSARecvFrom: An overlapped operation was successfully initiated and completion will be indicated at a later time.")

        case WSA_OPERATION_ABORTED:
          LERR("WSARecvFrom: The overlapped operation has been canceled due to the closure of the socket.")

        default:
          LERR("WSARecvFrom: Unkown Error!")
      }
    }

    return (received);
  }


  //
  // Sent a packet of x bytes
  //
  void Socket::SampleSent(U32 bytes)
  {
    if (stats)
    {
      stats->Sent(bytes);
    }
  }


  //
  // Recv a packet of x bytes
  //
  void Socket::SampleRecv(U32 bytes)
  {
    if (stats)
    {
      stats->Recv(bytes);
    }
  }


  //
  // Initialization
  //
  void Socket::Init()
  {
    ASSERT(!initialized)

    #define CHECKFUNC(x) if (!x) { LERR("Could not find '" #x "' in ws2_32.dll") };

    dll = LoadLibrary("ws2_32.dll");

    if (dll)
    {
      WSAConnect = (WSAConnectFunc) GetProcAddress(dll, "WSAConnect");
      CHECKFUNC(WSAConnect)

      WSAEnumNetworkEvents = (WSAEnumNetworkEventsFunc) GetProcAddress(dll, "WSAEnumNetworkEvents");
      CHECKFUNC(WSAEnumNetworkEvents)

      WSAEnumProtocols = (WSAEnumProtocolsFunc) GetProcAddress(dll, "WSAEnumProtocolsA");
      CHECKFUNC(WSAEnumProtocols)

      WSAEventSelect = (WSAEventSelectFunc) GetProcAddress(dll, "WSAEventSelect");
      CHECKFUNC(WSAEventSelect)

      WSARecv = (WSARecvFunc) GetProcAddress(dll, "WSARecv");
      CHECKFUNC(WSARecv)

      WSARecvFrom = (WSARecvFromFunc) GetProcAddress(dll, "WSARecvFrom");
      CHECKFUNC(WSARecvFrom)

      WSASend = (WSASendFunc) GetProcAddress(dll, "WSASend");
      CHECKFUNC(WSASend)

      WSASendTo = (WSASendToFunc) GetProcAddress(dll, "WSASendTo");
      CHECKFUNC(WSASendTo)

      WSASocket = (WSASocketFunc) GetProcAddress(dll, "WSASocketA");
      CHECKFUNC(WSASocket)
    }
    else
    {
      ERR_MESSAGE(("This application requires Winsock 2.2.  The Winsock 2 DLL 'ws2_32.dll' could not be found on this system.  Winsock 2.2 can be installed from the installation CD or can be downloaded by going to http://www.microsoft.com and searching for Winsock2."))
    }

    WSADATA wsaData;
    WORD ver = MAKEWORD(2, 2);

    // Attempt to start WinSock
    switch (WSAStartup(ver, &wsaData)) 
    {
      case 0:
        break;

      case WSASYSNOTREADY: 
        LERR("WSAStartup: Indicates that the underlying network subsystem is not ready for network communication.")

      case WSAVERNOTSUPPORTED: 
        LERR("WSAStartup: The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.")

      case WSAEINPROGRESS: 
        LERR("WSAStartup: A blocking Windows Sockets 1.1 operation is in progress.")

      case WSAEPROCLIM: 
        LERR("WSAStartup: Limit on the number of tasks supported by the Windows Sockets implementation has been reached.")

      case WSAEFAULT:
        LERR("WSAStartup: The lpWSAData is not a valid pointer.")

      default:
        LERR("WSAStartup: Unknown Error")
    }

    // Confirm version
    if (
      LOBYTE(wsaData.wVersion) != 2 ||
      HIBYTE(wsaData.wVersion) != 2 ) 
    {
      WSACleanup();
      ERR_MESSAGE(("This application requires Winsock 2.2.  This system only has Winsock %d.%d.  Winsock 2.2 can be installed from the installation CD or can be downloaded by going to http://www.microsoft.com and searching for Winsock2.", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion)))
    }

    //LDIAG("Winsock Initialized")
    //LDIAG("Version: " << U32(LOBYTE(wsaData.wVersion)) << "." << U32(HIBYTE(wsaData.wVersion)))
    //LDIAG("High Version: " << U32(LOBYTE(wsaData.wHighVersion)) << "." << U32(HIBYTE(wsaData.wHighVersion)))
    //LDIAG("Description: " << wsaData.szDescription)
    //LDIAG("Status: " << wsaData.szSystemStatus)
    //LDIAG("Max Sockets: " << wsaData.iMaxSockets)
    //LDIAG("Vendor Info: " << wsaData.lpVendorInfo)


    U32 sizeProtocols = 0;
    if (WSAEnumProtocols(NULL, NULL, &sizeProtocols) == SOCKET_ERROR)
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("WSAEnumProtocols: A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("WSAEnumProtocols: The network subsystem has failed.")

        case WSAEINPROGRESS:
          LERR("WSAEnumProtocols: A blocking Windows Sockets 1.1 call is in progress.")

        case WSAEINVAL:
          LERR("WSAEnumProtocols: Indicates that one of the specified parameters was invalid.")

        case WSAENOBUFS:
          //LERR("WSAEnumProtocols: The buffer length was too small to receive all the relevant WSAPROTOCOL_INFO structures and associated information. Pass in a buffer at least as large as the value returned in lpdwBufferLength.")
          break;

        case WSAEFAULT:
          LERR("WSAEnumProtocols: One or more of the lpiProtocols, lpProtocolBuffer, or lpdwBufferLength arguments are not a valid part of the user address space.")

        default:
          LERR("WSAEnumProtocols: Unknown Error!")
      }
    }

    U32 numProtocols = sizeProtocols / sizeof (WSAPROTOCOL_INFO);
    //LDIAG("Number of protocols: " << numProtocols);

    if (numProtocols)
    {
      WSAPROTOCOL_INFO *protocols = new WSAPROTOCOL_INFO[sizeProtocols];
     
      if (WSAEnumProtocols(NULL, protocols, &sizeProtocols) == SOCKET_ERROR)
      {
        switch (WSAGetLastError())
        {
          case WSANOTINITIALISED:
            LERR("WSAEnumProtocols: A successful WSAStartup call must occur before using this function.")

          case WSAENETDOWN:
            LERR("WSAEnumProtocols: The network subsystem has failed.")

          case WSAEINPROGRESS:
            LERR("WSAEnumProtocols: A blocking Windows Sockets 1.1 call is in progress.")

          case WSAEINVAL:
            LERR("WSAEnumProtocols: Indicates that one of the specified parameters was invalid.")

          case WSAENOBUFS:
            LERR("WSAEnumProtocols: The buffer length was too small to receive all the relevant WSAPROTOCOL_INFO structures and associated information. Pass in a buffer at least as large as the value returned in lpdwBufferLength.")

          case WSAEFAULT:
            LERR("WSAEnumProtocols: One or more of the lpiProtocols, lpProtocolBuffer, or lpdwBufferLength arguments are not a valid part of the user address space.")

          default:
            LERR("WSAEnumProtocols: Unknown Error!")
        }
      }

      /*
      for (U32 p = 0; p < numProtocols; p++)
      {
        LDIAG("Name: " << protocols[p].szProtocol)
        LDIAG("ProviderId: " << GUID::GUID2Text(protocols[p].ProviderId))
        LDIAG("ServiceFlags:"
          << (protocols[p].dwServiceFlags1 & XP1_CONNECTIONLESS ? " Connectionless" : "")
          << (protocols[p].dwServiceFlags1 & XP1_GUARANTEED_DELIVERY ? " GuaranteedDelivery" : "")
          << (protocols[p].dwServiceFlags1 & XP1_GUARANTEED_ORDER ? " GuaranteedOrder" : "")
          << (protocols[p].dwServiceFlags1 & XP1_MESSAGE_ORIENTED ? " MessageOrientated" : "")
          << (protocols[p].dwServiceFlags1 & XP1_PSEUDO_STREAM ? " PseudoStream" : "")
          << (protocols[p].dwServiceFlags1 & XP1_GRACEFUL_CLOSE ? " GracefulClose" : "")
          << (protocols[p].dwServiceFlags1 & XP1_EXPEDITED_DATA ? " ExpeditedData" : "")
          << (protocols[p].dwServiceFlags1 & XP1_CONNECT_DATA ? " ConnectData" : "")
          << (protocols[p].dwServiceFlags1 & XP1_DISCONNECT_DATA ? " DisconnectData" : "")
          << (protocols[p].dwServiceFlags1 & XP1_INTERRUPT ? " Interrupt" : "")
          << (protocols[p].dwServiceFlags1 & XP1_SUPPORT_BROADCAST ? " SupportBroadcast" : "")
          << (protocols[p].dwServiceFlags1 & XP1_SUPPORT_MULTIPOINT ? " SupportMultipoint" : "")
          << (protocols[p].dwServiceFlags1 & XP1_MULTIPOINT_CONTROL_PLANE ? " MultipointControlPlane" : "")
          << (protocols[p].dwServiceFlags1 & XP1_MULTIPOINT_DATA_PLANE ? " MultipointDataPlane" : "")
          << (protocols[p].dwServiceFlags1 & XP1_QOS_SUPPORTED ? " QOSSupported" : "")
          << (protocols[p].dwServiceFlags1 & XP1_UNI_SEND ? " UniSend" : "")
          << (protocols[p].dwServiceFlags1 & XP1_UNI_RECV ? " UniRecv" : "")
          << (protocols[p].dwServiceFlags1 & XP1_IFS_HANDLES ? " IFSHandles" : "")
          << (protocols[p].dwServiceFlags1 & XP1_PARTIAL_MESSAGE ? " PartialMessage" : ""))
        LDIAG("ProviderFlags:"
          << (protocols[p].dwProviderFlags & PFL_MULTIPLE_PROTO_ENTRIES ? " MultipleProtoEntries" : "")
          << (protocols[p].dwProviderFlags & PFL_RECOMMENDED_PROTO_ENTRY  ? " RecommendedProtoEntry" : "")
          << (protocols[p].dwProviderFlags & PFL_HIDDEN ? " Hidden;)" : "")
          << (protocols[p].dwProviderFlags & PFL_MATCHES_PROTOCOL_ZERO  ? " MatchesProtocolZero" : ""))

        // WSAPROTOCOLCHAIN ProtocolChain;
        LDIAG(
          "Version: " << protocols[p].iVersion <<
          " Address Family: " << protocols[p].iAddressFamily <<
          " Max Address Length: " << protocols[p].iMaxSockAddr <<
          " Min Address Length: " << protocols[p].iMinSockAddr)
        LDIAG(
          "Socket Type: " << protocols[p].iSocketType <<
          " Protocol: " << protocols[p].iProtocol <<
          " Protocol Max Offset: " << protocols[p].iProtocolMaxOffset)
        LDIAG(
          "Network Byte Order: " << (protocols[p].iNetworkByteOrder ? "LITTLEENDIAN" : "BIGENDIAN") <<
          " SecurityScheme: " << protocols[p].iSecurityScheme <<
          " Message Size: " << protocols[p].dwMessageSize <<
          " Provider Reserved: " << protocols[p].dwProviderReserved)
      }
      */

      delete [] protocols;
    }

    initialized = TRUE;
  }


  //
  // Shutdown
  //
  void Socket::Done()
  {
    ASSERT(initialized)

    if (!WSACleanup())
    {
      switch (WSAGetLastError())
      {
        case WSANOTINITIALISED:
          LERR("A successful WSAStartup call must occur before using this function.")

        case WSAENETDOWN:
          LERR("The network subsystem has failed.")

        case WSAEINPROGRESS:
          LERR("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")
      }
    }

    FreeLibrary(dll);

    initialized = FALSE;
  }
}

