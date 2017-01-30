///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// ObjectTweak - MapObj
//
// 15-SEP-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_objecttweak.h"
#include "studio_private.h"
#include "promote.h"
#include "mapobj.h"


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
    // Class MapObj
    //


    //
    // MapObj::MapObj
    //
    MapObj::MapObj(IControl *parent)
    : BaseObj(parent)
    {
      // Create IFace Vars
      hitPoints = new IFaceVar(this, CreateFloat("hitPoints", 0.0f, 0.0f, 1.0f));
      armour = new IFaceVar(this, CreateFloat("armour", 0.0f, 0.0f, 1.0f));
    }


    //
    // MapObj::~MapObj
    //
    MapObj::~MapObj()
    {
      // Delete IFace Vars
      delete hitPoints;
      delete armour;
    }


    //
    // MapObj::Notify
    //
    void MapObj::Notify(IFaceVar *var)
    {
      if (var == hitPoints)
      {
        // Iterate the selected objects
        for (MapObjList::Iterator i(&data.sList); *i; i++)
        {
          if ((*i)->Alive())
          {
            ::MapObj *mapObj = **i;

            // Modify the hitpoints of the object
            mapObj->ModifyHitPoints(-mapObj->GetHitPoints());
            mapObj->ModifyHitPoints(S32(hitPoints->GetFloatValue() * F32(mapObj->MapType()->GetHitPoints())));
          }
        }
      }
      else if (var == armour)
      {
        // Iterate the selected objects
        for (MapObjList::Iterator i(&data.sList); *i; i++)
        {
          if ((*i)->Alive())
          {
            ::MapObj *mapObj = **i;

            // Modify the armour of the object
            mapObj->ModifyArmour(-mapObj->GetArmour());
            mapObj->ModifyArmour(S32(armour->GetFloatValue() * F32(mapObj->MapType()->GetArmour())));
          }
        }
      }
      else
      {
        IControl::Notify(var);
      }
    }


    //
    // MapObj::Activate
    //
    Bool MapObj::Activate()
    {
      if (IControl::Activate())
      {
        ActivateVar(hitPoints);
        ActivateVar(armour);

        return (TRUE);
      }
      else
      {
        return (FALSE);
      }
    }


    //
    // MapObj::Deactivate
    //
    Bool MapObj::Deactivate()
    {
      if (IControl::Deactivate())
      {
        hitPoints->Deactivate();
        armour->Deactivate();

        return (TRUE);
      }
      else
      {
        return (FALSE);
      }
    }

  }

}
