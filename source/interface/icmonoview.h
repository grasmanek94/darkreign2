/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Mono viewer control
//
// 20-JUL-98
//


#ifndef __ICMONOVIEW_H
#define __ICMONOVIEW_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICMonoView
//
class ICMonoView : public IControl
{
  PROMOTE_LINK(ICMonoView, IControl, 0x59874A2E) // "ICMonoView"

protected:

  // Cell size
  Point<S32> cell;

  // Colors
  Color colorFg;
  Color colorBg;
  Color colorBright;

public:

  // Constructor
  ICMonoView(IControl *parent);

  // DrawSelf
  void DrawSelf(PaintInfo &pi);

  // Setup
  void Setup(FScope *fScope);

};

#endif
