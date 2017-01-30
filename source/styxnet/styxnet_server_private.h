////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_SERVER_PRIVATE_H
#define __STYXNET_SERVER_PRIVATE_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_server.h"
#include "styxnet_data.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Server::Item
  //
  class Server::Item
  {
  private:

    // The user the packet was from
    CRC user;

    // The packet
    Packet &packet;

    // Server node
    NList<Item>::Node nodeSession;

  public:

    // Constructor
    Item(CRC user, Packet &packet);

    // Destructor
    ~Item();

  public:

    friend class Server::Session;

  };


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Server::User
  //
  class Server::User
  {
  private:

    // Name of the user
    UserName name;

    // Socket which connects this user to the server
    Win32::Socket socket;

    // Event handle for the above socket
    Win32::EventIndex event;

    // Address of the user acording to them
    Win32::Socket::Address remoteAddress;

    // Address of the user
    Win32::Socket::Address address;

    // User flags
    U32 flags;

    // User secret
    U32 secret;

    // Time they disconnected
    U32 disconnectTime;

    // The session this user is currently in
    Session *session;

    // Server Node
    NList<User>::Node nodeServer;

    // Session Node
    NBinTree<User, CRC>::Node nodeSession;
    NBinTree<User, U32>::Node nodeMigration;

    // Buffer for receiving data
    Packet::Buffer *packetBuffer;

    // Connection information for this user
    Std::Data::UserConnection connection;

  public:

    // Constructor
    User(const Win32::Socket &socket, const Win32::Socket::Address &address);

    // Destructor
    ~User();

  public:

    const UserName & GetName()
    {
      return (name);
    }

  public:

    friend class Server;
    friend class Session;
    friend class Migration;

  };


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Server::Session
  //
  class Server::Session : public SessionData
  {
  private:

    // Server
    Server &server;

    // Password for the session
    CRC password;

    // Sequence number for outgoing packets
    U32 sequenceNumber;

    // Users which are in this session
    NBinTree<User, CRC> users;

    // Queued items to be processed
    NList<Item> items;

    // User who is the host of this session
    User *host;

    // Circular buffer of last 'maximumOldData'
    Packet *oldPkts[maximumOldData];

    // Indexes into the circular buffer
    U32 oldPktsIndex;

    // Session data
    Data data;

    // Migration
    Migration *migration;

    // Server node
    NBinTree<Session, CRC>::Node nodeServer;

  public:

    // Constructor and destructor
    Session(Server &server, User *host, const SessionName &name, CRC password, U32 maxUsers);
    ~Session();

    // Add User
    void AddUser(User &user);

    // Remove User
    void RemoveUser(User &user);

    // Process the session, return TRUE to terminate the session
    Bool Process();

    // Migrate this session to another user
    void Migrate();

    // This session has finished migrating
    void Migrated();

    // Send a packet to all users in the session
    void SendToAll(Packet &pkt);

    // Send information about this session to everyone inside it
    void SendInfo();

    // Send information about this session
    void SendInfo(Win32::Socket &socket);

    // Send information on the socket to the address about this session
    void SendInfo(Win32::Socket &socket, const Win32::Socket::Address &address);

    // Add old packet
    void AddOldPkt(Packet &pkt);

  public:

    // Get the users
    const NBinTree<User, CRC> & GetUsers()
    {
      return (users);
    }

  protected:

    // Get the next index
    U32 GetNextIndex(U32 index)
    {
      return ((index + 1) & maximumOldDataMask);
    }

    // Get an old index
    U32 GetOldIndex(U32 index)
    {
      return ((oldPktsIndex - index) & maximumOldDataMask);
    }

  public:

    friend class Server;
    friend class User;
    friend class Migration;

  };


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Server::Migration
  //
  class Server::Migration
  {
  private:

    // Session which we are migrating
    Session &session;

    // Users sorted by migration suitability
    NBinTree<User, U32> users;

    // User we're currently offering to
    User *offer;

    // Server node
    NList<Migration>::Node nodeServer;

  public:

    // Constructor
    Migration(Session &session);

    // Destructor
    ~Migration();

    // Offer migration to a user
    void OfferMigration();

    // Remove User
    void RemoveUser(User &user);

    // User accepted migration request
    void RequestAccepted(const Win32::Socket::Address &address, U32 key);

    // User denied migration request
    void RequestDenied();

    // Process migration, return TRUE to terminate the migration
    Bool Process();

  public:

    friend class Server;

  };

}

#endif
