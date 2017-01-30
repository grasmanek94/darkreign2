/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __ICSYSTEMBUTTON_H
#define __ICSYSTEMBUTTON_H


#include "icbutton.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICSystemButton - System button control
//
class ICSystemButton : public ICButton
{
  PROMOTE_LINK(ICSystemButton, ICButton, 0xC48224FB); // "ICSystemButton"

public:

  // Styles
  enum
  {
    // Allow code to draw button face
    STYLE_CODEDRAWN   = (1 << 0)
  };

  // System button functions
  enum Function
  {
    CLOSE,
    HELP,
    SLIDER_LEFT,
    SLIDER_RIGHT,
    SLIDER_UP,
    SLIDER_DOWN,
    MINIMIZE,
    MAXIMIZE,
    DROPLIST
  };

protected:

  // Style
  U32 sysButtonStyle;

  // Functions
  Function function;

  // Delay before polling begins
  S32 pollDelay;

  // Font
  Font *font;

protected:

  // Event handling
  U32 HandleEvent(Event &e);

  // Polling callback
  void Poll();

  // Draw this control into the bitmap
  void DrawSelf(PaintInfo &pi);

  // Draw close icon
  void DrawCloseIcon(const ClipRect &r, Color c);

  // Draw slider icons
  void DrawLeftIcon(const ClipRect &r, Color c);
  void DrawRightIcon(const ClipRect &r, Color c);
  void DrawUpIcon(const ClipRect &r, Color c);
  void DrawDownIcon(const ClipRect &r, Color c);

  // Draw character icon
  void DrawCharacterIcon(const ClipRect &r, Color c, CH ch);

public:

  // Constructor
  ICSystemButton(Function func, IControl *parent);

  // Activate
  Bool Activate();

  // Configure a style item
  Bool SetStyleItem(const char *s, Bool toggle);
};


///////////////////////////////////////////////////////////////////////////////
//
// Type definitions
//
typedef Reaper<ICSystemButton> ICSystemButtonPtr;


#endif
