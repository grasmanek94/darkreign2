////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_CLIENTRESPONSE_H
#define __STYXNET_CLIENTRESPONSE_H


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ClientResponse
  //
  // Client Responses are from the server to the client
  //
  namespace ClientResponse
  {
    const CRC SessionRequestMigrateAccept = 0xECEAEF2B; // "StyxNet::ClientResponse::SessionRequestMigrateAccept"
    const CRC SessionRequestMigrateDeny   = 0xC2ADBA45; // "StyxNet::ClientResponse::SessionRequestMigrateDeny"


    ////////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Data
    //
    namespace Data
    {
      #pragma pack(push, 1)

      struct SessionRequestMigrateAccept
      {
        Win32::Socket::Address address; // Address of the new server
        U32 key;                        // Key for clients to use to make the migration
      };

      #pragma pack(pop)
    }

  }

}

#endif