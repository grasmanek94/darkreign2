////////////////////////////////////////////////////////////////////////////////
//
// Network Ping
//
// Copyright 1999-2000
// Matthew Versluys
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "network_ping.h"
#include "network_ip.h"
#include "network_icmp.h"
#include "clock.h"
#include <ws2tcpip.h>
#include "logging.h"
#include "queue.h"
#include "win32_thread.h"
#include "win32_critsec.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace Network
//
namespace Network
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Ping
  //
  namespace Ping
  {

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Struct Handle
    //
    struct Handle
    {
      // Address being pinged
      Win32::Socket::Address address;

      // Callback
      Callback callback;

      // Context
      void *context;

      // List node
      NList<Handle>::Node node;

      // Constructor
      Handle(const Win32::Socket::Address &address, Callback callback, void *context)
      : address(address),
        callback(callback),
        context(context)
      {
      }

    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Struct Event
    //
    struct Event
    {
      // Handle 
      Handle *handle;

      // RTT
      U32 rtt;

      // Hop Count
      U32 hopCount;

    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Class Method
    //
    class Method
    {
    public:

      // Constructor and Destructor
      Method() {} 
      virtual ~Method() {}

      // Is the method available
      virtual Bool IsAvailable() = 0;

      // Send a ping
      virtual void Send(Handle &handle) = 0;

    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Class IPHelperAPI
    //
    class IPHelperAPI : public Method
    {
    private:

      typedef BOOL (STDCALL *GetRTTAndHopCountFunc)(U32 DestIpAddress, PULONG HopCount, ULONG MaxHops, PULONG RTT);

      // DLL
      static HINSTANCE dll;

      // Get RttAndHopCount function call
      static GetRTTAndHopCountFunc GetRTTAndHopCount;

      // The thread for sending/recving pings
      Win32::Thread thread;

      // Event to stop ping thread
      Win32::EventIndex eventQuit;

      // Event to indicate there's a new ping
      Win32::EventIndex eventAdd;

      // Critical section for accessing handles
      Win32::CritSec handleCritSec;

      // List of outgoing pings
      NList<Handle> handles;

    public:

      // Constructor and Destructor
      IPHelperAPI();
      ~IPHelperAPI();

      // Is the method available
      Bool IsAvailable();

      // Send a ping
      void Send(Handle &handle);

    private:

      // Thread for handling pings
      static U32 STDCALL ThreadProc(void *context);

    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Class RawSocket
    //
    class RawSocket : public Method
    {
    private:

      struct Seq
      {
        U16 out;
        U16 in;
        NBinTree<Seq>::Node node;

        Seq()
        : out(0), in(0)
        {
        }
      };

      // Socket
      Win32::Socket pingSocket;

      // The thread for sending/recving pings
      Win32::Thread thread;

      // Event to stop ping thread
      Win32::EventIndex eventQuit;

      // Sequence numbers by address
      NBinTree<Seq> sequenceNumbers;

      // Critical section for accessing handles
      Win32::CritSec handleCritSec;

      // Handles
      NList<Handle> handles;

    public:

      // Constructor and Destructor
      RawSocket();
      ~RawSocket();

      // Is the method available
      Bool IsAvailable();

      // Send a ping
      void Send(Handle &handle);

    private:

      // Build a ping packet
      void BuildPacket(U8 *buffer, U32 size, U32 seq);

      // Find a handle by ip
      Handle * FindHandleByAddress(U32 ip);

      // Receive a ping
      static Bool Recv(Win32::Socket &socket, Win32::Socket::Address &address, U32 &type, U16 &seq, U32 &rtt, U8 &hops);

      // Thread for handling pings
      static U32 STDCALL ThreadProc(void *context);

    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //
    static Bool initialized = FALSE;
    static Method *method;
    static SafeQueue<Event, 256> events;


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    Handle * FindHandleByAddress(U32 ip);


    //
    // Initialization
    //
    void Init()
    {
      ASSERT(!initialized)

      LDIAG("Trying RawSocket")
      method = new RawSocket();
      if (!method->IsAvailable())
      {
        LDIAG("Trying IP Helper API")
        delete method;
        method = new IPHelperAPI();
        if (!method->IsAvailable())
        {
          LERR("No ping methods available!")
        }
      }

      initialized = TRUE;
    }


    //
    // Shutdown
    //
    void Done()
    {
      ASSERT(initialized)

      // Cleanup the method
      delete method;

      initialized = FALSE;
    }


    //
    // Send a ping
    //
    void Send(const Win32::Socket::Address &address, Callback callback, void *context)
    {
      ASSERT(initialized)

      // For this address, add an entry so that when we receive pings we can call the correct callback
      Handle *handle = new Handle(address, callback, context);

      method->Send(*handle);
    }


    //
    // Process any pings
    //
    void Process()
    {
      ASSERT(initialized)

      // Are there any events in the event queue ?
      while (Event *e = events.RemovePre(0))
      {
        // We got one
        e->handle->callback(e->handle->address, e->rtt, e->hopCount, e->handle->context);

        // Delete the handle
        delete e->handle;

        events.RemovePost();
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Class IPHelperAPI
    //

    // DLL
    HINSTANCE IPHelperAPI::dll = NULL;

    // Get RttAndHopCount function call
    IPHelperAPI::GetRTTAndHopCountFunc IPHelperAPI::GetRTTAndHopCount = NULL;


    //
    // Constructor
    //
    IPHelperAPI::IPHelperAPI()
    : handles(&Handle::node)
    {
      if (!dll)
      {
        dll = LoadLibrary("iphlpapi.dll");

        #define CHECKFUNC(x) if (!x) { LERR("Could not find '" #x "' in ws2_32.dll") };

        if (dll)
        {
          GetRTTAndHopCount = (GetRTTAndHopCountFunc) GetProcAddress(dll, "GetRTTAndHopCount");
          CHECKFUNC(GetRTTAndHopCount)
        }

        LDIAG("Starting IP Helper thread")

        // Start the thread
        thread.Start(ThreadProc, this);
      }
    }


    //
    // Destructor
    //
    IPHelperAPI::~IPHelperAPI()
    {
      if (dll)
      {
        FreeLibrary(dll);
        dll = NULL;
      }

      LDIAG("Stopping IP Helper thread")

      // Signal the quit event
      eventQuit.Signal();

      // Wait for the thread to stop
      thread.Stop();

      // Delete any remaining handles
      handles.DisposeAll();
    }


    //
    // Is the method available
    //
    Bool IPHelperAPI::IsAvailable()
    {
      return (dll ? TRUE : FALSE);
    }


    //
    // Send a ping
    //
    void IPHelperAPI::Send(Handle &handle)
    {
      LDIAG("Adding ping to " << handle.address << " to queue")

      // Add the handle to the outgoing queue
      handleCritSec.Enter();
      handles.Append(&handle);
      handleCritSec.Exit();

      eventAdd.Signal();
    }


    //
    // Thread for handling pings
    //
    U32 STDCALL IPHelperAPI::ThreadProc(void *context)
    {
      IPHelperAPI *iphelper = static_cast<IPHelperAPI *>(context);

      Bool quit = FALSE;
      Win32::EventIndex::List<2> events;
      events.AddEvent(iphelper->eventAdd, NULL);
      events.AddEvent(iphelper->eventQuit, iphelper);

      // Enter processing loop
      while (!quit)
      {
        void *context;
        if (events.Wait(context, FALSE))
        {
          if (!context)
          {
            Handle *handle = NULL;
            do
            {
              iphelper->handleCritSec.Enter();
              handle = iphelper->handles.GetHead();
              if (handle)
              {
                iphelper->handles.Unlink(handle);
              }
              iphelper->handleCritSec.Exit();

              if (handle)
              {
                LDIAG("Pinging " << handle->address)

                U32 hopCount;
                U32 rtt;

                if (GetRTTAndHopCount(htonl(handle->address.GetIP()), &hopCount, 256, &rtt))
                {
                  LDIAG("Got ping RTT " << rtt << " Hops " << hopCount)

                  Event *e = Ping::events.AddPre();
                  e->handle = handle;
                  e->rtt = rtt;
                  e->hopCount = hopCount;
                  Ping::events.AddPost();
                }
                else
                {
                  LDIAG("Ping failed")

                  Event *e = Ping::events.AddPre();
                  e->handle = handle;
                  e->rtt = U32_MAX;
                  e->hopCount = U32_MAX;
                  Ping::events.AddPost();
                }
              }
            }
            while (handle);
          }
          else
          {
            LDIAG("Got quit event")

            quit = TRUE;
          }
        }
      }
      return (TRUE);
    }


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Class RawSocket
    //


    //
    // Constructor
    //
    RawSocket::RawSocket()
    : pingSocket(Win32::Socket::RAW),
      sequenceNumbers(&Seq::node),
      handles(&Handle::node)
    {
      if (IsAvailable())
      {
        // Start the thread
        thread.Start(ThreadProc, this);

        // Make it above normal
        thread.SetPriority(Win32::Thread::ABOVE_NORMAL);
      }
    }


    //
    // Destructor
    //
    RawSocket::~RawSocket()
    {
      // Signal the quit event
      eventQuit.Signal();

      // Wait for the thread to stop
      thread.Stop();

      // Cleanup the sequence number tree
      sequenceNumbers.DisposeAll();

      // Delete any remaining handles
      handles.DisposeAll();

      // Make sure the ping socket is closed
      pingSocket.Close();
    }

    
    //
    // Is the method available
    //
    Bool RawSocket::IsAvailable()
    {
      return (pingSocket.IsValid());
    }


    //
    // Send a ping
    //
    void RawSocket::Send(Handle &handle)
    {
      // Add the handle to the list of handles
      handleCritSec.Enter();
      handles.Append(&handle);
      handleCritSec.Exit();

      U8 buffer[sizeof (ICMP::Header) + 4];
      U32 size = sizeof (ICMP::Header) + 4;

      U32 ip = handle.address.GetIP();

      Seq *seq = sequenceNumbers.Find(ip);
      if (!seq)
      {
        seq = new Seq;
        sequenceNumbers.Add(ip, seq);
      }

      // Clear the buffer
      Utils::Memset(buffer, 0x00, size);

      // Get in the header
      ICMP::Header *header = reinterpret_cast<ICMP::Header *>(buffer);
      header->type = ICMP::Type::EchoRequest;
      header->code = 0;
      header->checksum = 0;
      header->id = (U16) GetCurrentProcessId();
      header->seq = (U16) seq->out;

      // Increment the out sequence number
      seq->out++;

      // Add the time now
      *(U32 *) (buffer + sizeof (ICMP::Header)) = Clock::Time::Ms();

      // Put in the checksum
      header->checksum = IP::CheckSum(reinterpret_cast<U16 *>(buffer), size);

      // For some reason we can't change TTL when winsock indicates its status as 'On Win95'
      //socket.SetSockOpt(IPPROTO_IP, IP_TTL, (char *) &ttl, sizeof (U32));
      pingSocket.Send(handle.address, buffer, sizeof (buffer));
    }


    //
    // FindHandleByAddress
    //
    Handle * RawSocket::FindHandleByAddress(U32 ip)
    {
      handleCritSec.Enter();
      for (NList<Handle>::Iterator h(&handles); *h; ++h)
      {
        if ((*h)->address.GetIP() == ip)
        {
          Handle *handle = *h;
          handles.Unlink(handle);
          handleCritSec.Exit();
          return (handle);
        }
      }
      handleCritSec.Exit();
      return (NULL);
    }


    //
    // Decode an incoming ping
    //
    Bool RawSocket::Recv(Win32::Socket &socket, Win32::Socket::Address &address, U32 &type, U16 &seq, U32 &rtt, U8 &hops)
    {
      U8 buffer[512];
      U32 recv = socket.Recv(address, buffer, sizeof (buffer));

      // Skip ahead to the ICMP header within the IP packet
      IP::Header *headerIP = (IP::Header *) buffer;
      U32 headerLength = headerIP->h_len * 4;

      ICMP::Header *headerICMP = (ICMP::Header *) (buffer + headerLength);

      // Make sure the reply is sane
      if (recv < headerLength + ICMP::MinimumSize) 
      {
        //LOG_DIAG("Too few bytes from " << address)
        return (FALSE);
      }
      else
      {
        type = headerICMP->type;

        switch (headerICMP->type)
        {
          case ICMP::Type::EchoReply:
            if (headerICMP->id != (U16) GetCurrentProcessId())
            {
              // This is a reply for another process
              return (FALSE);
            }
            break;

          case ICMP::Type::TimeExceeded:
            break;

          case ICMP::Type::DestinationUnreachable:
            return (FALSE);

          default:
            //LDIAG("Unknown ICMP packet type " << int(headerICMP->type) << " received");
            return (FALSE);
        }
      }

      // Grab the sequence number
      seq = headerICMP->seq;

      // Figure out how far the packet travelled
      hops = U8(256 - headerIP->ttl);

      if (hops == 192)
      {
        // Probably from a host on the LAN, make it 1 hop
        hops = 1;
      }
      else if (hops == 128)
      {
        // Probably from local host, make it 1 hops
        hops = 1;
      }
      else if (hops > 128)
      {
        // Probably initial ttl was 128
        hops = U8(128 - headerIP->ttl);
      }
      else if (hops > 64)
      {
        // Probably initial ttl was 64
        hops = U8(64 - headerIP->ttl);
      }

      // Calculate the round trip time
      U32 timestamp = *(U32 *)(buffer + headerLength + sizeof (ICMP::Header));
      rtt = Clock::Time::Ms() - timestamp;

      if (headerICMP->type == ICMP::Type::TimeExceeded) 
      {
        //LDIAG("TTL expired.");
        return (FALSE);
      }
      else 
      {
        //LDIAG("Ping: " << (int) rtt << " Hops: " << (int) hops << " TTL: " << (int) headerIP->ttl)
        return (TRUE);
      }
    }


    //
    // ThreadProc
    //
    U32 STDCALL RawSocket::ThreadProc(void *context)
    {
      RawSocket *rawSocket = static_cast<RawSocket *>(context);

      Win32::EventIndex pingEvent;
      rawSocket->pingSocket.EventSelect(pingEvent, FD_READ);

      Bool quit = FALSE;
      Win32::EventIndex::List<2> events;
      events.AddEvent(pingEvent, NULL);
      events.AddEvent(rawSocket->eventQuit, rawSocket);

      // Enter processing loop
      while (!quit)
      {
        void *context;
        if (events.Wait(context, FALSE))
        {
          if (!context)
          {
            // An incoming ping perhaps
            Win32::Socket::NetworkEvents networkEvents;
            rawSocket->pingSocket.EnumEvents(pingEvent, networkEvents);

            if (networkEvents.GetEvents() & FD_READ)
            {
              Win32::Socket::Address address;
              U32 type;
              U16 seq;
              U32 rtt;
              U8 hops;

              if (Recv(rawSocket->pingSocket, address, type, seq, rtt, hops))
              {
                S32 diff;
                U32 lost = 0;

                Seq *sequence = rawSocket->sequenceNumbers.Find(address.GetIP());

                if (sequence)
                {
                  diff = seq - sequence->in;

                  if (!diff)
                  {
                    // Got the one we were expecting
                  }
                  else if (diff < 0)
                  {
                    // We got an old out of sequence ping, ignore it as we already counted this as a loss
                  }
                  else if (diff > 0)
                  {
                    // We lost a ping or they are returning out of sequence
                    lost = diff;

                    // Move the expeceted sequence to this ping
                    sequence->in = U16(seq);
                  }

                  // Increment the next expected sequence
                  sequence->in++;

                  Handle *handle;

                  while (lost--)
                  {
                    handle = rawSocket->FindHandleByAddress(address.GetIP());
                    if (handle)
                    {
                      Event *e = Ping::events.AddPre();
                      e->handle = handle;
                      e->rtt = U32_MAX;
                      e->hopCount = U32_MAX;
                      Ping::events.AddPost();
                    }
                    else
                    {
                      break;
                    }
                  }

                  handle = rawSocket->FindHandleByAddress(address.GetIP());
                  if (handle)
                  {
                    Event *e = Ping::events.AddPre();
                    e->handle = handle;
                    e->rtt = rtt;
                    e->hopCount = hops;
                    Ping::events.AddPost();
                  }
                }
              }

            }
          }
          else
          {
            quit = TRUE;
          }
        }
      }

      return (TRUE);
    }

  }

}
