////////////////////////////////////////////////////////////////////////////////
//
// Socket
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __WIN32_SOCKET_H
#define __WIN32_SOCKET_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32.h"
#include <winsock2.h>
#include "win32_event.h"
#include "stream.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace Win32
//
namespace Win32
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Socket
  //
  class Socket
  {
  public:


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Class Statistics
    //
    class Statistics
    {
    public:

      struct Flow
      {
        // Number of packets
        U32 packets;

        // Number of bytes
        U32 bytes;

        // Rate
        F32 rate;

        // Recent rate
        F32 smoothRate;

        // Maximum rate
        F32 maxRate;

        // Minimum rate
        F32 minRate;

        // Size accumulated since last rate reading
        U32 markSize;

        // Time accumulated since last rate readins
        U32 markTime;

        // Constructor
        Flow();

        // Reset
        void Reset();

        // Add a packet
        AddPacket(U32 size);

        // Write a report to the log file for this flow
        void Report(U32 time) const;

      };

    private:

      U32 timeOpened;
      Flow recv;
      Flow sent;

    public:

      // Socket was opened
      void Opened();

      // Packet was received
      void Recv(U32 size);

      // Packet was sent
      void Sent(U32 size);

      // Write a report to the log file for this socket
      void Report() const;

    public:

      U32 GetTimeOpened() const
      {
        return (timeOpened);
      }

      const Flow & GetRecvFlow() const
      {
        return (recv);
      }

      const Flow & GetSentFlow() const
      {
        return (sent);
      }

    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Class Address
    //
    class Address : public sockaddr_in
    {
    public:

      // Constructors
      Address();
      Address(const char *ip, U16 port);
      Address(U32 ip, U16 port);
      Address(const Address &address);
      Address(const sockaddr_in &addr);

      // Set ip
      void SetIP(const char *ip);

      // Set ip
      void SetIP(U32 ip);

      // Set port
      void SetPort(U16 port);

      // Get the text representation of the address
      const char * GetText() const;

      // Get the display representation of the address (will use DNS)
      const char * GetDisplayText() const;

      // Get the IP of the address
      U32 GetIP() const;

      // Get the port of the address
      U16 GetPort() const;

      // Set this address to be local
      void SetLocal();

      // Is this a local address
      Bool IsLocal() const;

      // Comparison operators
      Bool operator==(const Address &a) const
      {
        return (sin_addr.s_addr == a.sin_addr.s_addr && sin_port == a.sin_port);
      }
      Bool operator!=(const Address &a) const
      {
        return (sin_addr.s_addr != a.sin_addr.s_addr || sin_port != a.sin_port);
      }

      // Stream operator
      friend ostream & operator<<(ostream &o, const Address &a)
      {
        return (o << a.GetText() << ":" << a.GetPort());
      }

    public:

      // Is the given string a valid address ?
      static Bool IsAddress(const char *address);
    
    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Class Overlapped
    //
    class Overlapped : public WSAOVERLAPPED
    {
    public:

      // Constructor which takes an event
      Overlapped(const Win32::Event &event);

      // Constructor which takes a completion function and context data
      Overlapped();

    };

    typedef void (CALLBACK *CompletionRoutine)(U32 error, U32 transferred, Overlapped *overlapped, U32 flags);


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Class Buffer
    //
    class Buffer : public WSABUF
    {
    public:

      // Constructor
      Buffer(U8 *data, U32 length);

    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Class NetworkEvents
    //
    class NetworkEvents : public WSANETWORKEVENTS
    {
    public:

      // Get the event bitmask
      U32 GetEvents()
      {
        return (lNetworkEvents);
      }

      // Get the error code for the given bit
      S32 GetError(U32 eventBit)
      {
        return (iErrorCode[eventBit]);
      }

    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Enum Type
    //
    enum Type
    {
      TCP,
      UDP,
      RAW
    };


  private:

    // Windows socket
    SOCKET socket;

    // Type
    Type type;

    // Optional statistics
    Statistics *stats;

  public:

    // Default Constructor
    Socket(Type type = TCP, Bool stats = FALSE);

    // Copy constructor
    Socket(const Socket &socket);

    // Initializing constructor
    Socket(const SOCKET &socket);

    // Destructor
    ~Socket();


    // Use Statistics
    void UseStats();

    // Get Statistics
    const Statistics & GetStats();


    // Set a socket option
    Bool SetSockOpt(U32 proto, U32 option, const char *value, U32 length);

    // Get a socket option
    Bool GetSockOpt(U32 proto, U32 option, char *value, U32 &length);

 
    // Connect this socket to the given address
    Bool Connect(const Address &address);

    // Bind this socket to the given local address
    Bool Bind(const Address &address);

    // Listen
    Bool Listen(U32 backlog);


    // Accept a connection
    Socket Accept(Address &address);


    // Open a new socket
    void Open();

    // Close a connection
    void Close();


    // Get the local address of a socket
    Bool GetLocalAddress(Address &address);


    // Setup an event for this socket
    void EventSelect(Win32::Event &event, U32 events);

    // Enum events
    Bool EnumEvents(const Win32::Event &event, NetworkEvents &events);


    // Ioctl
    void Ioctl();


    // Send
    U32 Send(const U8 *data, U32 length);

    // Send (to an address)
    U32 Send(const Address &address, const U8 *data, U32 length);

    // Send (Overlapped)
    U32 Send(Buffer *buffers, U32 numBuffers, Overlapped &overlapped, CompletionRoutine completionRoutine = NULL);

    // Send (to an address overlapped)
    U32 Send(const Address &address, Buffer *buffers, U32 numBuffers, Overlapped &overlapped, CompletionRoutine completionRoutine = NULL);


    // Recv
    U32 Recv(U8 *data, U32 length);

    // Recv (from an address)
    U32 Recv(Address &address, U8 *data, U32 length);

    // Recv (Overlapped)
    U32 Recv(Buffer *buffers, U32 numBuffers, Overlapped &overlapped, CompletionRoutine completionRoutine = NULL);

    // Recv (from an address overlapped)
    U32 Recv(Address &address, Buffer *buffers, U32 numBuffers, Overlapped &overlapped, CompletionRoutine completionRoutine = NULL);


    // Statistical Gatherers

    // Sent a packet of x bytes
    void SampleSent(U32 bytes);

    // Recv a packet of x bytes
    void SampleRecv(U32 bytes);

  public:

    // Is the socket valid
    Bool IsValid()
    {
      return (socket == INVALID_SOCKET ? FALSE : TRUE);
    }

  public:

    // Initialization and Shutdown
    static void Init();
    static void Done();

  };

}

#endif