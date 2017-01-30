///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// ObjectTweak - UnitObj
//
// 15-SEP-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_objecttweak.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ObjectTweak
  //
  namespace ObjectTweak
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class UnitObj
    //


    //
    // UnitObj::UnitObj
    //
    UnitObj::UnitObj(IControl *parent)
    : BaseObj(parent)
    {
      // Create IFace Vars
      team = new IFaceVar(this, CreateString("team", ""));
    }


    //
    // UnitObj::~UnitObj
    //
    UnitObj::~UnitObj()
    {
      // Delete IFace Vars
      delete team;
    }


    //
    // UnitObj::Notify
    //
    void UnitObj::Notify(IFaceVar *var)
    {
      IControl::Notify(var);
    }


    //
    // UnitObj::Activate
    //
    Bool UnitObj::Activate()
    {
      if (IControl::Activate())
      {
        return (TRUE);
      }
      else
      {
        return (FALSE);
      }
    }


    //
    // UnitObj::Deactivate
    //
    Bool UnitObj::Deactivate()
    {
      if (IControl::Deactivate())
      {
        return (TRUE);
      }
      else
      {
        return (FALSE);
      }
    }

  }

}
