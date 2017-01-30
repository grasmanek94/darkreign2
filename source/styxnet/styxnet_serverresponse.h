////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_SERVERRESPONSE_H
#define __STYXNET_SERVERRESPONSE_H


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ServerResponse
  //
  // Server responses are responses to client queries
  //
  namespace ServerResponse
  {
    const CRC UserConnected           = 0xBA412BF5; // "StyxNet::ServerResponse::UserConnected"
    const CRC UserNotInSession        = 0x30DCCD11; // "StyxNet::ServerResponse::UserNotInSession"

    const CRC UserMigrated            = 0xB967C0C6; // "StyxNet::ServerResponse::UserMigrated"
    const CRC UserMigrationFailed     = 0x428AFA16; // "StyxNet::ServerResponse::UserMigrationFailed"
    const CRC UserMigrateNotNeeded    = 0x7B579186; // "StyxNet::ServerResponse::UserMigrateNotNeeded"

    const CRC UserReconnected         = 0x851D97B0; // "StyxNet::ServerResponse::UserReconnected"
    const CRC UserReconnectFailed     = 0x644081B8; // "StyxNet::ServerResponse::UserReconnectFailed"

    const CRC ServerShuttingDown      = 0xE8515181; // "StyxNet::ServerResponse::ServerShuttingDown"

    const CRC SessionCreated          = 0xD87F6B4C; // "StyxNet::ServerResponse::SessionCreated"
    const CRC SessionConnected        = 0x3904DAE9; // "StyxNet::ServerResponse::SessionConnected"
    const CRC SessionClosed           = 0xD9863CC6; // "StyxNet::ServerResponse::SessionClosed"
    const CRC SessionList             = 0x3741DEAC; // "StyxNet::ServerResponse::SessionList"
    const CRC SessionJoined           = 0x15E324E6; // "StyxNet::ServerResponse::SessionJoined"
    
    const CRC SessionAlreadyExists    = 0xFD8B77BC; // "StyxNet::ServerResponse::SessionAlreadyExists"
    const CRC SessionLocalOnly        = 0x4525CCC3; // "StyxNet::ServerResponse::SessionLocalOnly"
    const CRC SessionSingleOnly       = 0x2FAD183C; // "StyxNet::ServerResponse::SessionSingleOnly"
    const CRC SessionHostOnly         = 0x0E6B7C5B; // "StyxNet::ServerResponse::SessionHostOnly"
    const CRC SessionNotFound         = 0x4F76D243; // "StyxNet::ServerResponse::SessionNotFound"
    const CRC SessionBadUser          = 0x9D61099E; // "StyxNet::ServerResponse::SessionBadUser"
    const CRC SessionBadPassword      = 0x29A317F2; // "StyxNet::ServerResponse::SessionBadPassword"
    const CRC SessionFull             = 0xA153E0D8; // "StyxNet::ServerResponse::SessionFull"
    const CRC SessionIsLocked         = 0x68C0A6F3; // "StyxNet::ServerResponse::SessionIsLocked"


    ////////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Data
    //
    namespace Data
    {
      #pragma pack(push, 1)

      struct UserConnected
      {
        U32 secret;         // Secret to be used for reconnecting
      };
      struct SessionList
      {
        U32 numSessions;    // How many session are there
      };
      struct Session
      {
        SessionName name;   // Name of the session
        U32 numUsers;       // Number of users in the session
        U32 maxUsers;       // Maximum number of users
        U8 password : 1;    // Is the session password protected ?
      };

      #pragma pack(pop)
    }

  }

}

#endif