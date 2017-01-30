////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_CLIENT_PRIVATE_H
#define __STYXNET_CLIENT_PRIVATE_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_client.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Client::User
  //
  class Client::User
  {
  private:

    // Name of the user
    UserName name;

    // User connection information
    Std::Data::UserConnection connection;

    // Session Node
    NBinTree<User, CRC>::Node nodeSession;

  public:

    // Constructor
    User(const UserName &name);

  public:

    // Get the name of the user
    const UserName & GetName() const
    {
      return (name);
    }

    friend class Session;
    friend class Client;

  };


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Client::Session
  //
  class Client::Session : public SessionData
  {
  private:

    // Users which are in this session
    NBinTree<User, CRC> users;

    // User who is the host of this session
    User *host;

    // Session data
    Data data;

  public:

    // Constructor and Destructor
    Session();
    ~Session();

    // Add a user to the session
    void AddUser(const UserName &user);

    // Remove a user from the session
    void RemoveUser(CRC user);

    // Find a user in the session
    User * FindUser(CRC user);

  public:

    friend class Client;

  };

}

#endif