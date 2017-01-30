///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Unit Context Controls
//
// 24-NOV-1999
//


#ifndef __CLIENT_UNITCONTEXT_H
#define __CLIENT_UNITCONTEXT_H


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
  // Class UnitContextButton - A single unit context item
  //
  class UnitContextButton : public ICButton
  {
    PROMOTE_LINK(Icon, ICButton, 0x5EE82923); // "Client::UnitContextButton"

  public:

    // Possible modes
    enum Mode 
    { 
      INVALID, HIDDEN, INACTIVE, ACTIVE
    };
    
  protected:

    // The context control items
    GameIdent primary;
    GameIdent secondary;

    // The discrete event to trigger
    GameIdent event;

    // The current mode
    Mode mode;

    // Display custom tool tips (TRUE if handled)
    Bool DisplayTip();

  public:

    // Constructor and destructor
    UnitContextButton(IControl *parent);
    ~UnitContextButton();

    // Configure control from an FScope
    void Setup(FScope *fScope);

    // Control methods
    U32 HandleEvent(Event &e);

    // Update this button using the given unit info
    Bool Update(UnitObjInfo *info, Bool automatic);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitContext - Handles a group of context buttons
  //
  class UnitContext : public IControl
  {
    PROMOTE_LINK(UnitContext, IControl, 0x30F543AF); // "Client::UnitContext"

  protected:

    // Control styles
    enum
    {
      STYLE_AUTOMATIC = 0x00000001,
    };

    // Style flags
    U32 unitContextStyle;

    // List of icons
    List<UnitContextButton> buttons;

    // Update this control using the given unit info
    void Update(UnitObjInfo *info);

  public:

    // Constructor and destructor
    UnitContext(IControl *parent);
    ~UnitContext();

    // Style configuration
    Bool SetStyleItem(const char *s, Bool toggle);

    // Poll each cycle
    void Poll();

    // Draw this control
    void DrawSelf(PaintInfo &pi);

    // Add a button to the list
    void Add(UnitContextButton *button);
  };
}

#endif
