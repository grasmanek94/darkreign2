////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_CLIENT_H
#define __STYXNET_CLIENT_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_private.h"
#include "styxnet_data.h"
#include "styxnet_packet.h"
#include "styxnet_event.h"
#include "styxnet_eventqueue.h"
#include "win32_thread.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Client
  //
  class Client : public EventQueue
  {
  public:

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Struct Config
    //
    struct Config
    {
      // Prefered user name when connecting to servers
      UserName userName;

      // Address of the server to connect to
      Win32::Socket::Address address;

      // Can we become the host ?
      Bool migratable;

      // Interval (in milliseconds) to send pings
      U32 pingInterval;

      // Constructor
      Config(const UserName &userName, const Win32::Socket::Address &address, Bool migratable, U32 pingInterval = defaultPingInterval)
      : userName(userName),
        migratable(migratable),
        address(address),
        pingInterval(pingInterval)
      {
      }

    };

    class Session;
    class User;

  private:

    // Client configuration
    Config config;

    // Flags
    U32 flags;

    // Secret for reconnecting
    U32 secret;

    // Last sync data sequence we received
    U32 sequence;

    // Client Thread
    Win32::Thread thread;

    // Event to stop the client
    Win32::EventIndex eventQuit;

    // Address of the server
    Win32::Socket::Address address;

    // Socket which connects our client to the server
    Win32::Socket socket;

    // Event handle for the above socket
    Win32::EventIndex event;

    // Buffer for receiving data
    Packet::Buffer *packetBuffer;
    
    // The session we're in
    Session *session;

    // Local Connetion information
    Std::Data::UserConnection connection;

    // Migration key
    U32 migrationKey;

    // Index
    U32 index;

  public:

    // Constructor
    Client(const Config &config);

    // Destructor
    ~Client();


    // Create a session
    void CreateSession(const SessionName &name, CRC password, U32 maxUsers);

    // Connect to a session
    void ConnectSession(const SessionName &name, CRC password, U32 maxUsers);

    // Join a session
    void JoinSession(const SessionName &name, CRC password);


    // Send data to the session
    void SendData(CRC key, U32 length, const U8 *data, Bool sync);

    // Send data to a private recipient list .. zero recipients is special, sends to the host
    void SendData(U32 numRecipients, const CRC *recipients, CRC key, U32 length, const U8 *data);


    // Store synchronous data
    void StoreData(CRC key, CRC index, U32 length, const U8 *data);

    // Clear synchronous data
    void ClearData(CRC key, CRC index);

    // Flush all synchronous data
    void FlushData();

    // Get the name of the session
    const char * GetSessionName();

    // Lock the session
    void LockSession();

    // Unlock the session
    void UnlockSession();

    // Kick a user
    void KickUser(CRC kick);

    // Get connection information for the given user
    Std::Data::UserConnection * GetUserConnection(CRC user);

    // Get the local address
    Bool GetLocalAddress(Win32::Socket::Address &address);


    // Migrate this session to someone else
    void MigrateSession();

    // Migration is completed
    void MigrationComplete();

    // Accept migration
    void AcceptMigration(U16 port, U32 key);

    // Deny migration
    void DenyMigration();


    // Shutdown the client
    void Shutdown();

    // Bogus data
    void Bogus();

    // Migrate the session to a new host
    void MigrateSession(const U8 *data, U32 length);

  public:

    // ExtractSyncData
    static Bool ExtractSyncData(const U8 *&ptr, U32 &remaining, CRC &type, CRC &from, CRC &key, CRC &index, U32 &length, const U8 *&data);

  public:

    // Get the session
    const Session * GetSession() const
    {
      return (session);
    }

    // Get client socket statistics
    const Win32::Socket::Statistics & GetStatistics()
    {
      return (socket.GetStats());
    }

  private:

    // Thread procedure
    static U32 STDCALL ThreadProc(void *context);

    // Ping callback
    static void PingCallback(const Win32::Socket::Address &address, U32 rtt, U32 hopCount, void *context);

    // Handle an incomming packet
    void ProcessPacket(const Packet &packet);

    // Process ping information
    void Ping(U16 ping, U8 hops);

        
  };

}

#endif