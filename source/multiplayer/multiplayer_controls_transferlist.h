///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_TRANSFERLIST_H
#define __MULTIPLAYER_CONTROLS_TRANSFERLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"
#include "ifvar.h"
#include "multiplayer_transfer_private.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace TransferListMsg
    //
    namespace TransferListMsg
    {
      const U32 Halt = 0x98984398; // "MultiPlayer::TransferList::Message::Halt"
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class TransferList
    //
    class TransferList : public ICListBox
    {
      PROMOTE_LINK(TransferList, ICListBox, 0x12C92F36) // "TransferList"

    private:

      class Item;
      friend Item;

      // Offsets
      Point<S32> offsetFile;
      Point<S32> offsetPlayer;
      Point<S32> offsetTransferred;
      Point<S32> offsetRate;
      Point<S32> offsetETA;
      Point<S32> offsetProgress;
      S32 heightProgress;

    public:

      // Constructor
      TransferList(IControl *parent);

      // Setup
      void Setup(FScope *fScope);

      // HandleEvent
      U32 HandleEvent(Event &e);

      // Add Transfer
      IControl * AddTransfer(Transfer::Offer *offer);

    };

  }

}


#endif
