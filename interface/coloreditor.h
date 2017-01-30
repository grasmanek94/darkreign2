///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Color Picker
//
// 26-FEB-1999
//


#ifndef __COLOREDITOR_H
#define __COLOREDITOR_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ColorBar
//
class ColorBar : public IControl
{
  PROMOTE_LINK(ColorBar, IControl, 0x8A4ACE1C) // "ColorBar"

protected:

  // Current value
  IFaceVar *var;

  // Top Color
  Color topColor;

  // Bottom Color
  Color bottomColor;

  // Middle Color
  Color middleColor;

public:

  // Constructor 
  ColorBar(const char *name, const ClipRect &rect, IControl *parent);

  // Event handler
  U32 HandleEvent(Event &e);

  // Set Top Color
  void SetTopColor(const Color &color);

  // Set Bottom Color
  void SetBottomColor(const Color &color);

  // Set Middle Color
  void SetMiddleColor(const Color &color);

  // Setup the var for this slider to use
  void SetVar(IFaceVar *item);

protected:

  // Control draws itself
  void DrawSelf(PaintInfo &pi);

};


///////////////////////////////////////////////////////////////////////////////
//
// Class ColorMap
//
class ColorMap : public IControl
{
  PROMOTE_LINK(ColorMap, IControl, 0x8806D6DF) // "ColorMap"

protected:

  // Hue
  IFaceVar *hueVar;

  // Saturatiom
  IFaceVar *saturationVar;

  // Saturation
  IFaceVar *luminosityVar;

public:

  // Constructor 
  ColorMap(const char *name, const Point<S32> &point, IControl *parent);

  // Event handler
  U32 HandleEvent(Event &e);

  // Setup the var for this slider to use
  void SetHueVar(IFaceVar *item);

  // Setup the var for the saturation
  void SetSaturationVar(IFaceVar *item);

  // Setup the var for the luminosity
  void SetLuminosityVar(IFaceVar *item);

protected:

  // Control draws itself
  void DrawSelf(PaintInfo &pi);

};


///////////////////////////////////////////////////////////////////////////////
//
// Class ColorEditor
//
class ColorEditor : public ICWindow
{
  PROMOTE_LINK(ColorEditor, ICWindow, 0x9B6B36ED) // "ColorEditor"

protected:

  IFaceVar *color;

  IFaceVar *hue;
  IFaceVar *luminosity;
  IFaceVar *saturation;

  IFaceVar *red;
  IFaceVar *green;
  IFaceVar *blue;

  ColorMap *hueSatMap;
  ColorBar *lumBar;
  ColorBar *redBar;
  ColorBar *greenBar;
  ColorBar *blueBar;

  ColorBar *colorDisp;
  ColorBar *redDisp;
  ColorBar *greenDisp;
  ColorBar *blueDisp;

  Point<S32> cPoint;

protected:

  // Notification that a local var has changed value
  void Notify(IFaceVar *var);

public:

  // Constructor
  ColorEditor(IControl *parent);
  ~ColorEditor();

  // Event handling
  U32 HandleEvent(Event &e);

  // Control draws itself
  void DrawSelf(PaintInfo &pi);

  // Activate and deactivate this control
  Bool Activate();
  Bool Deactivate();

  // Set the var to be used for color
  void SetColorVar(IFaceVar *color);

};

#endif
