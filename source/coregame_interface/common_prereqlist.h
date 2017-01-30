///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 9-AUG-1999
//


#ifndef __COMMON_PREREQLIST_H
#define __COMMON_PREREQLISt_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "prereq.h"
#include "multilanguage.h"
#include "common_typepreview.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Editor
//
namespace Common
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class PrereqList
  //
  class PrereqList : public IControl
  {
    PROMOTE_LINK(PrereqList, IControl, 0xF210E7EF) // "PrereqList"

  protected:

    // Point
    Point<S32> point;

    // The team
    Team *team;

    // The type prereqs are being displayed for
    UnitObjType *type;

    // Reaper to the facility
    UnitObjPtr constructor;

    // Cost and time language keys
    MultiIdent costKey, timeKey;

    // Type preview
    TypePreviewPtr preview;

  public:

    // Constructor
    PrereqList(IControl *parent);
    ~PrereqList();

    // Configure control from an FScope
    void Setup(FScope *fScope);

    // Setup
    void Setup(const Point<S32> &point, Team *team, UnitObjType *type, UnitObj *constructor);

    // Called after Configure() is completed
    void PostConfigure();

    // Activate
    Bool Activate();

  protected:

    // Draw control
    void DrawSelf(PaintInfo &pi);

    // Build
    U32 Build(Bool first, UnitObjType *type, BinTree<UnitObjType> &types, List<UnitObjType> &list);

  };
}

#endif
