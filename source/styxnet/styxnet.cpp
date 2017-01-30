////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_private.h"
#include "win32_socket.h"
#include "win32_dns.h"
#include "logging.h"
#include "logging_destdr2.h"
#include "network_ping.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  static Bool initialized = FALSE;
  static Win32::Mutex clientMutex;
  static Win32::Mutex serverMutex;
  static U32 numServers;
  static U32 numClients;


  //
  // Initialization
  //
  void Init()
  {
    ASSERT(!initialized)

    // Initialize logging
    Logging::Init();

    // We want to send logs to DR2 and to the debugger
    LOG_ADDDEST(new Logging::DestDR2())

    // Initalize Sockets
    Win32::Socket::Init();
    Win32::DNS::Init();

    // Initialize Ping System
    Network::Ping::Init();

    // Clear the number of clients and servers
    numClients = 0;
    numServers = 0;

    // Set initialized flag
    initialized = TRUE;
  }


  //
  // Shutdown
  //
  void Done()
  {
    ASSERT(initialized)

    // Wait for all clients/servers to shutdown
    Bool finished = FALSE;
    while (!finished)
    {
      finished = TRUE;

      clientMutex.Wait();
      if (numClients)
      {
        finished = FALSE;
      }
      clientMutex.Signal();

      serverMutex.Wait();
      if (numServers)
      {
        finished = FALSE;
      }
      serverMutex.Signal();

      // Release our timeslice and let other threads process
      Sleep(0);
    }

    // Shutdown Ping System
    Network::Ping::Done();

    // Shutdown sockets
    Win32::DNS::Done();
    Win32::Socket::Done();

    // Shutdown logging
    Logging::Done();

    // Clear initialized flag
    initialized = FALSE;
  }


  //
  // Process
  //
  void Process()
  {
    ASSERT(initialized)

    // Process ping system
    Network::Ping::Process();
  }


  //
  // Add client
  //
  void AddClient()
  {
    clientMutex.Wait();
    numClients++;
    clientMutex.Signal();
  }


  //
  // Remove client
  //
  void RemoveClient()
  {
    clientMutex.Wait();
    numClients--;
    clientMutex.Signal();
  }


  //
  // Add server
  //
  void AddServer()
  {
    serverMutex.Wait();
    numServers++;
    serverMutex.Signal();
  }


  //
  // Remove server
  //
  void RemoveServer()
  {
    serverMutex.Wait();
    numServers--;
    serverMutex.Signal();
  }

}

