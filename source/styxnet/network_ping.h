////////////////////////////////////////////////////////////////////////////////
//
// Network Ping
//
// Copyright 1999-2000
// Matthew Versluys
//
// Try's to PING using several APIs
// - IPHelper interface
// - ICMP interface
// - RAW socket
//


#ifndef __NETWORK_PING_H
#define __NETWORK_PING_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32_socket.h"


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

    // Callback
    typedef void (*Callback)(const Win32::Socket::Address &address, U32 rtt, U32 hopCount, void *context);

    // Initialization and Shutdown
    void Init();
    void Done();

    // Send a ping
    void Send(const Win32::Socket::Address &address, Callback callback, void *context);

    // Process any pings (done so that callbacks don't need to be thread safe)
    void Process();

  }
}

#endif

