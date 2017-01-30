///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Tranfer
//

#ifndef __MULTIPLAYER_TRANSFER_H
#define __MULTIPLAYER_TRANSFER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Transfer
  //
  namespace Transfer
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Type
    //
    namespace Type
    {
      static CRC Mission = 0xA1C27797; // "Type::Mission"
    }


    // Initialization and shutdown
    void Init();
    void Done();

    // Reset
    void Reset();

    // Process
    void Process();

    // Interpret a command
    void Handler(CRC from, CRC key, U32 size, const U8 *data);

    // Player has left
    void PlayerLeft(U32 id);

    // Make an offer
    void MakeOffer(CRC to, CRC type, const char *path);

  }

}

#endif
