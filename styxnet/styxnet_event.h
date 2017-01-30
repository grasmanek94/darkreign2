////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_EVENT_H
#define __STYXNET_EVENT_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32_socket.h"
#include "styxnet_session.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Struct Event
  //
  struct Event
  {
    struct Data;

    // Message
    CRC message;

    // Data for this message
    Data *data;

  };


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Struct Event::Data
  //
  struct Event::Data
  {
    // Virtual destructor for the data should it require it
    virtual ~Data() { };

  };


  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace EventMessage
  //
  namespace EventMessage
  {
    // Server Specific
    const CRC ServerStarted           = 0xF2473339; // "StyxNet::Event::ServerStarted"
    const CRC ServerUserAdded         = 0x50FC36F7; // "StyxNet::Event::ServerUserAdded"
    const CRC ServerUserRemoved       = 0x12E90345; // "StyxNet::Event::ServerUserRemoved"
    const CRC ServerNoSessions        = 0xCE420A29; // "StyxNet::Event::ServerNoSessions"
    const CRC ServerMigrateFailed     = 0x09F3954C; // "StyxNet::Event::ServerMigrateFailed"
                                      
    const CRC SessionAdded            = 0x4418DFBF; // "StyxNet::Event::SessionAdded"
    const CRC SessionRemoved          = 0x7879E95A; // "StyxNet::Event::SessionRemoved"
    const CRC SessionModified         = 0xFF4890D5; // "StyxNet::Event::SessionModified"
                                      
    // Client Specific                
    const CRC ServerConnectFailed     = 0x662F80D6; // "StyxNet::Event::ServerConnectFailed"
    const CRC ServerConnected         = 0xFEB71D20; // "StyxNet::Event::ServerConnected"
    const CRC ServerDisconnected      = 0x1A3A0F18; // "StyxNet::Event::ServerDisconnected"
    const CRC ServerReconnectFailed   = 0xBD3D5A17; // "StyxNet::Event::ServerReconnectFailed"
    const CRC ServerReconnected       = 0x24946AEE; // "StyxNet::Event::ServerReconnected"
                                      
    const CRC SessionCreated          = 0x5D983C06; // "StyxNet::Event::SessionCreated"
    const CRC SessionConnected        = 0xA0119478; // "StyxNet::Event::SessionConnected"
    const CRC SessionAlreadyExists    = 0x9DCA05C5; // "StyxNet::Event::SessionAlreadyExists"
                                      
    const CRC SessionJoined           = 0xD80A84A7; // "StyxNet::Event::SessionJoined"
    const CRC SessionBadUser          = 0x18865ED4; // "StyxNet::Event::SessionBadUser"
    const CRC SessionBadPassword      = 0xF66A66EC; // "StyxNet::Event::SessionBadPassword"
    const CRC SessionFull             = 0xF045F8A8; // "StyxNet::Event::SessionFull"
    const CRC SessionIsLocked         = 0xF15E6776; // "StyxNet::Event::SessionIsLocked"
                                      
    const CRC SessionClosed           = 0x146F9C87; // "StyxNet::Event::SessionClosed"
    const CRC SessionKicked           = 0x32866B0E; // "StyxNet::Event::SessionKicked"
                                      
    const CRC SessionLocalUserAdded   = 0xAC2BD3BD; // "StyxNet::Event::SessionLocalUserAdded"
    const CRC SessionUserAdded        = 0x0E5ABFAF; // "StyxNet::Event::SessionUserAdded"
    const CRC SessionUserRemoved      = 0x4E911352; // "StyxNet::Event::SessionUserRemoved"
    const CRC SessionUserDisconnected = 0x42C05C46; // "StyxNet::Event::SessionUserDisconnected"
    const CRC SessionUserReconnected  = 0xF5D9CAEB; // "StyxNet::Event::SessionUserReconnected"
                                      
    const CRC SessionInfo             = 0x23F23C8C; // "StyxNet::Event::SessionInfo"

    const CRC SessionMigrateRequest   = 0xD5884D12; // "StyxNet::Event::SessionMigrateRequest"
    const CRC SessionMigrateComplete  = 0xC9EC5E36; // "StyxNet::Event::SessionMigrateComplete"
    const CRC SessionMigrateFailed    = 0x4123F0B7; // "StyxNet::Event::SessionMigrateFailed"
    const CRC SessionMigrateNotNeeded = 0xDECB42A3; // "StyxNet::Event::SessionMigrateNotNeeded"

    const CRC SessionData             = 0xFD63F48F; // "StyxNet::Event::SessionData"
    const CRC SessionPrivateData      = 0x7283F4C2; // "StyxNet::Event::SessionPrivateData"
    const CRC SessionSyncData         = 0x41E8635F; // "StyxNet::Event::SessionSyncData"

    const CRC SyncData                = 0x5FF1444A; // "StyxNet::Event::SyncData"
    const CRC SyncStore               = 0xE72AF205; // "StyxNet::Event::SyncStore"
    const CRC SyncClear               = 0x579E3B23; // "StyxNet::Event::SyncClear"
    const CRC SyncFlush               = 0xF0D51FE7; // "StyxNet::Event::SyncFlush"
    const CRC SyncMigrate             = 0x8DC9879E; // "StyxNet::Event::SyncMigrate"

    const CRC Ping                    = 0x7512DD57; // "StyxNet::Event::Ping"


    ////////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Data
    //
    namespace Data
    {

      struct ServerUserAdded : public Event::Data
      {
        // Name of the user
        UserName name;

        // Address of the user
        Win32::Socket::Address address;

        // Constructor
        ServerUserAdded(const UserName &name, const Win32::Socket::Address &address)
        : name(name),
          address(address)
        {
        }

      };

      struct ServerUserRemoved : public Event::Data
      {
        // Name of the user
        UserName name;

        ServerUserRemoved(const UserName &name)
        : name(name)
        {
        }

      };

      struct SessionAdded : public Event::Data
      {
        // Name of the session
        SessionName name;

        SessionAdded(const SessionName &name)
        : name(name)
        {
        }

      };

      struct SessionRemoved : public Event::Data
      {
        // Name of the session
        SessionName name;

        SessionRemoved(const SessionName &name)
        : name(name)
        {
        }

      };

      struct SessionModified : public Event::Data
      {
        // Name of the session
        SessionName name;

        // Name of the host of the session
        UserName host;

        // Number of users in the session
        U32 users;

        // Maximum number of users in the session
        U32 maxUsers;

        SessionModified(const SessionName &name, const UserName &host, U32 users, U32 maxUsers)
        : name(name),
          host(host),
          users(users),
          maxUsers(maxUsers)
        {
        }

      };

      struct SessionUserAdded : public Event::Data
      {
        // Name of the user
        UserName name;

        // Constructor
        SessionUserAdded(const UserName &name)
        : name(name)
        {
        }

      };

      typedef SessionUserAdded SessionUserRemoved;
      typedef SessionUserAdded SessionUserDisconnected;
      typedef SessionUserAdded SessionUserReconnected;

      struct SessionInfo : public Event::Data, StyxNet::SessionData
      {
        // Constructor
        SessionInfo(const StyxNet::SessionData &sessionData)
        : SessionData(sessionData)
        {
        }

      };

      struct SessionMigrateRequest : public Event::Data
      {
        // Sequence number new session should start at
        U32 seq;

        // Constructor
        SessionMigrateRequest(U32 seq)
        : seq(seq)
        {
        }

      };

      struct SessionData : public Event::Data
      {
        // Who the data is from
        CRC from;

        // Data key
        CRC key;

        // Length of the data
        U32 length;

        // The data
        U8 *data;

        // Destructor
        ~SessionData()
        {
          if (data)
          {
            delete [] data;
          }
        }

      };

      typedef SessionData SessionPrivateData;

      struct SessionSyncData : public Event::Data
      {
        // Sequence number
        U32 seq;

        // Time the update arrived
        U32 time;

        // Time to next update
        U32 interval;

        // Length of the data
        U32 length;

        // The data chunk
        U8 *data;

        // Destructor
        ~SessionSyncData()
        {
          if (data)
          {
            delete [] data;
          }
        }

      };
  
      struct Ping : public Event::Data
      {
        // Address ping is from
        Win32::Socket::Address address;

        // Ping
        U32 rtt;

        // Number of hops
        U32 hopCount;

        // Constructor
        Ping(const Win32::Socket::Address &address, U32 rtt, U32 hopCount)
        : address(address),
          rtt(rtt),
          hopCount(hopCount)
        {
        }

      };

    }

  }

}

#endif