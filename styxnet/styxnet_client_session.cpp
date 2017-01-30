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
#include "styxnet_client_private.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Client::Session
  //

  //
  // Constructor
  //
  Client::Session::Session()
  : users(&User::nodeSession)
  {
  }


  //
  // Destructor
  //
  Client::Session::~Session()
  {
    users.DisposeAll();
  }


  //
  // Add a user to the session
  //
  void Client::Session::AddUser(const UserName &name)
  {
    ASSERT(!users.Find(name.crc))
    users.Add(name.crc, new User(name));
    ++numUsers;
  }


  //
  // Remove a user from the session
  //
  void Client::Session::RemoveUser(CRC name)
  {
    User *user = users.Find(name);

    if (user)
    {
      users.Dispose(user);
      --numUsers;
    }
    else
    {
      LWARN("Instructed to remove user " << HEX(name, 8) << " but they could not be found")
    }
  }


  //
  // Find a user in the session
  //
  Client::User * Client::Session::FindUser(CRC name)
  {
    return (users.Find(name));
  }

}
