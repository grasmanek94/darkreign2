/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Static Text Control
//
// 06-JUL-1998
//


#ifndef __ICSTATIC_H
#define __ICSTATIC_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "varsys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICStatic - static text or image control
//
class ICStatic : public IControl
{
  PROMOTE_LINK(ICStatic, IControl, 0xD877779B); // "ICStatic"

public:

  // Control styles
  enum
  {
    // Show numeric valid in min:sec format
    STYLE_SHOWMINSEC      = 0x00000001,

    // Sheet textures
    STYLE_SHEET           = 0x00000002
  };

protected:

  // Display style
  U32 staticStyle;

  // Var to display
  IFaceVar *displayVar;

  // Image var
  IFaceVar *imageVar;

  // Forced text color
  Color textClr;

  // Multiple images/sheets
  BinTree<TextureInfo> images;
  Point<U8> sheets;

  // Indent
  Point<S32> indent;

  // Draw control
  void DrawSelf(PaintInfo &pi);

  // Var changed
  void Notify(IFaceVar *var);

  // Updated sheeted texture coordinates
  void UpdateSheets();

public:
  
  ICStatic(IControl *parent);
  ~ICStatic();

  // Control methods
  void Setup(FScope *fScope);
  Bool SetStyleItem(const char *s, Bool toggle); 
  Bool Activate();
  Bool Deactivate();

  // Use a named image
  void SetNamedImage(U32 nameCrc);

  // Set the display var
  void SetDisplayVar(const char *name);

  // Set indent
  void SetIndent(const Point<S32> &i)
  {
    indent = i;
  }

};


///////////////////////////////////////////////////////////////////////////////
//
// Type definitions
//
typedef Reaper<ICStatic> ICStaticPtr;


#endif
