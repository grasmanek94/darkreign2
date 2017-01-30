///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_SYNCLIST_H
#define __MULTIPLAYER_CONTROLS_SYNCLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"
#include "ifvar.h"


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
    // Class SyncList
    //
    class SyncList : public ICListBox
    {
    private:

      class PlayerItem;
      friend PlayerItem;

      // Icons
      TextureInfo iconReady[2];

      // Offsets
      S32 offsetName;
      S32 offsetReady;

    public:

      // Constructor
      SyncList(IControl *parent);

      // Redraw self
      void DrawSelf(PaintInfo &pi);

      // Setup
      void Setup(FScope *fScope);

    };

  }

}


#endif
