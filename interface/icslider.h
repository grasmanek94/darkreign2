/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Slider control
//
// 06-JUL-1998
//


#ifndef __ICSLIDER_H
#define __ICSLIDER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icbutton.h"
#include "varsys.h"


///////////////////////////////////////////////////////////////////////////////
//
// ICSlider messages
//
namespace ICSliderMsg
{
  const U32 Increment = 0x29455E91; // "Slider::Message::Increment"
  const U32 Decrement = 0xA828C0E3; // "Slider::Message::Decrement"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICSlider
//
class ICSlider : public IControl
{
  PROMOTE_LINK(ICSlider, IControl, 0x79908AD7); // "ICSlider"

public:

  // Slider styles
  enum
  {
    // Create increment/decrement buttons
    STYLE_BUTTONS      = (1 << 0),

    // Don't draw client aread
    STYLE_NODRAWCLIENT = (1 << 1),

    // Create square thumb
    STYLE_SQUARETHUMB  = (1 << 2)
  };

  // Slider orientation
  enum Orientation
  {
    VERTICAL,
    HORIZONTAL,
  };

protected:

  // Display style
  U32 sliderStyle;

  // Is slider horizontal?
  U32 horizontal : 1,

  // Knob needs to be moved
      moveKnob : 1,

  // Use variable range
      useRange : 1;

  // Current value
  IFaceVar *sliderVar;

  // Current value
  F32 curVal;

  // Sliding range
  F32 minVal;
  F32 maxVal;
  F32 range;

  // Increment value when sliding by one notch
  F32 incStep;

  // Var range
  F32 rangeHi;
  F32 rangeLo;

  // Thumb track control
  IControlPtr thumbBtn;

  // Customised configuration
  FScope *decBtnCfg;
  FScope *incBtnCfg;
  FScope *thumbCfg;

  // Friends
  friend class ICSliderThumb;

protected:

  // Retrieve the slider value from the var
  virtual void GetSliderValue();

  // Store the slider value in the var
  virtual void SetSliderValue(F32 value);

  // Calculate an adjustment rect based on the style
  ClipRect GetAdjustmentRect();

  // PostConfigure
  void PostConfigure();

  // Initialise range parameters, only to be done when active
  void InitRange();
  
  // Draw the control
  void DrawSelf(PaintInfo &pi);

  // Event handler
  U32 HandleEvent(Event &e);

  // Var notification
  void Notify(IFaceVar *var);

public:

  ICSlider(IControl *parent);
  ~ICSlider();

  // Configure control from an FScope
  void Setup(FScope *fScope);

  // Style configuration
  Bool SetStyleItem(const char *s, Bool toggle);

  // Activate control
  Bool Activate();

  // Deactivate control
  Bool Deactivate();

  // Set the orientation of the slider
  void SetOrientation(ICSlider::Orientation o);

  // Set the orientation of the slider using the string representation
  void SetOrientation(const char *s);

  // Setup the var for this slider to use
  void UseVar(VarSys::VarItem *item);

  // Return range of movement of thumb
  ClipRect GetThumbRange();
};


///////////////////////////////////////////////////////////////////////////////
//
// Class ICSliderThumb - Slider thumb track
//
class ICSliderThumb : public ICButton
{
  PROMOTE_LINK(ICSliderThumb, ICButton, 0x89D2B50F); // "ICSliderThumb"

protected:

  // Parent
  ICSlider *slider;

  // Position to lock mouse to while dragging
  Point<S32> dragPos;

protected:

  // PostConfigure
  void PostConfigure();

public:

  // Constructor
  ICSliderThumb(IControl *parent);

  // Event handler
  U32 HandleEvent(Event &e);
};


//
// Type definitions
//
typedef Reaper<ICSlider> ICSliderPtr;
typedef Reaper<ICSliderThumb> ICSliderThumbPtr;


#endif
