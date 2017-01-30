////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_SERVER_H
#define __STYXNET_SERVER_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_private.h"
#include "styxnet_packet.h"
#include "styxnet_eventqueue.h"
#include "win32_thread.h"
#include "win32_event.h"
#include "win32_mutex.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Server
  //
  class Server : public EventQueue
  {
  public:

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Struct Config
    //
    struct Config
    {
      // Port the listen on
      U16 port;

      // Update interval (ms)
      U32 updateInterval;

      enum Filter
      {
        FILTER_NONE,
        FILTER_ALLOW,
        FILTER_DENY
      };

      // Filter type
      Filter filter;

      // Addresses to filter
      List<Win32::Socket::Address> addresses;

      // Constructor
      Config()
      : port(defaultPort),
        updateInterval(defaultUpdateInterval),
        filter(FILTER_NONE)
      {
      }

      ~Config()
      {
        addresses.UnlinkAll();
      }

    };

  private:

    class Item;
    class User;
    class Session;
    class Migration;

    // Server configuration
    Config config;

    // Server flags
    U32 flags;

    // Users connected to this server
    NList<User> users;

    // Users who have been disconnected
    NList<User> disconnected;

    // Sessions which are being hosted on this server
    NBinTree<Session, CRC> sessions;

    // Migrations
    NList<Migration> migrations;

    // Mutex for accesses to the user list
    Win32::Mutex mutexUsers;

    // Mutex for accesses to the session list
    Win32::Mutex mutexSessions;

    // Server Thread
    Win32::Thread thread;

    // Event to stop the server
    Win32::EventIndex eventQuit;

  public:

    // Constructor
    Server(const Config &config, Bool standalone = FALSE);

    // Destructor
    ~Server();

    // Get the local address
    Bool GetLocalAddress(Win32::Socket::Address &address, U32 who);

    // Shutdown the server
    void Shutdown();

    // Setup a session for migration
    void SetupMigration(const SessionName &name, U32 maxUsers, U32 seq, U32 &key);

  private:

    // Thread procedure
    static U32 STDCALL ThreadProc(void *);

    // Handle an incomming packet
    Bool ProcessPacket(User &user, const Packet &packet);

    friend Session;
    friend User;
    friend Migration;

  };

}

#endif
