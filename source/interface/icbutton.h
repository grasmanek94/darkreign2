/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __ICBUTTON_H
#define __ICBUTTON_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// Button messages
//
namespace ICButtonMsg
{
  const U32 Press   = 0x835C3689; // "Button::Message::Press"
  const U32 Toggle  = 0xFEA88EEA; // "Button::Message::Toggle"
}

namespace ICButtonNotify
{
  const U32 Pressing = 0xFA95B735; // "Button::Notify::Pressing"
  const U32 Pressed = 0x3710E857; // "Button::Notify::Pressed"
  const U32 DisabledDown = 0x1893BCB4; // "Button::Notify::DisabledDown"
  const U32 Toggled = 0x19BA89AA; // "Button::Notify::Toggled"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICButton - Standard button control
//
class ICButton : public IControl
{
  PROMOTE_LINK(ICButton, IControl, 0xBF84153D); // "ICButton"

public:

  // Button styles
  enum 
  {
    // Toggle button
    STYLE_TOGGLE      = 0x00000001,
    STYLE_FLAT        = 0x00000002,
    STYLE_SELECTDOWN  = 0x00000004
  };

  // Current button state
  enum ButtonState
  {
    BS_UP = 0,
    BS_DOWN,
    BS_MAX,
  };

protected:

  // Button style
  U32 buttonStyle;

  // Current state
  ButtonState buttonState;

  // Client rectangles for each state
  ClipRect clientRects[BS_MAX];

  // Var for toggle button state
  IFaceVar *toggleVar;

  // Sounds
  U8 soundOver;
  U8 soundClick;

  // Icon
  TextureInfo *icon;

protected:

  // Draw this control into the bitmap
  void DrawSelf(PaintInfo &pi);

  // Set button state
  void SetState(ButtonState state);

  // Var changed
  void Notify(IFaceVar *var);

public:

  ICButton(IControl *parent);
  ~ICButton();

  // Activate the control
  Bool Activate();
  Bool Deactivate();

  // Style configuration
  Bool SetStyleItem(const char *s, Bool toggle);

  void SetIcon( TextureInfo & tinfo);

  // Configure this control with an FScope
  void Setup(FScope *fScope);

  // Adjust geometry of control
  void AdjustGeometry();

  // Event handling
  U32 HandleEvent(Event &e);
};


///////////////////////////////////////////////////////////////////////////////
//
// Type definitions
//
typedef Reaper<ICButton> ICButtonPtr;


#endif
