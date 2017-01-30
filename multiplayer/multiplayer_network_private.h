///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//

#ifndef __MULTIPLAYER_NETWORK_PRIVATE_H
#define __MULTIPLAYER_NETWORK_PRIVATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_network.h"
#include "styxnet.h"
#include "styxnet_client.h"
#include "styxnet_server.h"
#include "styxnet_explorer.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Network
  //
  namespace Network
  {

    // The current client
    extern StyxNet::Client *client;

    // The current server
    extern StyxNet::Server *server;

    // What's our address ?
    Bool GetLocalAddress(Win32::Socket::Address &address, CRC who);
  }
  
}


#endif
