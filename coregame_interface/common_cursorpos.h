///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 11-AUG-1998
//


#ifndef __COMMON_CURSORPOS_H
#define __COMMON_CURSORPOS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icstatic.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TerrainCursorPos
  //
  class TerrainCursorPos : public ICStatic
  {
    PROMOTE_LINK(TerrainCursorPos, ICStatic, 0xF8E8F42E) // "TerrainCursorPos"

  protected:

    // Buffer for text
    CH buf[128];

    // Units of display
    U32 units;

    // Update cursor position and redraw self
    void DrawSelf(PaintInfo &pi);

  public:

    // Constructor
    TerrainCursorPos(IControl *parent);
    
    // Setup
    void Setup(FScope *fScope);
  };

}

#endif
