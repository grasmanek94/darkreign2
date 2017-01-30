///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_PLAYERINFO_H
#define __MULTIPLAYER_CONTROLS_PLAYERINFO_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
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
    // Class PlayerInfo
    //
    class PlayerInfo : public ICWindow
    {
    private:

      // Ping graph rectangle
      Area<S32> pingGraph;

      // Network id
      U32 networkId;

    public:

      // Constructor
      PlayerInfo(IControl *parent);

      // Destructor
      ~PlayerInfo();

      // Activate
      Bool Activate();

      // Setup
      void Setup(FScope *fScope);

      // Draw self
      void DrawSelf(PaintInfo &pi);

    };

  }

}


#endif
