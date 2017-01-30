///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 9-AUG-1999
//


#ifndef __COMMON_PREREQTREE_H
#define __COMMON_PREREQTREE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "prereq.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Editor
//
namespace Common
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class PrereqTree
  //
  class PrereqTree : public IControl
  {
    PROMOTE_LINK(PrereqTree, IControl, 0x509FE9F5) // "PrereqTree"

  protected:

    // Point
    Point<S32> point;

    // The team
    Team *team;

    // The type prereqs are being displayed for
    UnitObjType *type;

    // Reaper to the facility
    UnitObjPtr constructor;

  public:

    // Constructor
    PrereqTree(IControl *parent);
    ~PrereqTree();

    // Setup
    void Setup(const Point<S32> &point, Team *team, UnitObjType *type, UnitObj *constructor);

  protected:

    // Draw control
    void DrawSelf(PaintInfo &pi);

    // Draw 
    void Draw(Bool first, PaintInfo &pi, S32 x, S32 &y, S32 &bottom, UnitObjType *type);

  };
}

#endif
