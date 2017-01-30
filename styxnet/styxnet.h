////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_H
#define __STYXNET_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ident.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace SessionFlags
  //
  namespace SessionFlags
  {

    // Does the session require a password ? 
    const U32 Password        = 0x00000001;

    // Is the session locked ?
    const U32 Locked          = 0x00000002;

    // Is the session on a routing server ?
    const U32 RoutingServer   = 0x00000004;

    // Is this a session which is migrating
    const U32 MigratingFrom   = 0x00000008;

    // Is this a session which is being migrated to
    const U32 MigratingTo     = 0x00000010;

  }


  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace SessionFlags
  //
  namespace UserFlags
  {

    // User is logged in
    const U32 LoggedIn          = 0x00000001;

    // User is local
    const U32 Local             = 0x00000002;

    // Was a previous host
    const U32 PreviousHost      = 0x00000004;

    // Can the user accept migration
    const U32 AcceptMigration   = 0x00000010;

    // User is behind an Address translator (as far as we're concerned)
    const U32 BehindNAT         = 0x00000040;

  }


  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ServerFlags
  //
  namespace ServerFlags
  {

    // Is this server standalone
    const U32 StandAlone        = 0x00000001;

    // Is the server shutting down
    const U32 ShuttingDown      = 0x00000002;

  }


  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ServerFlags
  //
  namespace ClientFlags
  {
    // Is the client connected
    const U32 Connected         = 0x00000001;

    // Has this client been disconnected
    const U32 Disconnected      = 0x00000002;

    // Is the client attempting reconnection
    const U32 Reconnecting      = 0x00000004;

    // Is this client attempting migration
    const U32 Migrating         = 0x00000010;

    // Is this client who accepted migration
    const U32 AcceptedMigration = 0x00000020;

  }


  // Default port number
  const U32 defaultPort = 0x6666;

  // The maximum number of users on a server
  const U32 maximumUsers = 256;

  // The minimum setting for the number of users in a session
  const U32 minimumSessionUsers = 2;

  // The maximum setting for the number of users in a session
  const U32 maximumSessionUsers = 32;

  // The default ping interval
  const U32 defaultPingInterval = 6000;

  // The size of a user name
  const U32 maximumUserName = 16;

  // The size of a session name
  const U32 maximumSessionName = 32;

  // Number of old sync data's to be saved
  const U32 maximumOldData = 256;
  const U32 maximumOldDataMask = 0xFF;

  // Maximum amount of time a user can be disconnected before deleting them
  const U32 maximumDisconnectTime = 60000;

  // User and session name typedefs
  typedef StrCrc<maximumUserName> UserName;
  typedef StrCrc<maximumSessionName> SessionName;

  // Initialization and Shutdown
  void Init();
  void Done();

  // Process
  void Process();

}


#endif