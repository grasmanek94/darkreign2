///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tactical Settings Control
//
// 24-NOV-1999
//


#ifndef __CLIENT_TACTICAL_H
#define __CLIENT_TACTICAL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icbutton.h"
#include "unitobjinfo.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TacticalButton - A single tactical modifier button
  //
  class TacticalButton : public ICButton
  {
    PROMOTE_LINK(Icon, IControl, 0xCEA5E93F); // "Client::TacticalButton"

  protected:

    // Has the button been setup
    Bool setup;

    // The tactical setting
    struct { U8 m; U8 s; } pair;
    
    // The color group used for multiple settings
    ColorGroup *multipleColorGroup;

    // Draw control
    void DrawSelf(PaintInfo &pi);

  public:

    // Constructor and destructor
    TacticalButton(IControl *parent);
    ~TacticalButton();

    // Configure control from an FScope
    void Setup(FScope *fScope);

    // Control methods
    U32 HandleEvent(Event &e);
  };
}

#endif
