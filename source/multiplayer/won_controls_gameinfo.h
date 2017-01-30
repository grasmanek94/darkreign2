///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Won Stuff
//


#ifndef __WON_CONTROLS_PLAYERINFO_H
#define __WON_CONTROLS_PLAYERINFO_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "ifvar.h"
#include "won_controls_gamelist.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Won
//
namespace Won
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class GameInfo
    //
    class GameInfo : public ICWindow
    {
    private:

      // Ping graph rectangle
      Area<S32> pingGraph;

      // Game list item we are displaying info for
      Reaper<GameList::Item> itemPtr;

      // Name CRC of the control
      U32 itemNameCrc;

      // List of games
      IControlPtr containerPtr;

    public:

      // Constructor
      GameInfo(IControl *parent);

      // Destructor
      ~GameInfo();

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
