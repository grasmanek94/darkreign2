////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_CLIENTMESSAGE_H
#define __STYXNET_CLIENTMESSAGE_H


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ClientMessage
  //
  // Client Messages are from the client to the server
  //
  namespace ClientMessage
  {
    const CRC UserLogin             = 0xCB97D3F2; // "StyxNet::ClientMessage::UserLogin"
    const CRC UserLogout            = 0x4F74EE08; // "StyxNet::ClientMessage::UserLogout"
    const CRC UserMigrating         = 0x9AD9C62F; // "StyxNet::ClientMessage::UserMigrating"
    const CRC UserReconnect         = 0x19EE1C6E; // "StyxNet::ClientMessage::UserReconnect"

    const CRC SessionCreate         = 0x76F7BF7E; // "StyxNet::ClientMessage::SessionCreate"
    const CRC SessionConnect        = 0xD1518EEB; // "StyxNet::ClientMessage::SessionConnect"
    const CRC SessionDestroy        = 0x2922E1A2; // "StyxNet::ClientMessage::SessionDestroy"
    const CRC SessionList           = 0x5CF4AFF7; // "StyxNet::ClientMessage::SessionList"
    const CRC SessionJoin           = 0xC1B86BD1; // "StyxNet::ClientMessage::SessionJoin"

    const CRC SessionLock           = 0x3C28B93B; // "StyxNet::ClientMessage::SessionLock"
    const CRC SessionUnlock         = 0x4C081824; // "StyxNet::ClientMessage::SessionUnlock"

    const CRC SessionKick           = 0x804C6339; // "StyxNet::ClientMessage::SessionKickUser"

    const CRC SessionData           = 0xC7C09DA4; // "StyxNet::ClientMessage::SessionData"
    const CRC SessionPrivateData    = 0x2F8CF088; // "StyxNet::ClientMessage::SessionPrivateData"
    const CRC SessionSyncData       = 0x46133B52; // "StyxNet::ClientMessage::SessionSyncData"

    const CRC SessionStoreData      = 0x56342829; // "StyxNet::ClientMessage::SessionStoreData"
    const CRC SessionClearData      = 0x39BA1A62; // "StyxNet::ClientMessage::SessionClearData"
    const CRC SessionFlushData      = 0x38B9DE5E; // "StyxNet::ClientMessage::SessionFlushData"

    const CRC SessionMigrate        = 0x4D69BF59; // "StyxNet::ClientMessage::SessionMigrate"
    const CRC SessionMigrated       = 0x62A25FA4; // "StyxNet::ClientMessage::SessionMigrated"


    ////////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Data
    //
    namespace Data
    {
      #pragma pack(push, 1)

      struct UserLogin
      {
        U32 version;                    // Version number of this client
        Win32::Socket::Address address; // Address the user thinks they are connecting to us with
        UserName name;                  // Name they are logging in with
        Bool migratable;                // Can sessions be migrated to this user
      };

      struct UserMigrating
      {
        U32 migrationKey;               // Migration key used to gain entry on the new server
      };

      struct UserReconnect
      {
        U32 secret;                     // Secret for reconnecting
        U32 sequence;                   // Last sequence number received
      };

      struct SessionCreate
      {
        U32 maxUsers;                   // Maximum number of users allowed in the session
        SessionName name;               // Name of the session
        CRC password;                   // Password for the session
      };
      typedef SessionCreate SessionConnect;
      struct SessionDestroy
      {
        CRC name;                       // Name of the session
      };
      struct SessionJoin
      {
        CRC name;                       // Name of the session to join
        CRC password;                   // Password to use to gain entry to the session
      };

      struct SessionKick
      {
        CRC user;                       // Name of the user to kick
      };
      struct SessionPrivateData
      {
        CRC key;                        // Data key
        U32 numRecipients;              // Number of recipients
        CRC recipients[];               // The recipients
      };
      struct SessionData
      {
        CRC key;                        // Data Key
        U8 data[];                      // The data
      };
      typedef SessionData SessionSyncData;

      struct SessionStoreData
      {
        CRC key;                        // Data Key
        CRC index;                      // Index Key
        U8 data[];                      // The data
      };
      struct SessionClearData
      {
        CRC key;                        // Data Key
        CRC index;                      // Index Key
      };

      #pragma pack(pop)
    }

  }

}

#endif