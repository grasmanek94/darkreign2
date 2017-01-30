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
#include "styxnet_server_private.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Server::User
  //

  //
  // Constructor
  //
  Server::User::User(const Win32::Socket &s, const Win32::Socket::Address &address)
  : socket(s),
    address(address),
    flags(0),
    secret(0),
    packetBuffer(Packet::Buffer::Create(serverBufferSize)),
    session(NULL)
  {
    socket.EventSelect(event, FD_READ | FD_WRITE | FD_CLOSE);
  }


  //
  // Destructor
  //
  Server::User::~User()
  {
    // Dispose of the packet buffer
    delete packetBuffer;

    // If we're in a session, remove ourselves
    if (session)
    {
      LDIAG("User at " << address << " was in session, removing them")
      session->RemoveUser(*this);
    }

    // Make sure the socket is closed
    socket.Close();
  }

}
