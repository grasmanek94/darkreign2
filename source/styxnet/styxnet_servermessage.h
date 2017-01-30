////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_SERVERMESSAGE_H
#define __STYXNET_SERVERMESSAGE_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_private.h"
#include "styxnet_session.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ServerMessage
  //
  // Server Messages are from the server to the client
  //
  namespace ServerMessage
  {
    const CRC ServerShutdown          = 0xE07C2179; // "StyxNet::ServerMessage::ServerShutdown"

    const CRC SessionUserAdded        = 0x019908C2; // "StyxNet::ServerMessage::SessionUserAdded"
    const CRC SessionUserRemoved      = 0x14B2983A; // "StyxNet::ServerMessage::SessionUserRemoved"
    const CRC SessionUserDisconnected = 0x95E2A69E; // "StyxNet::ServerMessage::SessionUserDisconnected"
    const CRC SessionUserReconnected  = 0x5FD04D91; // "StyxNet::ServerMessage::SessionUserReconnected"

    const CRC SessionInfo             = 0xCDABE443; // "StyxNet::ServerMessage::SessionInfo"

    const CRC SessionData             = 0x133A2C40; // "StyxNet::ServerMessage::SessionData"
    const CRC SessionPrivateData      = 0x28A07FAA; // "StyxNet::ServerMessage::SessionPrivateData"
    const CRC SessionSyncData         = 0x25FA7CF3; // "StyxNet::ServerMessage::SyncData"

    const CRC SessionRequestMigrate   = 0x8B0FFDFE; // "StyxNet::ServerMessage::SessionRequestMigrate"
    const CRC SessionMigrate          = 0x4981F300; // "StyxNet::ServerMessage::SessionMigrate"

    const CRC SessionNowHost          = 0x86B7C0C8; // "StyxNet::ServerMessage::SessionNowHost"

    const CRC SessionKicked           = 0x3CE04E00; // "StyxNet::ServerMessage::SessionKicked"


    ////////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Data
    //
    namespace Data
    {
      #pragma pack(push, 1)

      struct SessionUserAdded
      {
        UserName who;                   // Name of the user who was added
      };
      struct SessionUserRemoved
      {
        CRC who;                        // User which was removed
      };
      struct SessionUserDisconnected
      {
        CRC who;                        // User who has been disconnected
      };
      struct SessionUserReconnected
      {
        CRC who;                        // User who has been reconnected
      };
      struct SessionInfo : public SessionData
      {
      };
      struct SessionData
      {
        CRC from;                       // Who the data is from
        CRC key;                        // Data key
        U8 data[];                      // The data
      };
      typedef SessionData SessionPrivateData;
      struct SessionSyncData
      {
        U32 seq;                        // Sequence number
        U32 interval;                   // Time to next update
        U8 data[];                      // The data
      };
      struct SessionSyncDataData
      {
        CRC command;                    // EventMessage::SyncData
        CRC from;                       // Who the data is from
        CRC key;                        // Data key
        U16 length;                     // Length of the data
        U8 data[];                      // The data
      };
      struct SessionSyncDataStoreData
      {
        CRC command;                    // EventMessage::SyncStore
        CRC from;                       // Who the data is from
        CRC key;                        // Data key
        CRC index;                      // Index key
        U16 length;                     // Length of the data
        U8 data[];                      // The data
      };
      struct SessionSyncDataClearData
      {
        CRC command;                    // EventMessage::SyncClear
        CRC from;                       // Who is clearing the data
        CRC key;                        // Data key
        CRC index;                      // Index key
      };
      struct SessionSyncDataFlushData
      {
        CRC command;                    // EventMessage::SyncFlush
        CRC from;                       // Who is flushing the data
      };
      struct SessionSyncDataMigrate
      {
        CRC command;                    // EventMessage::SyncMigrate
        Win32::Socket::Address address; // Address of the new host
        U32 key;                        // Key to gain entry to the new session
      };
      struct SessionRequestMigrate
      {
        CRC session;                    // Session identifier
        U32 seq;                        // Sequence number new session should start at
      };
      struct SessionMigrate
      {
        Win32::Socket::Address address; // Address of new host
        U32 key;                        // Key to gain entry to the new session
      };

      #pragma pack(pop)
    }

  }

}

#endif