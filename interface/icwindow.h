/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//

#ifndef __ICWINDOW_H
#define __ICWINDOW_H


#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// ICWindow messages
//
namespace ICWindowMsg
{
  const U32 Raise         = 0x5B7D4009; // "Window::Message::Raise"
  const U32 Close         = 0xCC038C22; // "Window::Message::Close"
  const U32 Maximize      = 0x27CEC8F1; // "Window::Message::Maximize"
  const U32 Minimize      = 0xB8465630; // "Window::Message::Minimize"
  const U32 Destroy       = 0xA05179A1; // "Window::Message::Destroy"
  const U32 Help          = 0xDE247A8D; // "Window::Message::Help"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICWindow - Standard window control
//
class ICWindow : public IControl
{
  PROMOTE_LINK(ICWindow, IControl, 0x5D341364); // "ICWindow"

public:

  // ICWindow attributes
  enum 
  {
    // Create a title bar
    STYLE_TITLEBAR      = 0x00000001,

    // Create a thin title bar
    STYLE_THINTITLEBAR  = 0x00000002,

    // Create a close button
    STYLE_CLOSEBUTTON   = 0x00000004,

    // Create a max button
    STYLE_MAXBUTTON     = 0x00000008,

    // Create a min button
    STYLE_MINBUTTON     = 0x00000010,

    // Create a help button
    STYLE_HELPBUTTON    = 0x00000020,

    // No system buttons
    STYLE_NOSYSBUTTONS  = 0x00000040,

    // Adjusts window rect to fit client
    STYLE_ADJUSTWINDOW  = 0x00000080,

    // Creates a resizable frame
    STYLE_RESIZEFRAME   = 0x00000100,

    // Can not be moved
    STYLE_IMMOVABLE     = 0x00000200,

  };

protected:

  // Window Style settings for appearance and behaviour
  U32 windowStyle;

  // Configuration scope for title bar
  FScope *titleBarConfig;

  // Configuration scope for close button
  FScope *closeBtnConfig;

  // Titlebar
  IControlPtr titleBar;

protected:

  // Post configuration
  void PostConfigure();

  // Calculate an adjustment rect based on the style
  ClipRect GetAdjustmentRect();

  // Draw the window
  void DrawSelf(PaintInfo &pi);

public:

  // Constructor
  ICWindow(IControl *parent);

  // Destructor
  ~ICWindow();

  // Configure a style item
  Bool SetStyleItem(const char *s, Bool toggle);

  // Configure control
  void Setup(FScope *fScope);

  // Event handler
  U32 HandleEvent(Event &e);

  // Set the text to display on this control
  void SetTextString(const CH *s, Bool dup, Bool cleanup = FALSE);

  // Return window title control
  IControl *GetWindowTitle();

  // Return window styel
  U32 GetWinStyle()
  {
    return (windowStyle);
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class ICWindowTitle - Window titlebar control
//
class ICWindowTitle : public IControl
{
  PROMOTE_LINK(ICWindowTitle, IControl, 0xB506CE12); // "ICWindowTitle"

protected:

  // Position that window dragging begins
  Point<S32> dragPos;

  // Draw the control
  void DrawSelf(PaintInfo &pi);

public:

  // Constructor
  ICWindowTitle(IControl *parent);

  // Event handler
  U32 HandleEvent(Event &e);
};


//
// Type definitions
//
typedef Reaper<ICWindow> ICWindowPtr;
typedef Reaper<ICWindowTitle> ICWindowTitlePtr;


#endif
