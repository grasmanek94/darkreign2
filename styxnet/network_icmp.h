////////////////////////////////////////////////////////////////////////////////
//
// Network ICMP
//
// Copyright 1999-2000
// Matthew Versluys
//


#ifndef __NETWORK_ICMP_H
#define __NETWORK_ICMP_H


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace Network
//
namespace Network
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ICMP
  //
  namespace ICMP
  {

    ////////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Type
    //
    namespace Type
    {
      const U32 EchoReply = 0;
      const U32 DestinationUnreachable = 3;
      const U32 SourceQuench = 4;
      const U32 Redirect = 5;
      const U32 EchoRequest = 8;
      const U32 RouterAdvertisement = 9;
      const U32 RouterSolicitation = 10;
      const U32 TimeExceeded = 11;
      const U32 ParameterProblem = 12;
      const U32 TimestampRequest = 13;
      const U32 TimestampReply = 14;
      const U32 InformationRequest = 15;
      const U32 InformationReply = 16;
      const U32 AddressMaskRequest = 17;
      const U32 AddressMaskReply = 18;
    }

    namespace DestinationUnreachable
    {
      const U32 NetworkUnreachable = 0;
      const U32 HostUnreachable = 1;
      const U32 ProtocolUnreachable = 2;
      const U32 PortUnreachable = 3;
      const U32 FragmentationNeeded = 4;
      const U32 SourceRouteFailed = 5;
      const U32 DestinationNetworkUnknown = 6;
      const U32 DestinationHostUnknown = 7;
      const U32 SourceHostIsoloated = 8;
      const U32 DestinationNetworkProhibited = 9;
      const U32 DestinationHostProhibited = 10;
      const U32 NetworkUnreachableForTOS = 11;
      const U32 HostUnreachableForTOS = 12;
      const U32 CommunicationFiltered = 13;
      const U32 HostPrecedenceViolation = 14;
      const U32 PrecendenceCutoffInEffect = 15;
    }

    namespace Redirect
    {
      const U32 Network = 0;
      const U32 Host = 1;
      const U32 NetworkTOS = 2;
      const U32 HostTOS = 3;
    }

    namespace TimeExceeded
    {
      const U32 Transit = 0;
      const U32 Reassembly = 1;
    }

    namespace ParameterProblem
    {
      const U32 BadHeader = 0;
      const U32 MissingOption = 1;
    }


    //
    // Minimum ICMP packet size, in bytes
    //
    const U32 MinimumSize = 8;

    #pragma pack (push, 1)

    //
    // Header
    //
    struct Header 
    {
      // ICMP packet type
      U8 type;

      // Type sub code
      U8 code;

      // Checksum
      U16 checksum;

      // Id (set to pid)
      U16 id;

      // Sequence number
      U16 seq;

    };

    #pragma pack(pop)

  }

}


#endif