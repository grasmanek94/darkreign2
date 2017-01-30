///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
//


#ifndef __MULTIPLAYER_CONTROLS_LOCALPINGS_H
#define __MULTIPLAYER_CONTROLS_LOCALPINGS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icstatic.h"


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
    // Class LocalPings
    //
    class LocalPings : public ICStatic
    {
      PROMOTE_LINK(LocalPings, ICStatic, 0xEF5F1CA7) // "LocalPings"

    public:

      // Constructor
      LocalPings(IControl *parent)
      : ICStatic(parent)
      { 
      }

      // Redraw self
      void DrawSelf(PaintInfo &pi);


    };

  }

}


#endif
