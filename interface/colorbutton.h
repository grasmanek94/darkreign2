/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __COLORBUTTON_H
#define __COLORBUTTON_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icbutton.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ColorButton
//
class ColorButton : public ICButton
{
  PROMOTE_LINK(ColorButton, ICButton, 0xC7175E0B); // "ColorButton"

protected:

  // Interface var being used
  IFaceVar *var;

  // Draw this control into the bitmap
  void DrawSelf(PaintInfo &pi);

  // Edit the color
  void EditColor();

public:

  // Constructor and Destructor
  ColorButton(IControl *parent);
  ~ColorButton();

  // Setup this control from one scope function
  void Setup(FScope *fScope);

  // Activation
  Bool Activate();

  // Deactivate
  Bool Deactivate();

  // Handle input and interface events
  U32 HandleEvent(Event &e);

  // Handle notifications
  void Notify(IFaceVar *var);

  // Set the var to use
  void UseVar(IFaceVar *varIn);
  
};

#endif
