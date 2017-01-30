////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_std.h"

#include "random.h"

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

    ////////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Data
    //
    namespace Data
    {

      ////////////////////////////////////////////////////////////////////////////////
      //
      // Struct UserConnection
      //


      //
      // Constructor
      //
      UserConnection::UserConnection()
      {
        Utils::Memset(this, 0x00, sizeof (UserConnection));
      }


      //
      // ProcessPing
      //
      void UserConnection::ProcessPing(U16 pingIn, U8 hopsIn)
      {
        // Increment the number of received pings
        num++;

        if (pingIn == U16_MAX)
        {
          // This was a loss
          lost++;
          AddToArray(U16_MAX, pings, maxPings);
        }
        else
        {
          // Err = M - A
          S32 err = pingIn - pingSmooth;

          // A <- A + gErr  (g = 1/8)
          pingSmooth = U16(pingSmooth + err / 8);

          // D <- D + h(|Err| - D)  (g = 1/4)
          pingSmoothDev = U16(pingSmoothDev + (abs(err) - pingSmoothDev) / 4);

          // RTO <- A + max(A/2,4D) // we'll use this calculation in the server for determining the interval
          //rto = rtt + max(rtt / 2, 4 * rttdev);

          pingMax = Max(pingIn, pingMax);
          pingMin = Min(pingIn, pingMin);
          pingSum += pingIn;
          AddToArray(pingIn, pings, Std::Data::maxPings);

          hopsSmooth = hopsIn;
          hopsMax = Max(hopsIn, hopsMax);
          hopsMin = Max(hopsIn, hopsMin);
          hopsSum += hopsSum;
        }
      }


      //
      // Insert a ping into a ping array
      //
      void UserConnection::AddToArray(U16 ping, U16 *pings, U32 numPings)
      {
        Utils::Memmove(&pings[1], &pings[0], sizeof (U16) * (numPings - 1));
        pings[0] = ping;
      }

    }

  }

}

