////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_STD_H
#define __STYXNET_STD_H


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
// 
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Std
  //
  // Standard Data Keys
  //
  namespace Std
  {
    const CRC UserConnection = 0x7E675085; // "StyxNet::DataKey::UserConnection"
    const CRC UserPing       = 0x9FF6AA50; // "StyxNet::DataKey::Ping"


    ////////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Data
    //
    namespace Data
    {
      // How many pings to save
      const U32 maxPings = 75;

      #pragma pack(push, 1)


      ////////////////////////////////////////////////////////////////////////////////
      //
      // Struct UserPing
      //
      struct UserPing
      {
        // Ping in milliseconds
        U16 ping;

        // Hops
        U8 hops;

      };


      ////////////////////////////////////////////////////////////////////////////////
      //
      // Struct UserConnection
      //
      struct UserConnection
      {
        // Number of pings & hops samples
        U32 num;

        // Number of pings lost
        U32 lost;

        // Smooth ping
        U16 pingSmooth;

        // Smooth ping deviation
        U16 pingSmoothDev;

        // Max ping
        U16 pingMax;

        // Min ping
        U16 pingMin;

        // Sum ping
        U32 pingSum;

        // Recent pings
        U16 pings[maxPings];

        // Smooth hos
        U8 hopsSmooth;

        // Max hops
        U8 hopsMax;

        // Min hops
        U8 hopsMin;

        // Sum hops
        U32 hopsSum;

        // Constructor
        UserConnection();

        // Process a ping
        void ProcessPing(U16 ping, U8 hops);

        // Add a ping to a ping array
        void AddToArray(U16 ping, U16 *pings, U32 numPings);

      };

      #pragma pack(pop)
    }

  }

}


#endif