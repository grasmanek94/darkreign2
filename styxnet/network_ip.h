////////////////////////////////////////////////////////////////////////////////
//
// Network IP
//
// Copyright 1999-2000
// Matthew Versluys
//


#ifndef __NETWORK_IP_H
#define __NETWORK_IP_H


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace Network
//
namespace Network
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace IP
  //
  namespace IP
  {

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Struct Header
    //
    struct Header 
    {
      // Length of the header in dwords
      U8 h_len : 4;

      // Version of IP
      U8 version : 4;

      // Type of service
      U8 tos;
      
      // Length of the packet in dwords
      U16 total_len;

      // unique identifier
      U16 ident;

      // Flags
      U16 flags;

      // Time to live
      U8 ttl;

      // Protocol number (TCP, UDP etc)
      U8 proto;

      // IP checksum
      U16 checksum;

      // Source IP
      U32 source_ip;

      // Dest IP
      U32 dest_ip;

    };

    // Compute an IP checksum
    U16 CheckSum(U16 *buffer, U32 size);

  }

}

#endif