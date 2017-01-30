////////////////////////////////////////////////////////////////////////////////
//
// Network IP
//
// Copyright 1999-2000
// Matthew Versluys
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "network_ip.h"


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

    // 
    // CheckSum
    //
    // Compute an IP checksum
    //
    U16 CheckSum(U16 *buffer, U32 size)
    {
      U32 cksum = 0;

      // Sum all the words together, adding the final byte if size is odd
      while (size > 1) 
      {
        cksum += *buffer++;
        size -= sizeof (U16);
      }

      if (size) 
      {
        cksum += *(U8 *) buffer;
      }

      // Do a little shuffling
      cksum = (cksum >> 16) + (cksum & 0xffff);
      cksum += (cksum >> 16);
    
      // Return the bitwise complement of the resulting mishmash
      return ((U16)(~cksum));
    }

  }

}
